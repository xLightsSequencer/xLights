#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/artprov.h>

#ifdef __WXMAC__
    #include "OpenGL/gl.h"
#else
    #include <GL/gl.h>
#endif

#include "ModelPreview.h"
#include "models/Model.h"
#include "models/ViewObject.h"
#include "PreviewPane.h"
#include "DrawGLUtils.h"
#include "osxMacUtils.h"
#include "ColorManager.h"
#include "LayoutGroup.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(ModelPreview, xlGLCanvas)
EVT_MOTION(ModelPreview::mouseMoved)
EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
EVT_LEFT_UP(ModelPreview::mouseLeftUp)
EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
EVT_RIGHT_DOWN(ModelPreview::rightClick)
//EVT_KEY_DOWN(ModelPreview::keyPressed)
//EVT_KEY_UP(ModelPreview::keyReleased)
EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
EVT_MIDDLE_DOWN(ModelPreview::mouseMiddleDown)
EVT_MIDDLE_UP(ModelPreview::mouseMiddleUp)
EVT_PAINT(ModelPreview::render)
END_EVENT_TABLE()

const long ModelPreview::ID_VIEWPOINT2D = wxNewId();
const long ModelPreview::ID_VIEWPOINT3D = wxNewId();

static glm::mat4 Identity(glm::mat4(1.0f));

void ModelPreview::setupCameras()
{
    camera3d = new PreviewCamera(true);
    camera2d = new PreviewCamera(false);
}

void ModelPreview::SetCamera2D(int i)
{
    *camera2d = *(xlights->viewpoint_mgr.GetCamera2D(i));
}

void ModelPreview::SetCamera3D(int i)
{
    *camera3d = *(xlights->viewpoint_mgr.GetCamera3D(i));
    SetCameraPos(0,0,false,true);
    SetCameraView(0,0,false,true);
}

void ModelPreview::SaveCurrentCameraPosition()
{
    wxTextEntryDialog dlg(this, "Enter a name for this ViewPoint", "ViewPoint Name", "");
    bool name_ok = false;
    while (!name_ok) {
        if (dlg.ShowModal() == wxID_OK) {
            const std::string name = dlg.GetValue().ToStdString();
            if (name != "" && xlights->viewpoint_mgr.IsNameUnique(name, is_3d)) {
                PreviewCamera* current_camera = (is_3d ? camera3d : camera2d);
                xlights->viewpoint_mgr.AddCamera( name, current_camera, is_3d );
                name_ok = true;
            }
            else {
                wxMessageDialog msgDlg(this, "Error: ViewPoint name is a duplicate or blank.", "Error", wxOK | wxCENTRE);
                msgDlg.ShowModal();
            }
        }
        else {
            name_ok = true; // cancelled operation
        }
    }
}

void ModelPreview::mouseMoved(wxMouseEvent& event) {
	if (m_mouse_down) {
		int delta_x = event.GetPosition().x - m_last_mouse_x;
		int delta_y = event.GetPosition().y - m_last_mouse_y;
		SetCameraView(delta_x, delta_y, false);
        Render();
    }
    else if (m_wheel_down) {
        float new_x = event.GetX() - m_last_mouse_x;
        float new_y = event.GetY() - m_last_mouse_y;
        if (!is_3d) {
            new_y *= -1.0f;
        }
        // account for grid rotation
        float angle = glm::radians(GetCameraRotation());
        float delta_x = new_x * std::cos(angle) - new_y * std::sin(angle);
        float delta_y = new_y * std::cos(angle) + new_x * std::sin(angle);
        delta_x *= GetZoom() * 2.0f;
        delta_y *= GetZoom() * 2.0f;
        SetPan(delta_x, delta_y);
        m_last_mouse_x = event.GetX();
        m_last_mouse_y = event.GetY();
        Render();
    }

    if (_model != nullptr)
    {
        wxString tip =_model->GetNodeNear(this, event.GetPosition());
        SetToolTip(tip);
    }

    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftDown(wxMouseEvent& event) {
    SetFocus();
	m_mouse_down = true;
	m_last_mouse_x = event.GetX();
	m_last_mouse_y = event.GetY();

	event.ResumePropagation(1);
	event.Skip(); // continue the event
}

void ModelPreview::mouseLeftUp(wxMouseEvent& event) {
	m_mouse_down = false;
	SetCameraView(0, 0, true);

	event.ResumePropagation(1);
	event.Skip(); // continue the event
}

void ModelPreview::mouseLeftWindow(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {
    int i = event.GetWheelRotation();
    float delta = -0.1f;
    if (i < 0)
    {
        delta *= -1.0f;
    }
    SetZoomDelta(delta);
    Render();

    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelPreview::mouseMiddleDown(wxMouseEvent& event) {
    m_wheel_down = true;
    m_last_mouse_x = event.GetX();
    m_last_mouse_y = event.GetY();
}

void ModelPreview::mouseMiddleUp(wxMouseEvent& event) {
    m_wheel_down = false;
}

void ModelPreview::render(wxPaintEvent& event)
{
    if (mIsDrawing) return;

    //if(!mIsInitialized) { InitializeGLCanvas(); }
    //SetCurrentGLContext();
    //wxPaintDC(this);

    if (_model != nullptr) {
        _model->DisplayEffectOnWindow(this, 2);
    }
    else {
        if (!StartDrawing(mPointSize)) return;
        Render();
        EndDrawing();
    }
}

void ModelPreview::Render()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // draw all the view objects
    if( is_3d ) {
        for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
            ViewObject *view_object = it->second;
            view_object->Draw(this, view_object_accumulator, allowSelected);
        }
    }

    if (PreviewModels != nullptr)
    {
        bool isModelSelected = false;
        for (int i = 0; i < PreviewModels->size(); ++i) {
            if ((*PreviewModels)[i] == nullptr)
            {
                logger_base.crit("*PreviewModels[%d] is nullptr ... this is not going to end well.", i);
            }
            if (((*PreviewModels)[i])->Selected || ((*PreviewModels)[i])->GroupSelected) {
                isModelSelected = true;
                break;
            }
        }

        for (int i = 0; i < PreviewModels->size(); ++i) {
            const xlColor *color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
            if (((*PreviewModels)[i])->Selected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
            }
            else if (((*PreviewModels)[i])->GroupSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
            }
            else if (((*PreviewModels)[i])->Overlapping && isModelSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_OVERLAP);
            }
            if (!allowSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
            }
            if (is_3d) {
                (*PreviewModels)[i]->DisplayModelOnWindow(this, accumulator3d, true, color, allowSelected);
            }
            else {
                (*PreviewModels)[i]->DisplayModelOnWindow(this, accumulator, false, color, allowSelected);
                // FIXME:  Delete when not needed for debugging
                //if ((*PreviewModels)[i]->Highlighted) {
                //    (*PreviewModels)[i]->GetModelScreenLocation().DrawBoundingBox(accumulator);
                //}
            }
        }
    }
}

void ModelPreview::Render(const unsigned char *data, bool swapBuffers/*=true*/) {
    if (StartDrawing(mPointSize)) {
        // draw all the view objects
        if (is_3d) {
            for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
                ViewObject *view_object = it->second;
                view_object->Draw(this, view_object_accumulator, allowSelected);
            }
        }
        if (PreviewModels != nullptr) {
            for (int m = 0; m < PreviewModels->size(); m++) {
                int NodeCnt = (*PreviewModels)[m]->GetNodeCount();
                for (size_t n = 0; n < NodeCnt; ++n) {
                    int start = (*PreviewModels)[m]->NodeStartChannel(n);
                    (*PreviewModels)[m]->SetNodeChannelValues(n, &data[start]);
                }
                if (is_3d)
                    (*PreviewModels)[m]->DisplayModelOnWindow(this, accumulator3d, true);
                else
                    (*PreviewModels)[m]->DisplayModelOnWindow(this, accumulator, false);
            }
        }
        EndDrawing(swapBuffers);
    }
}

void ModelPreview::rightClick(wxMouseEvent& event) {
    if (allowPreviewChange && xlights != nullptr) {
        wxMenu mnu;
        wxMenuItem* item = mnu.Append(0x1001, "3D", wxEmptyString, wxITEM_CHECK);
        item->Check(is_3d);
        mnu.AppendSeparator();
        mnu.Append(1, "House Preview");
        int index = 2;
        for (auto it = LayoutGroups->begin(); it != LayoutGroups->end(); ++it) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            mnu.Append(index++, grp->GetName());
        }
        // ViewPoint menus
        mnu.AppendSeparator();
        if (is_3d) {
            if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
                wxMenu* mnuViewPoint = new wxMenu();
                for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i) {
                    PreviewCamera* camera = xlights->viewpoint_mgr.GetCamera3D(i);
                    mnuViewPoint->Append(camera->GetMenuId(), camera->GetName());
                }
                mnuViewPoint->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ModelPreview::OnPopup, nullptr, this);
                mnu.Append(ID_VIEWPOINT3D, "Load ViewPoint", mnuViewPoint, "");
            }
        } else {
            if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
                wxMenu* mnuViewPoint = new wxMenu();
                for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i) {
                    PreviewCamera* camera = xlights->viewpoint_mgr.GetCamera2D(i);
                    mnuViewPoint->Append(camera->GetMenuId(), camera->GetName());
                }
                mnuViewPoint->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ModelPreview::OnPopup, nullptr, this);
                mnu.Append(ID_VIEWPOINT2D, "Load ViewPoint", mnuViewPoint, "");
            }
        }
        mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ModelPreview::OnPopup, nullptr, this);
        PopupMenu(&mnu);
    }
}

void ModelPreview::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId() - 1;
    if (id == 0) {
        SetModels(*HouseModels);
        SetBackgroundBrightness(xlights->GetDefaultPreviewBackgroundBrightness());
        SetbackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
    }
    else if (id > 0 && id <= LayoutGroups->size()) {
        SetModels((*LayoutGroups)[id - 1]->GetModels());
        SetBackgroundBrightness((*LayoutGroups)[id - 1]->GetBackgroundBrightness());
        SetbackgroundImage((*LayoutGroups)[id - 1]->GetBackgroundImage());
    }
    else if (id == 0x1000) {
        is_3d = !is_3d;
    }
    else if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId())
                {
                    SetCamera3D(i);
                    break;
                }
            }
        }
    }
    else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId())
                {
                    SetCamera2D(i);
                    break;
                }
            }
        }
    }
    Refresh();
    Update();
}

void ModelPreview::keyPressed(wxKeyEvent& event) {}
void ModelPreview::keyReleased(wxKeyEvent& event) {}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame* xlights_, std::vector<Model*> &models, std::vector<LayoutGroup *> &groups, bool a, int styles, bool apc)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", false),
      image(nullptr), PreviewModels(&models), HouseModels(&models), LayoutGroups(&groups), allowSelected(a), allowPreviewChange(apc), xlights(xlights_),
      m_mouse_down(false), m_wheel_down(false), is_3d(false), camera3d(nullptr), camera2d(nullptr)
{
    maxVertexCount = 5000;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    virtualWidth = 0;
    virtualHeight = 0;
    sprite = nullptr;
    _model = nullptr;
    setupCameras();
}

ModelPreview::ModelPreview(wxPanel* parent)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ModelPreview", false), PreviewModels(nullptr), allowSelected(false), image(nullptr)
{
    _model = nullptr;
    maxVertexCount = 5000;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    virtualWidth = 0;
    virtualHeight = 0;
    is_3d = false;
    setupCameras();
    image = nullptr;
    sprite = nullptr;
    xlights = nullptr;
}

ModelPreview::~ModelPreview()
{
    if (camera2d) {
        delete camera2d;
    }
    if (camera3d) {
        delete camera3d;
    }

    if (image) {
        if (cache) {
            cache->AddTextureToDelete(image->getID());
            image->setID(0);
        }
        delete image;
    }
    if (sprite) {
        delete sprite;
    }
}

void ModelPreview::SetCanvasSize(int width,int height)
{
    SetVirtualCanvasSize(width, height);
}

void ModelPreview::SetVirtualCanvasSize(int width, int height) {
    virtualWidth = width;
    virtualHeight = height;
}

void ModelPreview::InitializePreview(wxString img, int brightness)
{
    if (img != mBackgroundImage) {
        if (image) {
            if (cache) {
                cache->AddTextureToDelete(image->getID());
                image->setID(0);
            }
            delete image;
            image = nullptr;
        }
        if (sprite) {
            delete sprite;
            sprite = nullptr;
        }
        mBackgroundImage = img;
        mBackgroundImageExists = wxFileExists(mBackgroundImage) && wxIsReadable(mBackgroundImage) ? true : false;
    }
    mBackgroundBrightness = brightness;
}

void ModelPreview::InitializeGLCanvas()
{
    if (!IsShownOnScreen()) return;
    SetCurrentGLContext();

    if (allowSelected) {
        wxColor c = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
        LOG_GL_ERRORV(glClearColor(c.Red()/255.0f, c.Green()/255.0f, c.Blue()/255.0, 1.0f)); // Black Background
    } else {
        LOG_GL_ERRORV(glClearColor(0.0, 0.0, 0.0, 1.0f)); // Black Background
    }
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    mIsInitialized = true;
}

void ModelPreview::SetOrigin()
{
}

void ModelPreview::SetScaleBackgroundImage(bool b) {
    scaleImage = b;
    Refresh();
}

void ModelPreview::SetbackgroundImage(wxString img)
{
    if (img != mBackgroundImage) {
        ObtainAccessToURL(img.ToStdString());
        if (image) {
            if (cache) {
                cache->AddTextureToDelete(image->getID());
                image->setID(0);
            }
            delete image;
            image = nullptr;
        }
        if (sprite) {
            delete sprite;
            sprite = nullptr;
        }
        mBackgroundImage = img;
        mBackgroundImageExists = wxFileExists(mBackgroundImage) && wxIsReadable(mBackgroundImage) ? true : false;
    }
}

void ModelPreview::SetBackgroundBrightness(int brightness)
{
    mBackgroundBrightness = brightness;
    if(mBackgroundBrightness < 0 || mBackgroundBrightness > 100) {
        mBackgroundBrightness = 100;
    }
}

void ModelPreview::SetPointSize(wxDouble pointSize)
{
    mPointSize = pointSize;
    LOG_GL_ERRORV(glPointSize( mPointSize ));
}

double ModelPreview::calcPixelSize(double i) {
    double d = translateToBacking(i * currentPixelScaleFactor);
    if (d < 1.0) {
        d = 1.0;
    }
    return d;
}

bool ModelPreview::GetActive()
{
    return mPreviewPane->GetActive();
}

void ModelPreview::render(const wxSize& size/*wxSize(0,0)*/)
{
    wxPaintEvent dummyEvent;
    wxSize origSize(0, 0);
    wxSize origVirtSize(virtualWidth, virtualHeight);
    if (size != wxSize(0, 0)) {
        origSize = wxSize(mWindowWidth, mWindowHeight);
        mWindowWidth = ((float)size.GetWidth() / GetContentScaleFactor());
        mWindowHeight = ((float)size.GetHeight() / GetContentScaleFactor());
        float mult = float(mWindowWidth) / origSize.GetWidth();
        virtualWidth = int(mult * origVirtSize.GetWidth());
        virtualHeight = int(mult * origVirtSize.GetHeight());
    }

    render(dummyEvent);

    if (origSize != wxSize(0, 0)) {
        mWindowWidth = origSize.GetWidth();
        mWindowHeight = origSize.GetHeight();
        virtualWidth = origVirtSize.GetWidth();
        virtualHeight = origVirtSize.GetHeight();
    }
}

void ModelPreview::SetActive(bool show) {
    if (show) {
        mPreviewPane->Show();
    }
    else {
        mPreviewPane->Hide();
    }
}

void ModelPreview::SetCameraView(int camerax, int cameray, bool latch, bool reset)
{
	static int last_offsetx = 0;
	static int last_offsety = 0;
	static int latched_x = camera3d->GetAngleX();
	static int latched_y = camera3d->GetAngleY();

	if( reset ) {
        last_offsetx = 0;
        last_offsety = 0;
        latched_x = camera3d->GetAngleX();
        latched_y = camera3d->GetAngleY();
	} else {
        if (latch) {
            camera3d->SetAngleX(latched_x + last_offsetx);
            camera3d->SetAngleY(latched_y + last_offsety);
            latched_x = camera3d->GetAngleX();
            latched_y = camera3d->GetAngleY();
            last_offsetx = 0;
            last_offsety = 0;
        }
        else {
            camera3d->SetAngleX(latched_x + cameray / 2);
            camera3d->SetAngleY(latched_y + camerax / 2);
            last_offsetx = cameray / 2;
            last_offsety = camerax / 2;
        }
	}
}

void ModelPreview::SetCameraPos(int camerax, int cameraz, bool latch, bool reset)
{
	static int last_offsetx = 0;
	static int last_offsety = 0;
	static int latched_x = camera3d->GetPosX();
	static int latched_y = camera3d->GetPosY();

	if( reset ) {
        last_offsetx = 0;
        last_offsety = 0;
        latched_x = camera3d->GetPosX();
        latched_y = camera3d->GetPosY();
	} else {
        if (latch) {
            camera3d->SetPosX(latched_x + last_offsetx);
            camera3d->SetPosY(latched_y + last_offsety);
            latched_x = camera3d->GetPosX();
            latched_y = camera3d->GetPosY();
        }
        else {
            camera3d->SetPosX(latched_x + camerax);
            camera3d->SetPosY(latched_y + cameraz);
            last_offsetx = camerax;
            last_offsety = cameraz;
        }
	}
}

void ModelPreview::SetZoomDelta(float delta)
{
    if (is_3d) {
        camera3d->SetZoom( camera3d->GetZoom() * (1.0f + delta));
    }
    else {
        camera2d->SetZoom(camera2d->GetZoom() * (1.0f - delta));
        camera2d->SetZoomCorrX(((mWindowWidth * camera2d->GetZoom()) - mWindowWidth) / 2.0f);
        camera2d->SetZoomCorrY(((mWindowHeight * camera2d->GetZoom()) - mWindowHeight) / 2.0f);
    }
}

void ModelPreview::SetPan(float deltax, float deltay)
{
    if (is_3d) {
        camera3d->SetPanX(camera3d->GetPanX() + deltax);
        camera3d->SetPanY(camera3d->GetPanY() + deltay);
    }
    else {
        camera2d->SetPanX(camera2d->GetPanX() + deltax);
        camera2d->SetPanY(camera2d->GetPanY() + deltay);
    }
}

bool ModelPreview::StartDrawing(wxDouble pointSize)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!IsShownOnScreen()) return false;
    if (!mIsInitialized) { InitializeGLCanvas(); }
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    SetCurrentGLContext();
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));

    /*****************************   2D   ********************************/
    if (!is_3d) {
        float scale2d = 1.0f;
        float scale_corrx = 0.0f;
        float scale_corry = 0.0f;
        if (virtualWidth != 0 && virtualHeight != 0) {
            float scale2dh = (float)mWindowHeight / (float)virtualHeight;
            float scale2dw = (float)mWindowWidth / (float)virtualWidth;
            // maintain aspect ratio.... FIXME: maybe add as an option
            // centers the direction that is smaller
            if (scale2dh < scale2dw) {
                scale2d = scale2dh;
                scale_corrx = ((scale2dw*(float)virtualWidth - (scale2d*(float)virtualWidth)) * camera2d->GetZoom()) / 2.0f;
            }
            else {
                scale2d = scale2dw;
                scale_corry = ((scale2dh*(float)virtualHeight - (scale2d*(float)virtualHeight)) * camera2d->GetZoom()) / 2.0f;
            }
        }
        glm::mat4 ViewScale = glm::scale(glm::mat4(1.0f), glm::vec3(camera2d->GetZoom() * scale2d, camera2d->GetZoom() * scale2d, 1.0f));
        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(camera2d->GetPanX()*camera2d->GetZoom() - camera2d->GetZoomCorrX() + scale_corrx, camera2d->GetPanY()*camera2d->GetZoom() - camera2d->GetZoomCorrY() + scale_corry, 0.0f));
        ViewMatrix = ViewTranslate * ViewScale;
        ProjMatrix = glm::ortho(0.0f, (float)mWindowWidth, 0.0f, (float)mWindowHeight);  // this must match prepare2DViewport call
        ProjViewMatrix = ProjMatrix * ViewMatrix;

        prepare2DViewport(0, mWindowHeight, mWindowWidth, 0);
        LOG_GL_ERRORV(glClearColor(0, 0, 0, 0));   // background color
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::SetCamera(ViewMatrix);
        DrawGLUtils::PushMatrix();
        accumulator.PreAlloc(maxVertexCount);
        currentPixelScaleFactor = 1.0;
        if (!allowSelected && virtualWidth > 0 && virtualHeight > 0
            && (virtualWidth != mWindowWidth || virtualHeight != mWindowHeight)) {
            currentPixelScaleFactor = scale2d;
            LOG_GL_ERRORV(glPointSize(calcPixelSize(mPointSize)));
            accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
            accumulator.Finish(GL_TRIANGLES);
        }

        else {
            accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
            accumulator.Finish(GL_TRIANGLES);
        }
    }

    /*****************************   3D   ********************************/
    if (is_3d) {
        glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(camera3d->GetPosX() + camera3d->GetPanX(), 1.0f, camera3d->GetPosY() + camera3d->GetPanY()));
        glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, camera3d->GetDistance() * camera3d->GetZoom()));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleX()), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleY()), glm::vec3(0.0f, 1.0f, 0.0f));
        ViewMatrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewTranslatePan;
        ProjMatrix = glm::perspective(glm::radians(45.0f), (float)mWindowWidth / (float)mWindowHeight, 1.0f, 20000.0f);  // this must match prepare3DViewport call
        ProjViewMatrix = ProjMatrix * ViewMatrix;

        // FIXME: commented out for debugging speed
        // FIXME: transparent background does not draw correctly when depth testing enabled
        // enables depth testing to draw things in proper order
        glEnable(GL_DEPTH_TEST);
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glDepthFunc(GL_LESS);

        prepare3DViewport(0, mWindowHeight, mWindowWidth, 0);
        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::SetCamera(ViewMatrix);
        DrawGLUtils::PushMatrix();
        accumulator.PreAlloc(maxVertexCount);
        currentPixelScaleFactor = 1.0;
        if( mBackgroundImageExists ) {
            accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
            accumulator.Finish(GL_TRIANGLES);
        }
    }

    if (mBackgroundImageExists) {
        if (image == nullptr) {
            logger_base.debug("Loading background image file %s for preview %s.",
                              (const char *)mBackgroundImage.c_str(),
                              (const char *)GetName().c_str());
            image = new Image(mBackgroundImage);
            sprite = new xLightsDrawable(image);
        }
        float scaleh = 1.0;
        float scalew = 1.0;
        if (!scaleImage) {
            float nscaleh = float(image->height) / float(virtualHeight);
            float nscalew = float(image->width) / float(virtualWidth);
            if (nscalew < nscaleh) {
                scaleh = 1.0;
                scalew = nscalew / nscaleh;
            } else {
                scaleh = nscaleh / nscalew;
                scalew = 1.0;
            }
        }
        accumulator.PreAllocTexture(6);
        float tx1 = 0;
        float tx2 = image->tex_coord_x;
        accumulator.AddTextureVertex(0, 0, tx1, -0.5 / (image->textureHeight));
        accumulator.AddTextureVertex(virtualWidth * scalew, 0, tx2, -0.5 / (image->textureHeight));
        accumulator.AddTextureVertex(0, virtualHeight * scaleh, tx1, image->tex_coord_y);

        accumulator.AddTextureVertex(0, virtualHeight * scaleh, tx1, image->tex_coord_y);
        accumulator.AddTextureVertex(virtualWidth * scalew, 0, tx2, -0.5 / (image->textureHeight));
        accumulator.AddTextureVertex(virtualWidth * scalew, virtualHeight *scaleh, tx2, image->tex_coord_y);

        int i = mBackgroundBrightness * 255 / 100;
        accumulator.FinishTextures(GL_TRIANGLES, image->getID(), i);
    }
    return true;
}

void ModelPreview::EndDrawing(bool swapBuffers/*=true*/)
{
    if (accumulator.count > maxVertexCount) {
        maxVertexCount = accumulator.count;
    }
    DrawGLUtils::Draw(view_object_accumulator);
    DrawGLUtils::Draw(accumulator);
    DrawGLUtils::Draw(accumulator3d);
    DrawGLUtils::PopMatrix();
    if (swapBuffers)
    {
        LOG_GL_ERRORV(SwapBuffers());
    }
    view_object_accumulator.Reset();
    accumulator3d.Reset();
    accumulator.Reset();
    mIsDrawing = false;
}

