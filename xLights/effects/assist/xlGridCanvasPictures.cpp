#include "xlGridCanvasPictures.h"
#include "../../BitmapCache.h"
#include "../../DrawGLUtils.h"
#include <wx/filefn.h>
#include <wx/filename.h>

#include "../../ResizeImageDialog.h"

//static const wxString strSupportedImageTypes = "PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPG files(*.jpg)|*.jpg|All files (*.*)|*.*";
static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg|All files (*.*)|*.*";

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
    use_ping(true),
    scaleh(0.0),
    scalew(0.0),
    imageGL_ping(NULL),
    imageGL_pong(NULL),
    mImage(NULL),
    mImageCopy(NULL),
    sprite(NULL),
    copy_sprite(NULL),
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
    mPasteCopy(false)
{
}

xlGridCanvasPictures::~xlGridCanvasPictures()
{
    delete mSelectionColor;
}

void xlGridCanvasPictures::LoadAndProcessImage()
{

    // process loading new image
    if( mModified )
    {
        PictureName = wxEmptyString;
        mModified = false;
    }

    if ( NewPictureName != PictureName )
    {
        wxLogNull logNo;  // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
        imageCount = wxImage::GetImageCount(NewPictureName);
        imageIndex = 0;
        if (!image.LoadFile(NewPictureName,wxBITMAP_TYPE_ANY,0))
        {
            image.Create(mColumns, mRows, true);
        }
        if (!image.IsOk())
            return;
    }
    ProcessNewImage();
}
void xlGridCanvasPictures::ProcessNewImage()
{
    wxString image_size = wxEmptyString;

    wxCommandEvent eventImage(EVT_IMAGE_FILE_SELECTED);
    eventImage.SetClientData(&NewPictureName);
    wxPostEvent(mMessageParent, eventImage);

    int imgwidth=image.GetWidth();
    int imght   =image.GetHeight();

    image_size = wxString::Format("Image Size: %d x %d", imgwidth, imght);
    wxCommandEvent eventImageSize(EVT_IMAGE_SIZE);
    eventImageSize.SetString(image_size);
    wxPostEvent(mMessageParent, eventImageSize);

    if( imgwidth > mColumns || imght > mRows )
    {
        if( imageCount > 1 )
        {
            img_mode = IMAGE_MULTIPLE_OVERSIZED;
        }
        else
        {
            img_mode = IMAGE_SINGLE_OVERSIZED;
        }
    }
    else
    {
        if( imageCount > 1 )
        {
            img_mode = IMAGE_MULTIPLE_FITS;
        }
        else
        {
            img_mode = IMAGE_SINGLE_FITS;
        }
    }
    Refresh(false);
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
    if( result == wxID_CANCEL || new_filename == "" ) {
        return;
    }

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
    if( result == wxID_CANCEL || new_filename == "" ) {
        return;
    }
    wxFileName file_check(new_filename);
    if( file_check.GetExt() == "" )
    {
        file_check.SetExt("png");
    }
    save_name = file_check.GetFullPath();

    if( wxFile::Exists(save_name)) {
        if( wxMessageBox("Are you sure you want to overwrite this image file?\n" + save_name, "Confirm Overwrite?", wxICON_QUESTION | wxYES_NO) == wxYES )
        {
            PictureName = save_name;
            SaveImageToFile();
        }
        else
        {
            return;
        }
    }
    else
    {
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
            if( sprite != NULL ) {
                delete sprite;
                sprite = NULL;
            }
            ProcessNewImage();
        }
    }

}

void xlGridCanvasPictures::SaveImage()
{
    wxFileName save_file(PictureName);
    wxString save_name = PictureName;
    if( save_file.GetFullName() == "NewImage.png" )
    {
        // prompt for new filename
        wxFileDialog fd(this,
                        "Choose filename to Save Image:",
                        save_file.GetPath(),
                        wxEmptyString,
                        wxFileSelectorDefaultWildcardStr,
                        wxFD_SAVE);
        int result = fd.ShowModal();
        wxString new_filename = fd.GetPath();
        if( result == wxID_CANCEL || new_filename == "" ) {
            return;
        }
        wxFileName file_check(new_filename);
        if( file_check.GetExt() == "" )
        {
            file_check.SetExt("png");
        }
        save_name = file_check.GetFullPath();
    }

    if( wxFile::Exists(save_name)) {
        if( wxMessageBox("Are you sure you want to overwrite this image file?\n" + save_name, "Confirm Overwrite?", wxICON_QUESTION | wxYES_NO) == wxYES )
        {
            PictureName = save_name;
            SaveImageToFile();
        }
        else
        {
            return;
        }
    }
    else
    {
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
    for( int s = 0; s < all_settings.size(); s++ )
    {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if( parts[0] == "E_FILEPICKER_Pictures_Filename" ) {
            parts[1] = PictureName;
        }
        all_settings[s] = wxJoin(parts, '=');
    }
    settings = wxJoin(all_settings, ',');
    mEffect->SetSettings(settings.ToStdString());

    wxCommandEvent eventEffectChanged(EVT_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(mEffect);
    wxPostEvent(mMessageParent, eventEffectChanged);
}

void xlGridCanvasPictures::CreateNewImage(wxString& image_dir)
{
    if( image_dir == wxEmptyString )
    {
        wxMessageBox("Error creating new image file.  Image Directory is not defined.");
        return;
    }
    wxFileName new_file;
    new_file.SetPath(image_dir);
    new_file.SetFullName("NewImage.png");
    NewPictureName = new_file.GetFullPath();
    if( wxFile::Exists(NewPictureName)) {
        ::wxRemoveFile(NewPictureName);
    }
    image.Create(mColumns, mRows, true);
    image.SaveFile(NewPictureName, wxBITMAP_TYPE_PNG);

    if (!image.IsOk())
    {
        wxMessageBox("Error creating image file!");
        return;
    }
    mModified = true;
    ProcessNewImage();
}

void xlGridCanvasPictures::ForceRefresh()
{
    NewPictureName = GetImageFilename();
    LoadAndProcessImage();
}

void xlGridCanvasPictures::SetEffect(Effect* effect_)
{
    static wxString missing_file = wxEmptyString;

    mEffect = effect_;

    if( mEffect == NULL ) return;

    NewPictureName = GetImageFilename();

    if( NewPictureName == "" ) return;

    if( wxFile::Exists(NewPictureName)) {
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
    for( int s = 0; s < all_settings.size(); s++ )
    {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if( parts[0] == "E_FILEPICKER_Pictures_Filename" ) {
            return parts[1];
        }

    }
    return wxEmptyString;
}

void xlGridCanvasPictures::mouseRightDown(wxMouseEvent& event)
{
    if( !mLeftDown )
    {
        mRightDown = true;
        mouseDown(event.GetX(), event.GetY());
    }
}

void xlGridCanvasPictures::mouseLeftDown(wxMouseEvent& event)
{
    SetFocus();
    if( !mRightDown )
    {
        mLeftDown = true;
        mouseDown(event.GetX(), event.GetY());
    }
}

void xlGridCanvasPictures::mouseDown(int x, int y)
{
    if( mEffect == NULL ) return;

    if( img_mode == IMAGE_SINGLE_FITS )
    {
        int column =  GetCellFromPosition(x);
        int row = GetCellFromPosition(y);
        if( column >= 0 && column < mColumns && row >= 0 && row < mRows )
        {
            int draw_row = mRows - row - 1;
            row = image.GetHeight() - draw_row - 1;
            SetCurrentGLContext();
            if (column < image.GetWidth() && draw_row < image.GetHeight()) {
                if( mPaintMode == PAINT_PENCIL && !mRightDown ) {
                    DrawGLUtils::UpdateTexturePixel(mImage->getID(), (double)column, (double)draw_row, mPaintColor, mImage->hasAlpha());
                    image.SetRGB(column, row, mPaintColor.red, mPaintColor.green, mPaintColor.blue);
                    if( mImage->hasAlpha() ) {
                        image.SetAlpha(column, row, mPaintColor.alpha);
                    }
                } else if( (mPaintMode == PAINT_ERASER || mRightDown) && !mHoverSelection ) {
                    DrawGLUtils::UpdateTexturePixel(mImage->getID(), (double)column, (double)draw_row, mEraseColor, mImage->hasAlpha());
                    image.SetRGB(column, row, mEraseColor.red, mEraseColor.green, mEraseColor.blue);
                    if( mImage->hasAlpha() ) {
                        image.SetAlpha(column, row, mEraseColor.alpha);
                    }
                } else if( mPaintMode == PAINT_EYEDROPPER && !mRightDown ) {
                    xlColor eyedrop_color;
                    eyedrop_color.red = image.GetRed(column, row);
                    eyedrop_color.green = image.GetGreen(column, row);
                    eyedrop_color.blue = image.GetBlue(column, row);
                    wxCommandEvent eventEyedrop(EVT_EYEDROPPER_COLOR);
                    eventEyedrop.SetInt(eyedrop_color.GetRGB());
                    wxPostEvent(mMessageParent, eventEyedrop);
                } else if( mPaintMode == PAINT_SELECTCOPY ) {
                    if( !mRightDown ) {
                        if( !mHoverSelection )
                        {
                            mDragStartX = x;
                            mDragStartY = y;
                            mDragEndX = x;
                            mDragEndY = y;
                            CalcSelection();
                            mCopyAvailable = false;
                        }
                        else
                        {
                            mHoverDragCol = GetCellFromPosition(x);
                            mHoverDragRow = GetCellFromPosition(y);
                        }
                    }
                    else if( mHoverSelection ) {
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
        Refresh(false);
    }
}

void xlGridCanvasPictures::mouseMoved(wxMouseEvent& event)
{
    if( mEffect == NULL ) return;

    int column =  GetCellFromPosition(event.GetX());
    int row = GetCellFromPosition(event.GetY());
    if( img_mode == IMAGE_SINGLE_FITS && column >= 0 && column < mColumns && row >= 0 && row < mRows )
    {
        if( mRightDown )
        {
            SetCursor(wxCURSOR_PAINT_BRUSH);
        }
        else
        {
            switch(mPaintMode)
            {
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
                if( !mDragging )
                {
                    if( column >= mStartCol && column <= mEndCol && row >= mStartRow && row <= mEndRow )
                    {
                        SetCursor(wxCURSOR_HAND);
                        mHoverSelection = true;
                    }
                    else
                    {
                        SetCursor(wxCURSOR_CROSS);
                        mHoverSelection = false;
                    }
                }
                break;
            }
        }

        if( mDragging )
        {
            int draw_row = mRows - row - 1;
            row = image.GetHeight() - draw_row - 1;
            SetCurrentGLContext();
            if (column < image.GetWidth() && draw_row < image.GetHeight()) {
                if( mPaintMode == PAINT_PENCIL && !mRightDown ) {
                    DrawGLUtils::UpdateTexturePixel(mImage->getID(), (double)column, (double)draw_row, mPaintColor, mImage->hasAlpha());
                    image.SetRGB(column, row, mPaintColor.red, mPaintColor.green, mPaintColor.blue);
                    if( mImage->hasAlpha() ) {
                        image.SetAlpha(column, row, mPaintColor.alpha);
                    }
                } else if( (mPaintMode == PAINT_ERASER || mRightDown) && !mHoverSelection ) {
                    DrawGLUtils::UpdateTexturePixel(mImage->getID(), (double)column, (double)draw_row, mEraseColor, mImage->hasAlpha());
                    image.SetRGB(column, row, mEraseColor.red, mEraseColor.green, mEraseColor.blue);
                    if( mImage->hasAlpha() ) {
                        image.SetAlpha(column, row, mEraseColor.alpha);
                    }
                } else if( mPaintMode == PAINT_EYEDROPPER && !mRightDown ) {
                    xlColor eyedrop_color;
                    eyedrop_color.red = image.GetRed(column, row);
                    eyedrop_color.green = image.GetGreen(column, row);
                    eyedrop_color.blue = image.GetBlue(column, row);
                    wxCommandEvent eventEyedrop(EVT_EYEDROPPER_COLOR);
                    eventEyedrop.SetClientData(&eyedrop_color);
                    wxPostEvent(mMessageParent, eventEyedrop);
                } else if( mPaintMode == PAINT_SELECTCOPY && !mRightDown ) {
                    if( !mHoverSelection )
                    {
                        mDragEndX = event.GetX();
                        mDragEndY = event.GetY();
                        CalcSelection();
                    }
                    else
                    {
                        ProcessHoverDrag(column, row);
                    }
                }
            }
            Refresh(false);
            Update();
        }
    }
    else
    {
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
    if( mEffect == NULL ) return;
    if( mDragging )
    {
        ReleaseMouse();
        mDragging = false;
    }
    mRightDown = false;
    mLeftDown = false;
}

void xlGridCanvasPictures::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 0.0f)); // Black Background
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glDisable(GL_DEPTH_TEST));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    mIsInitialized = true;
}

void xlGridCanvasPictures::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }

    if( mEffect != nullptr )
    {
        DrawPicturesEffect();
        DrawBaseGrid();
        DrawSelection();
    }
    SwapBuffers();
}

void xlGridCanvasPictures::DrawPicturesEffect()
{
    if( NewPictureName == "" ) return;
    if( NewPictureName != PictureName || sprite == NULL || mPasteCopy)
    {
        if( sprite != NULL ) {
            delete sprite;
        }
        if( use_ping )
        {
            imageGL_ping = new Image(image);
            sprite = new xLightsDrawable(imageGL_ping);
            imageWidth = imageGL_ping->width;
            imageHeight = imageGL_ping->height;
            mImage = imageGL_ping;
        }
        else
        {
            imageGL_pong = new Image(image);
            sprite = new xLightsDrawable(imageGL_pong);
            imageWidth = imageGL_pong->width;
            imageHeight = imageGL_pong->height;
            mImage = imageGL_pong;
        }
        sprite->setFlip(false, false);
    }
    sprite->scale(mCellSize, mCellSize);
    sprite->setHotspot(-1, -mRows - 1 + imageHeight);

    DrawGLUtils::PushMatrix();

    if( mPaintMode != PAINT_SELECTCOPY || !mCopyAvailable )
    {
        sprite->render();
    }
    else
    {
        LOG_GL_ERRORV(glColor4f(1.0f, 1.0f, 1.0f, 0.8f));
        LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));   // textures
        sprite->render();
        LOG_GL_ERRORV(glColor4f(1.0f, 1.0f, 1.0f, 0.5f));
        LOG_GL_ERRORV(glEnable(GL_BLEND));
        LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        copy_sprite->render();
        LOG_GL_ERRORV(glDisable(GL_BLEND));
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
    }

    DrawGLUtils::PopMatrix();

    if( NewPictureName != PictureName || mPasteCopy)
    {
        mPasteCopy = false;
        if( use_ping ) {
            if( imageGL_pong != NULL ) {
                delete imageGL_pong;
            }
            use_ping = false;
        } else {
            if( imageGL_ping != NULL ) {
                delete imageGL_ping;
            }
            use_ping = true;
        }
        PictureName = NewPictureName;
    }
}

void xlGridCanvasPictures::CalcSelection()
{
    mStartCol = GetCellFromPosition(mDragStartX);
    mStartRow = GetCellFromPosition(mDragStartY);
    mEndCol = GetCellFromPosition(mDragEndX);
    mEndRow = GetCellFromPosition(mDragEndY);
    if( mStartRow > mEndRow ) {
        std::swap( mStartRow, mEndRow );
    }
    if( mStartCol > mEndCol ) {
        std::swap( mStartCol, mEndCol );
    }
}
void xlGridCanvasPictures::DrawSelection()
{
    if( mPaintMode == PAINT_SELECTCOPY && mDragStartX > 0 ) {
        int start_x = (mStartCol+1)*mCellSize;
        int start_y = (mStartRow+1)*mCellSize;
        int height = mCellSize * (mEndRow-mStartRow+1);
        int width = mCellSize * (mEndCol-mStartCol+1);
        LOG_GL_ERRORV(glEnable(GL_BLEND));
        DrawGLUtils::DrawFillRectangle(*mSelectionColor,70,start_x,start_y,width,height);
        DrawGLUtils::DrawRectangle(xlYELLOW,true,start_x,start_y,start_x+width,start_y+height);
        LOG_GL_ERRORV(glDisable(GL_BLEND));
    }
}

void xlGridCanvasPictures::ProcessHoverDrag(int column, int row)
{
    if( column > mHoverDragCol )
    {
        int columns_to_move = column - mHoverDragCol;
        if( mEndCol + columns_to_move < mColumns )
        {
            mDragStartX += mCellSize*columns_to_move;
            mDragEndX += mCellSize*columns_to_move;
            mHoverDragCol += columns_to_move;
        }
    }
    else if( column < mHoverDragCol )
    {
        int columns_to_move = mHoverDragCol - column;
        if( mStartCol - columns_to_move >= 0 )
        {
            mDragStartX -= mCellSize*columns_to_move;
            mDragEndX -= mCellSize*columns_to_move;
            mHoverDragCol -= columns_to_move;
        }
    }
    if( row > mHoverDragRow )
    {
        int rows_to_move = row - mHoverDragRow;
        if( mEndRow + rows_to_move < mRows )
        {
            mDragStartY += mCellSize*rows_to_move;
            mDragEndY += mCellSize*rows_to_move;
            mHoverDragRow += rows_to_move;
        }
    }
    else if( row < mHoverDragRow )
    {
        int rows_to_move = mHoverDragRow - row;
        if( mStartRow - rows_to_move >= 0 )
        {
            mDragStartY -= mCellSize*rows_to_move;
            mDragEndY -= mCellSize*rows_to_move;
            mHoverDragRow -= rows_to_move;
        }
    }
    CalcSelection();
    if( mCopyAvailable )
    {
        copy_sprite->setHotspot(-1 - mStartCol, -1 - mStartRow);
    }
}

void xlGridCanvasPictures::Copy()
{
    SetCurrentGLContext();

    if( mImageCopy != NULL ) {
        delete mImageCopy;
    }

    int height = mEndRow-mStartRow+1;
    int width = mEndCol-mStartCol+1;

    wxRect rect;
    rect.x = mStartCol;
    rect.y = mStartRow;
    rect.width = width;
    rect.height = height;

    image_copy = image.GetSubImage(rect);
    mImageCopy = new Image(image_copy);
    if( copy_sprite != NULL ) {
        delete copy_sprite;
    }
    copy_sprite = new xLightsDrawable(mImageCopy);
    copy_sprite->setHotspot(-1 - mStartCol, -1 - mStartRow);
    copy_sprite->setFlip(false, false);
    mCopyAvailable = true;
}

void xlGridCanvasPictures::Paste()
{
    if( mCopyAvailable )
    {
        image.Paste(image_copy, mStartCol, mStartRow);
        mPasteCopy = true;
        Refresh(false);
        Update();
    }
}
