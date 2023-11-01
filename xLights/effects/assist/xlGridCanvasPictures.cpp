/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGridCanvasPictures.h"

#include <wx/filefn.h>
#include <wx/filename.h>

#include "../../ResizeImageDialog.h"

#include "../../ExternalHooks.h"
#include "UtilFunctions.h"

static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                               ";*.webp"
                                               "|All files (*.*)|*.*";

BEGIN_EVENT_TABLE(xlGridCanvasPictures, xlGridCanvas)
EVT_PAINT(xlGridCanvasPictures::render)
EVT_MOTION(xlGridCanvasPictures::mouseMoved)
EVT_LEFT_DOWN(xlGridCanvasPictures::mouseLeftDown)
EVT_LEFT_UP(xlGridCanvasPictures::mouseLeftUp)
EVT_RIGHT_DOWN(xlGridCanvasPictures::mouseRightDown)
EVT_RIGHT_UP(xlGridCanvasPictures::mouseRightUp)
END_EVENT_TABLE()

xlGridCanvasPictures::xlGridCanvasPictures(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
:   xlGridCanvas(parent, id, pos, size, style, name),
    mRightDown(false),
    mLeftDown(false),
    img_mode(IMAGE_NONE),
    mModified(false),
    imageCount(1),
    imageIndex(0),
    imageWidth(1),
    imageHeight(1),
    frame(0),
    maxmovieframes(0),
    scaleh(0.0),
    scalew(0.0),
    PictureName(wxEmptyString),
    NewPictureName(wxEmptyString),
    mPaintColor(xlRED),
    mEraseColor(xlBLACK),
    mPaintMode(PAINT_PENCIL),
    mSelectionColor(new xlColor(255,255,255)),
    mDragStartX(-1),
    mDragStartY(-1),
    mDragEndX(-1),
    mDragEndY(-1),
    mHoverDragRow(-1),
    mHoverDragCol(-1),
    mStartRow(-1),
    mStartCol(-1),
    mEndRow(-1),
    mEndCol(-1),
    mHoverSelection(false),
    mCopyAvailable(false),
    imagesValid(false),
    imageTexture(nullptr),
    imageCopy(nullptr)
{
}

xlGridCanvasPictures::~xlGridCanvasPictures()
{
    delete mSelectionColor;
    if (imageTexture) {
        delete imageTexture;
    }
    if (imageCopy) {
        delete imageCopy;
    }
}

void xlGridCanvasPictures::LoadAndProcessImage()
{
    // process loading new image
    if (mModified) {
        PictureName = wxEmptyString;
        mModified = false;
    }

    if (NewPictureName != PictureName) {
        wxLogNull logNo;  // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
        imageCount = wxImage::GetImageCount(NewPictureName);
        imageIndex = 0;
        if (!image.LoadFile(NewPictureName, wxBITMAP_TYPE_ANY, 0)) {
            image.Create(mColumns, mRows, true);
        }
        if (!image.IsOk()) {
            return;
        }
        if (image.HasMask() && !image.HasAlpha()) {
            image.InitAlpha();
        }
    }
    ProcessNewImage();
}
void xlGridCanvasPictures::ProcessNewImage()
{
    wxString image_size = wxEmptyString;

    wxCommandEvent eventImage(EVT_IMAGE_FILE_SELECTED);
    eventImage.SetClientData(&NewPictureName);
    wxPostEvent(mMessageParent, eventImage);

    int imgwidth = image.GetWidth();
    int imght    = image.GetHeight();

    image_size = wxString::Format("Image Size: %d x %d", imgwidth, imght);
    wxCommandEvent eventImageSize(EVT_IMAGE_SIZE);
    eventImageSize.SetString(image_size);
    wxPostEvent(mMessageParent, eventImageSize);

    if (imgwidth > mColumns || imght > mRows) {
        if (imageCount > 1) {
            img_mode = IMAGE_MULTIPLE_OVERSIZED;
        } else {
            img_mode = IMAGE_SINGLE_OVERSIZED;
        }
    } else {
        if (imageCount > 1) {
            img_mode = IMAGE_MULTIPLE_FITS;
        } else {
            img_mode = IMAGE_SINGLE_FITS;
        }
    }
    imagesValid = false;
    render();
}

void xlGridCanvasPictures::LoadImage()
{
    wxFileName dummy_file(PictureName);
    // prompt for new filename
    wxFileDialog fd(this,
                    "Choose Image File to Load:",
                    dummy_file.GetPath(),
                    wxEmptyString,
                    strSupportedImageTypes,
                    wxFD_OPEN);
    int result = fd.ShowModal();
    wxString new_filename = fd.GetPath();
    if (result == wxID_CANCEL || new_filename == "") {
        return;
    }
    ObtainAccessToURL(new_filename.ToStdString());
    PictureName = new_filename;
    UpdateRenderedImage();

    mModified = true;
    NewPictureName = PictureName;
    LoadAndProcessImage();
}

void xlGridCanvasPictures::SaveAsImage()
{
    wxFileName save_file(PictureName);
    wxString save_name = PictureName;
    // prompt for new filename
    wxFileDialog fd(this,
                    "Choose filename to Save Image:",
                    save_file.GetPath(),
                    wxEmptyString,
                    wxFileSelectorDefaultWildcardStr,
                    wxFD_SAVE);
    int result = fd.ShowModal();
    wxString new_filename = fd.GetPath();
    if (result == wxID_CANCEL || new_filename == "") {
        return;
    }
    wxFileName file_check(new_filename);
    if (file_check.GetExt() == "") {
        file_check.SetExt("png");
    }
    save_name = file_check.GetFullPath();

    if (FileExists(save_name)) {
        if (wxMessageBox("Are you sure you want to overwrite this image file?\n" + save_name, "Confirm Overwrite?", wxICON_QUESTION | wxYES_NO) == wxYES ) {
            PictureName = save_name;
            SaveImageToFile();
        } else {
            return;
        }
    } else {
        PictureName = save_name;
        SaveImageToFile();
    }

    mModified = true;
    NewPictureName = PictureName;
    LoadAndProcessImage();
}

void xlGridCanvasPictures::ResizeImage()
{
    if (img_mode == IMAGE_SINGLE_OVERSIZED || img_mode == IMAGE_SINGLE_FITS) {
        ResizeImageDialog dlg(this);
        dlg.HeightSpinCtrl->SetValue(image.GetHeight());
        dlg.WidthSpinCtrl->SetValue(image.GetWidth());
        if (dlg.ShowModal() == wxID_OK) {
            wxImageResizeQuality type = wxIMAGE_QUALITY_NEAREST;

            switch (dlg.ResizeChoice->GetSelection()) {
                case 0:
                    type = wxIMAGE_QUALITY_NEAREST;
                    image.Rescale(dlg.WidthSpinCtrl->GetValue(), dlg.HeightSpinCtrl->GetValue(), type);
                    break;
                case 1:
                    type = wxIMAGE_QUALITY_BILINEAR;
                    image.Rescale(dlg.WidthSpinCtrl->GetValue(), dlg.HeightSpinCtrl->GetValue(), type);
                    break;
                case 2:
                    type = wxIMAGE_QUALITY_BICUBIC;
                    image.Rescale(dlg.WidthSpinCtrl->GetValue(), dlg.HeightSpinCtrl->GetValue(), type);
                    break;
                case 3:
                    type = wxIMAGE_QUALITY_BOX_AVERAGE;
                    image.Rescale(dlg.WidthSpinCtrl->GetValue(), dlg.HeightSpinCtrl->GetValue(), type);
                    break;
                case 4:
                    image.Resize(wxSize(dlg.WidthSpinCtrl->GetValue(), dlg.HeightSpinCtrl->GetValue()),
                                 wxPoint((dlg.WidthSpinCtrl->GetValue()-image.GetWidth()) / 2,
                                         (dlg.HeightSpinCtrl->GetValue()-image.GetHeight()) / 2),
                                 0,0,0);
                    break;
            }
            ProcessNewImage();
        }
    }
}

void xlGridCanvasPictures::SaveImage()
{
    wxFileName save_file(PictureName);
    wxString save_name = PictureName;
    if (save_file.GetFullName() == "NewImage.png") {
        // prompt for new filename
        wxFileDialog fd(this,
                        "Choose filename to Save Image:",
                        save_file.GetPath(),
                        wxEmptyString,
                        wxFileSelectorDefaultWildcardStr,
                        wxFD_SAVE);
        int result = fd.ShowModal();
        wxString new_filename = fd.GetPath();
        if (result == wxID_CANCEL || new_filename == "") {
            return;
        }
        wxFileName file_check(new_filename);
        if (file_check.GetExt() == "") {
            file_check.SetExt("png");
        }
        save_name = file_check.GetFullPath();
    }

    if (FileExists(save_name)) {
        if( wxMessageBox("Are you sure you want to overwrite this image file?\n" + save_name, "Confirm Overwrite?", wxICON_QUESTION | wxYES_NO) == wxYES ) {
            PictureName = save_name;
            SaveImageToFile();
        } else {
            return;
        }
    } else {
        PictureName = save_name;
        SaveImageToFile();
    }

    mModified = true;
    NewPictureName = PictureName;
    LoadAndProcessImage();
}

void xlGridCanvasPictures::SaveImageToFile()
{
    image.SetOption("quality", 100);
    image.SaveFile(PictureName);
    UpdateRenderedImage();
}

void xlGridCanvasPictures::UpdateRenderedImage()
{
    wxString settings = mEffect->GetSettingsAsString();
    wxArrayString all_settings = wxSplit(settings, ',');
    for (int s = 0; s < all_settings.size(); s++) {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if (parts[0] == "E_FILEPICKER_Pictures_Filename") {
            parts[1] = PictureName;
        }
        all_settings[s] = wxJoin(parts, '=');
    }
    settings = wxJoin(all_settings, ',');
    mEffect->SetSettings(settings.ToStdString(), false);

    wxCommandEvent eventEffectChanged(EVT_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(mEffect);
    wxPostEvent(mMessageParent, eventEffectChanged);
}

void xlGridCanvasPictures::CreateNewImage(wxString& image_dir)
{
    if (image_dir == wxEmptyString) {
        DisplayError("Error creating new image file.  Image Directory is not defined.");
        return;
    }
    wxFileName new_file;
    new_file.SetPath(image_dir);
    new_file.SetFullName("NewImage.png");
    NewPictureName = new_file.GetFullPath();
    if (FileExists(NewPictureName)) {
        ::wxRemoveFile(NewPictureName);
    }
    image.Create(mColumns, mRows, true);
    image.SaveFile(NewPictureName, wxBITMAP_TYPE_PNG);

    if (!image.IsOk()) {
        imagesValid = false;
        DisplayError("Error creating image file!");
        return;
    }
    mModified = true;
    ProcessNewImage();
}

void xlGridCanvasPictures::SetEffect(Effect* effect_)
{
    static wxString missing_file = wxEmptyString;

    mEffect = effect_;

    if (mEffect == nullptr) {
        return;
    }
    imagesValid = false;
    NewPictureName = GetImageFilename();
    if (NewPictureName == "") {
        return;
    }
    if (FileExists(NewPictureName)) {
        LoadAndProcessImage();
    } else {
        missing_file = "File Not Found: " + NewPictureName;
        wxCommandEvent eventImage(EVT_IMAGE_FILE_SELECTED);
        eventImage.SetClientData(&missing_file);
        wxPostEvent(mMessageParent, eventImage);
        NewPictureName = "";
    }
}

wxString xlGridCanvasPictures::GetImageFilename()
{
    wxString settings = mEffect->GetSettingsAsString();
    wxArrayString all_settings = wxSplit(settings, ',');
    for (int s = 0; s < all_settings.size(); s++) {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if (parts[0] == "E_FILEPICKER_Pictures_Filename") {
            return parts[1];
        }

    }
    return wxEmptyString;
}

void xlGridCanvasPictures::mouseRightDown(wxMouseEvent& event)
{
    if (!mLeftDown) {
        mRightDown = true;
        mouseDown(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
    }
}

void xlGridCanvasPictures::mouseLeftDown(wxMouseEvent& event)
{
    SetFocus();
    if (!mRightDown) {
        mLeftDown = true;
        mouseDown(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
    }
}

void xlGridCanvasPictures::mouseDown(int x, int y)
{
    if (mEffect == nullptr) return;

    if (img_mode == IMAGE_SINGLE_FITS ) {
        int column = GetCellFromPosition(x);
        int row = GetCellFromPosition(y);
        if (column >= 0 && column < mColumns && row >= 0 && row < mRows) {
            int draw_row = mRows - row - 1;
            row = image.GetHeight() - draw_row - 1;
            //SetCurrentGLContext();
            if (column < image.GetWidth() && draw_row < image.GetHeight()) {
                if (mPaintMode == PAINT_PENCIL && !mRightDown) {
                    if (imageTexture) imageTexture->UpdatePixel(column, draw_row, mPaintColor, image.HasAlpha());
                    image.SetRGB(column, row, mPaintColor.red, mPaintColor.green, mPaintColor.blue);
                    if (image.HasAlpha()) {
                        image.SetAlpha(column, row, mPaintColor.alpha);
                    }
                } else if ((mPaintMode == PAINT_ERASER || mRightDown) && !mHoverSelection) {
                    if (imageTexture) imageTexture->UpdatePixel(column, draw_row, mEraseColor, image.HasAlpha());
                    image.SetRGB(column, row, mEraseColor.red, mEraseColor.green, mEraseColor.blue);
                    if (image.HasAlpha()) {
                        image.SetAlpha(column, row, mEraseColor.alpha);
                    }
                } else if (mPaintMode == PAINT_EYEDROPPER && !mRightDown) {
                    xlColor eyedrop_color;
                    eyedrop_color.red = image.GetRed(column, row);
                    eyedrop_color.green = image.GetGreen(column, row);
                    eyedrop_color.blue = image.GetBlue(column, row);
                    wxCommandEvent eventEyedrop(EVT_EYEDROPPER_COLOR);
                    eventEyedrop.SetInt(eyedrop_color.GetRGB());
                    wxPostEvent(mMessageParent, eventEyedrop);
                } else if (mPaintMode == PAINT_SELECTCOPY) {
                    if (!mRightDown) {
                        if (!mHoverSelection) {
                            mDragStartX = x;
                            mDragStartY = y;
                            mDragEndX = x;
                            mDragEndY = y;
                            CalcSelection();
                            mCopyAvailable = false;
                        } else {
                            mHoverDragCol = GetCellFromPosition(x);
                            mHoverDragRow = GetCellFromPosition(y);
                        }
                    } else if (mHoverSelection) {
                        mDragStartX = -1;  // disables selection drawing
                        mEndCol = -1;
                        mCopyAvailable = false;
                        mHoverSelection = false;
                    }
                }
            }
        }

        mModified = true;
        mDragging = true;
        CaptureMouse();
        render();
    }
}

void xlGridCanvasPictures::mouseMoved(wxMouseEvent& event)
{
    if (mEffect == nullptr) return;

    int column =  GetCellFromPosition(mapLogicalToAbsolute(event.GetX()));
    int row = GetCellFromPosition(mapLogicalToAbsolute(event.GetY()));
    if (img_mode == IMAGE_SINGLE_FITS && column >= 0 && column < mColumns && row >= 0 && row < mRows) {
        if (mRightDown) {
            SetCursor(wxCURSOR_PAINT_BRUSH);
        } else {
            switch(mPaintMode) {
            case PAINT_PENCIL:
                SetCursor(wxCURSOR_PENCIL);
                break;
            case PAINT_ERASER:
                SetCursor(wxCURSOR_PAINT_BRUSH);
                break;
            case PAINT_EYEDROPPER:
                SetCursor(wxCURSOR_BULLSEYE);
                break;
            case PAINT_SELECTCOPY:
                if (!mDragging) {
                    if (column >= mStartCol && column <= mEndCol && row >= mStartRow && row <= mEndRow) {
                        SetCursor(wxCURSOR_HAND);
                        mHoverSelection = true;
                    } else {
                        SetCursor(wxCURSOR_CROSS);
                        mHoverSelection = false;
                    }
                }
                break;
            }
        }

        if (mDragging) {
            int draw_row = mRows - row - 1;
            row = image.GetHeight() - draw_row - 1;
            //SetCurrentGLContext();
            if (column < image.GetWidth() && draw_row < image.GetHeight()) {
                if (mPaintMode == PAINT_PENCIL && !mRightDown) {
                    if (imageTexture) imageTexture->UpdatePixel(column, draw_row, mPaintColor, image.HasAlpha());
                    image.SetRGB(column, row, mPaintColor.red, mPaintColor.green, mPaintColor.blue);
                    if (image.HasAlpha()) {
                        image.SetAlpha(column, row, mPaintColor.alpha);
                    }
                } else if ((mPaintMode == PAINT_ERASER || mRightDown) && !mHoverSelection) {
                    if (imageTexture) imageTexture->UpdatePixel(column, draw_row, mEraseColor, image.HasAlpha());
                    image.SetRGB(column, row, mEraseColor.red, mEraseColor.green, mEraseColor.blue);
                    if (image.HasAlpha()) {
                        image.SetAlpha(column, row, mEraseColor.alpha);
                    }
                } else if (mPaintMode == PAINT_EYEDROPPER && !mRightDown) {
                    xlColor eyedrop_color;
                    eyedrop_color.red = image.GetRed(column, row);
                    eyedrop_color.green = image.GetGreen(column, row);
                    eyedrop_color.blue = image.GetBlue(column, row);
                    wxCommandEvent eventEyedrop(EVT_EYEDROPPER_COLOR);
                    eventEyedrop.SetClientData(&eyedrop_color);
                    wxPostEvent(mMessageParent, eventEyedrop);
                } else if (mPaintMode == PAINT_SELECTCOPY && !mRightDown) {
                    if (!mHoverSelection) {
                        mDragEndX = mapLogicalToAbsolute(event.GetX());
                        mDragEndY = mapLogicalToAbsolute(event.GetY());
                        CalcSelection();
                    } else {
                        ProcessHoverDrag(column, GetCellFromPosition(mapLogicalToAbsolute(event.GetY())));
                    }
                }
            }
            render();
        }
    } else {
        SetCursor(wxCURSOR_DEFAULT);
    }
}

void xlGridCanvasPictures::mouseLeftUp(wxMouseEvent& event)
{
    mouseUp();
}

void xlGridCanvasPictures::mouseRightUp(wxMouseEvent& event)
{
    mouseUp();
}

void xlGridCanvasPictures::mouseUp()
{
    if (mEffect == nullptr) return;
    if (mDragging) {
        ReleaseMouse();
        mDragging = false;
    }
    mRightDown = false;
    mLeftDown = false;
}


void xlGridCanvasPictures::render( wxPaintEvent& event )
{
    wxPaintDC dc(this);
    render();
}
void xlGridCanvasPictures::render()
{
    if (!IsShownOnScreen()) {
        return;
    }
    if (!mIsInitialized) {
        PrepareCanvas();
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (mEffect != nullptr) {
        DrawPicturesEffect(ctx);
        DrawBaseGrid(ctx);
        DrawSelection(ctx);
    }
    FinishDrawing(ctx);
}

void xlGridCanvasPictures::DrawPicturesEffect(xlGraphicsContext *ctx)
{
    if (NewPictureName == "") {
        return;
    }
    if (NewPictureName != PictureName || !imagesValid) {
        if (imageTexture) {
            delete imageTexture;
        }
        if (imageCopy) {
            delete imageCopy;
            imageCopy = nullptr;
        }
        imageTexture = ctx->createTexture(image);

        if (mCopyAvailable) {
            imageCopy = ctx->createTexture(image_copy);
        }
    }
    if (imageTexture == nullptr) {
        return;
    }
    float mCellSize = getCellSize();
    float offset = mapLogicalToAbsolute(5.0);

    ctx->enableBlending();
    float mrow = mRows - image.GetHeight();
    float starty = offset + mrow * mCellSize;
    ctx->drawTexture(imageTexture, offset, starty, offset + mCellSize * image.GetWidth(), starty + mCellSize * image.GetHeight(),
                     0, 0, 1, 1,
                     false);
    if (imageCopy) {
        float startx = offset + mStartCol * mCellSize;
        float starty = offset + mStartRow * mCellSize;

        ctx->drawTexture(imageCopy, startx, starty,
                         startx + mCellSize * image_copy.GetWidth() - 1.0,
                         starty + mCellSize * image_copy.GetHeight() - 1.0,
                         0, 0, 1, 1,
                         false);
    }


    if (NewPictureName != PictureName) {
        PictureName = NewPictureName;
    }
}

void xlGridCanvasPictures::CalcSelection()
{
    mStartCol = GetCellFromPosition(mDragStartX);
    mStartRow = GetCellFromPosition(mDragStartY);
    mEndCol = GetCellFromPosition(mDragEndX);
    mEndRow = GetCellFromPosition(mDragEndY);
    if (mStartRow > mEndRow) {
        std::swap(mStartRow, mEndRow);
    }
    if (mStartCol > mEndCol) {
        std::swap(mStartCol, mEndCol);
    }
}
void xlGridCanvasPictures::DrawSelection(xlGraphicsContext *ctx)
{
    if (mPaintMode == PAINT_SELECTCOPY && mDragStartX > 0) {
        float mCellSize = getCellSize();
        float offset = mapLogicalToAbsolute(5.0);

        int start_x = ((float)mStartCol) * mCellSize + offset;
        int start_y = ((float)mStartRow) * mCellSize + offset;
        int height = mCellSize * (mEndRow - mStartRow + 1);
        int width = mCellSize * (mEndCol - mStartCol + 1);

        if (mCopyAvailable) {
            width = mCellSize * image_copy.GetWidth();
            height = mCellSize * image_copy.GetHeight();
        }

        auto *va = ctx->createVertexAccumulator();
        va->AddRectAsTriangles(start_x, start_y, start_x + width, start_y + height);
        xlColor c = *mSelectionColor;
        c.alpha = 70;

        ctx->enableBlending();
        ctx->drawTriangles(va, c);
        va->Reset();
        va->AddRectAsDashedLines(start_x, start_y, start_x + width, start_y + height, mapLogicalToAbsolute(8.0));
        ctx->drawLines(va, xlYELLOW);

        ctx->disableBlending();
        delete va;
    }
}

void xlGridCanvasPictures::ProcessHoverDrag(int column, int row)
{
    float mCellSize = getCellSize();
    if (column > mHoverDragCol) {
        int columns_to_move = column - mHoverDragCol;
        if (mEndCol + columns_to_move < mColumns) {
            mDragStartX += mCellSize*columns_to_move;
            mDragEndX += mCellSize*columns_to_move;
            mHoverDragCol += columns_to_move;
        }
    } else if (column < mHoverDragCol) {
        int columns_to_move = mHoverDragCol - column;
        if (mStartCol - columns_to_move >= 0) {
            mDragStartX -= mCellSize*columns_to_move;
            mDragEndX -= mCellSize*columns_to_move;
            mHoverDragCol -= columns_to_move;
        }
    }
    if (row > mHoverDragRow) {
        int rows_to_move = row - mHoverDragRow;
        if (mEndRow + rows_to_move < mRows) {
            mDragStartY += mCellSize*rows_to_move;
            mDragEndY += mCellSize*rows_to_move;
            mHoverDragRow += rows_to_move;
        }
    } else if (row < mHoverDragRow) {
        int rows_to_move = mHoverDragRow - row;
        if (mStartRow - rows_to_move >= 0) {
            mDragStartY -= mCellSize*rows_to_move;
            mDragEndY -= mCellSize*rows_to_move;
            mHoverDragRow -= rows_to_move;
        }
    }
    CalcSelection();
}

void xlGridCanvasPictures::Copy()
{
    int srow = mStartRow - image.GetHeight();
    int erow = mEndRow - image.GetHeight();
    if (srow < 0) {
        srow = 0;
    }
    if (erow < 0) {
        return;
    }

    int height = erow - srow + 1;
    int width = mEndCol - mStartCol + 1;

    wxRect rect;
    rect.x = mStartCol;
    rect.y = srow;
    rect.width = width;
    rect.height = height;

    image_copy = image.GetSubImage(rect);
    imagesValid = false;
    if (image_copy.IsOk()) {
        mCopyAvailable = true;
    }
}

void xlGridCanvasPictures::Paste()
{
    if (mCopyAvailable) {
        int srow = mStartRow - image.GetHeight();
        image.Paste(image_copy, mStartCol, srow);
        mCopyAvailable = false;
        imagesValid = false;
        render();
    }
}

void xlGridCanvasPictures::Cancel()
{
    if (mCopyAvailable) {
        mCopyAvailable = false;
        imagesValid = false;
        render();
    }
}
