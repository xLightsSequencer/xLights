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
    EVT_SYS_COLOUR_CHANGED(ModelPreview::OnSysColourChanged)
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

void ModelPreview::OnZoomGesture(wxZoomGestureEvent& event) {
    if (!event.IsGestureStart()) {
        float delta = (m_last_mouse_x - (event.GetZoomFactor() * 1000)) / 1000.0;
        SetZoomDelta(delta > 0.0 ? 0.1f : -0.1f);
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED) {
                Refresh();
                Update();
            }
        }
    }
    m_last_mouse_x = (event.GetZoomFactor() * 1000);
}

void ModelPreview::mouseMoved(wxMouseEvent& event) {
	if (m_mouse_down) {
		int delta_x = event.GetPosition().x - m_last_mouse_x;
		int delta_y = event.GetPosition().y - m_last_mouse_y;
		SetCameraView(delta_x, delta_y, false);
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED) {
                Refresh();
                Update();
            }
        }
    } else if (m_wheel_down) {
        float new_x = event.GetX() - m_last_mouse_x;
        float new_y = event.GetY() - m_last_mouse_y;
        if (!is_3d) {
            new_y *= -1.0f;
        }
        // account for grid rotation
        float angleX = glm::radians(GetCameraRotationX());
        float angleY = glm::radians(GetCameraRotationY());
        float delta_x = 0.0f;
        float delta_y = 0.0f;
        float delta_z = 0.0f;
        bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
        bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
        bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
        if( top_view ) {
            delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
            delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
        } else if( bottom_view ) {
            delta_x = -new_x * std::sin(angleY) - new_y * std::cos(angleY);
            delta_z = new_y * std::sin(angleY) - new_x * std::cos(angleY);
        } else {
            delta_x = new_x * std::cos(angleY);
            delta_y = new_y;
            delta_z = new_x * std::sin(angleY);
            if( !upside_down_view && is_3d) {
                delta_y *= -1.0f;
            }
        }
        delta_x *= GetZoom() * 2.0f;
        delta_y *= GetZoom() * 2.0f;
        delta_z *= GetZoom() * 2.0f;
        SetPan(delta_x, delta_y, delta_z);
        m_last_mouse_x = event.GetX();
        m_last_mouse_y = event.GetY();
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED) {
                Refresh();
                Update();
            }
        }
    }
    Model *model = xlights ? xlights->GetModel(currentModel) : nullptr;
    if (model != nullptr) {
        wxString tip = model->GetNodeNear(this, event.GetPosition());
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
    m_mouse_down = false;
    m_wheel_down = false;
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}
const std::vector<Model*> &ModelPreview::GetModels() {
    tmpModelList.clear();
    if (xlights) {
        if (currentLayoutGroup == "Default") {
            if (additionalModel == nullptr) {
                return xlights->PreviewModels;
            } else {
                tmpModelList = xlights->PreviewModels;
            }
        } else if (currentLayoutGroup == "All Models") {
            for (auto a : xlights->AllModels) {
                if (a.second->GetDisplayAs() != "ModelGroup") {
                    tmpModelList.push_back(a.second);
                }
            }
        } else if (currentLayoutGroup == "Unassigned") {
            for (auto a : xlights->AllModels) {
                if (a.second->GetLayoutGroup() == "Unassigned") {
                    if (a.second->GetDisplayAs() != "ModelGroup") {
                        tmpModelList.push_back(a.second);
                    }
                }
            }
        } else {
            bool foundGrp = false;
            for (auto grp : xlights->LayoutGroups) {
                if (currentLayoutGroup == grp->GetName()) {
                    foundGrp = true;
                    if (additionalModel == nullptr) {
                        return grp->GetModels();
                    } else {
                        tmpModelList = grp->GetModels();
                    }
                    break;
                }
            }
            if (!foundGrp) {
                tmpModelList = xlights->PreviewModels;
            }
        }
    }
    if (additionalModel != nullptr) {
        tmpModelList.push_back(additionalModel);
    }
    return tmpModelList;
}
void ModelPreview::SetModel(const Model* model) {
    if (model) {
        this->xlights = model->GetModelManager().GetXLightsFrame();
        currentModel = model->GetName();
    } else {
        currentModel = "";
    }
}

void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {
    if (event.GetWheelRotation() == 0) {
        //rotation of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    bool fromTrackPad = IsMouseEventFromTouchpad();
    if (!fromTrackPad || event.ControlDown()) {
        SetZoomDelta(event.GetWheelRotation() > 0 ? -0.1f : 0.1f);
    } else {
        float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
        float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;
        if (is_3d) {
            if (event.ShiftDown()) {
                SetPan(delta_x, delta_y, 0.0f);
            } else {
                SetCameraView(delta_x, delta_y, false);
                SetCameraView(0, 0, true);
            }
        } else {
            // account for grid rotation
            float angle = glm::radians(GetCameraRotationY());
            float new_x = delta_x;
            float new_y = delta_y;
            delta_x = new_x * std::cos(angle) - new_y * std::sin(angle);
            delta_y = new_y * std::cos(angle) + new_x * std::sin(angle);
            delta_x *= GetZoom() * 2.0f;
            delta_y *= GetZoom() * 2.0f;
            SetPan(delta_x, delta_y, 0.0f);
            m_last_mouse_x = event.GetX();
            m_last_mouse_y = event.GetY();
        }
    }

    if (xlights != nullptr) {
        if(xlights->GetPlayStatus() == PLAY_TYPE_STOPPED) {
            Refresh();
            Update();
        }
    }
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

    if (currentModel == "&---none---&") {
        if (!StartDrawing(mPointSize, true)) return;
        Render();
        EndDrawing();
    } else {
        Model *model = xlights ? xlights->GetModel(currentModel) : nullptr;
        if (model != nullptr) {
            model->DisplayEffectOnWindow(this, 2);
        } else {
            if (!StartDrawing(mPointSize, true)) return;
            EndDrawing();
        }
    }
}

void ModelPreview::Render()
{
    const std::vector<Model*> &models = GetModels();
    if (!models.empty()) {
        bool isModelSelected = false;
        for (auto m : models) {
            if (m->Selected || m->GroupSelected) {
                isModelSelected = true;
            }
        }
        const xlColor *defColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
        const xlColor *selColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
        const xlColor *overlapColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_OVERLAP);
        for (auto m : models) {
            const xlColor *color = defColor;
            if (m->Selected || m->GroupSelected) {
                color = selColor;
            } else if (m->Overlapping && isModelSelected) {
                color = overlapColor;
            }
            if (!allowSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
            }
            if (is_3d) {
                m->DisplayModelOnWindow(this, accumulator3d, true, color, allowSelected);
            } else {
                m->DisplayModelOnWindow(this, accumulator, false, color, allowSelected);
                // FIXME:  Delete when not needed for debugging
                //if ((*PreviewModels)[i]->Highlighted) {
                //    (*PreviewModels)[i]->GetModelScreenLocation().DrawBoundingBox(accumulator);
                //}
            }
        }
    }
    // draw all the view objects
    if (is_3d) {
        for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
            ViewObject *view_object = it->second;
            view_object->Draw(this, view_object_accumulator, allowSelected);
        }
    }
}

void ModelPreview::Render(const unsigned char *data, bool swapBuffers/*=true*/) {
    if (StartDrawing(mPointSize)) {
        const std::vector<Model*> &models = GetModels();
        for (auto m : models) {
            int NodeCnt = m->GetNodeCount();
            for (size_t n = 0; n < NodeCnt; ++n) {
                int start = m->NodeStartChannel(n);
                m->SetNodeChannelValues(n, &data[start]);
            }
            if (is_3d)
                m->DisplayModelOnWindow(this, accumulator3d, true);
            else
                m->DisplayModelOnWindow(this, accumulator, false);
        }
        // draw all the view objects
        if (is_3d) {
            for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
                ViewObject *view_object = it->second;
                view_object->Draw(this, view_object_accumulator, allowSelected);
            }
        }
        EndDrawing(swapBuffers);
    }
}

void ModelPreview::rightClick(wxMouseEvent& event) {
    if (allowPreviewChange && xlights != nullptr) {
        wxMenu mnu;
        mnu.Append(0x2001, "Reset");
        wxMenuItem* item = mnu.Append(0x1001, "3D", wxEmptyString, wxITEM_CHECK);
        item->Check(is_3d);
        mnu.AppendSeparator();
        mnu.Append(1, "House Preview");
        int index = 2;
        for (auto a : xlights->LayoutGroups) {
            mnu.Append(index++, a->GetName());
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
    if (xlights) {
        if (id == 0) {
            currentLayoutGroup = "Default";
            SetBackgroundBrightness(xlights->GetDefaultPreviewBackgroundBrightness(), xlights->GetDefaultPreviewBackgroundAlpha());
            SetbackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
        } else if (id > 0 && id <= xlights->LayoutGroups.size()) {
            currentLayoutGroup = xlights->LayoutGroups[id - 1]->GetName();
            SetBackgroundBrightness(xlights->LayoutGroups[id - 1]->GetBackgroundBrightness(), xlights->LayoutGroups[id - 1]->GetBackgroundAlpha());
            SetbackgroundImage(xlights->LayoutGroups[id - 1]->GetBackgroundImage());
        }
    }
    if (id == 0x2000)
    {
        Reset();
    }
    else if (id == 0x1000) {
        is_3d = !is_3d;
    } else if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId()) {
                    SetCamera3D(i);
                    break;
                }
            }
        }
    } else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId()) {
                    SetCamera2D(i);
                    break;
                }
            }
        }
    }
    Refresh();
    Update();
}

// reset cameras
void ModelPreview::Reset()
{
    // Reset
    if (is_3d)
    {
        camera3d->Reset();
    }
    else
    {
        camera2d->Reset();
    }
}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame* xlights_, bool a, int styles, bool apc)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", false),
    virtualWidth(0), virtualHeight(0), _display2DBox(false), _center2D0(false),
    image(nullptr), sprite(nullptr), allowSelected(a), allowPreviewChange(apc), mPreviewPane(nullptr),
    xlights(xlights_), currentModel("&---none---&"),  currentLayoutGroup("Default"), additionalModel(nullptr), is_3d(false), m_mouse_down(false), m_wheel_down(false),
    m_last_mouse_x(-1), m_last_mouse_y(-1), camera3d(nullptr), camera2d(nullptr), maxVertexCount(5000)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    setupCameras();
    
    if (!allowSelected) {
        EnableTouchEvents(wxTOUCH_ZOOM_GESTURE);
        Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&ModelPreview::OnZoomGesture, nullptr, this);
    }
}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame *xl)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ModelPreview", false),
    virtualWidth(0), virtualHeight(0), _display2DBox(false), _center2D0(false),
    image(nullptr), sprite(nullptr), allowSelected(false), allowPreviewChange(false), mPreviewPane(nullptr),
    xlights(xl), currentModel(""), currentLayoutGroup("Default"), additionalModel(nullptr), is_3d(false), m_mouse_down(false), m_wheel_down(false),
    m_last_mouse_x(-1), m_last_mouse_y(-1), camera3d(nullptr), camera2d(nullptr), maxVertexCount(5000)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    setupCameras();
    
    EnableTouchEvents(wxTOUCH_ZOOM_GESTURE);
    Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&ModelPreview::OnZoomGesture, nullptr, this);
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

void ModelPreview::InitializePreview(wxString img, int brightness, int alpha, bool center2d0)
{
    _center2D0 = center2d0;
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
    mBackgroundAlpha = alpha;
}

static inline wxColor GetBackgroundColor() {
    wxColor c = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
#ifdef __WXOSX__
    if (!c.IsSolid()) {
        c = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
    }
    if (!c.IsSolid()) {
        c.Set(204, 204, 204);
    }
#endif
    return c;
}
void ModelPreview::InitializeGLCanvas()
{
    SetCurrentGLContext();

    if (allowSelected) {
        wxColor c = GetBackgroundColor();
        LOG_GL_ERRORV(glClearColor(c.Red()/255.0f, c.Green()/255.0f, c.Blue()/255.0, 1.0f));
    } else {
        LOG_GL_ERRORV(glClearColor(0.0, 0.0, 0.0, 1.0f)); // Black Background
    }
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    mIsInitialized = true;
}
void ModelPreview::OnSysColourChanged(wxSysColourChangedEvent& event) {
    if (mIsInitialized) {
        SetCurrentGLContext();
        wxColor c = GetBackgroundColor();
        LOG_GL_ERRORV(glClearColor(c.Red()/255.0f, c.Green()/255.0f, c.Blue()/255.0, 1.0f));
    }
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

void ModelPreview::SetBackgroundBrightness(int brightness, int alpha)
{
    mBackgroundBrightness = brightness;
    mBackgroundAlpha = alpha;
    if(mBackgroundBrightness < 0 || mBackgroundBrightness > 100) {
        mBackgroundBrightness = 100;
    }
    if(mBackgroundAlpha < 0 || mBackgroundAlpha > 100) {
        mBackgroundAlpha = 100;
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

bool ModelPreview::GetActive() const
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
    } else {
        camera2d->SetZoom(camera2d->GetZoom() * (1.0f - delta));
        camera2d->SetZoomCorrX(((mWindowWidth * camera2d->GetZoom()) - mWindowWidth) / 2.0f);
        camera2d->SetZoomCorrY(((mWindowHeight * camera2d->GetZoom()) - mWindowHeight) / 2.0f);
    }
}

void ModelPreview::SetPan(float deltax, float deltay, float deltaz)
{
    if (is_3d) {
        camera3d->SetPanX(camera3d->GetPanX() + deltax);
        camera3d->SetPanY(camera3d->GetPanY() + deltay);
        camera3d->SetPanZ(camera3d->GetPanZ() + deltaz);
    } else {
        camera2d->SetPanX(camera2d->GetPanX() + deltax);
        camera2d->SetPanY(camera2d->GetPanY() + deltay);
    }
}

bool ModelPreview::StartDrawing(wxDouble pointSize, bool fromPaint)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!fromPaint && !IsShownOnScreen()) return false;
    if (!mIsInitialized) { InitializeGLCanvas(); }
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    SetCurrentGLContext();

    /*****************************   2D   ********************************/
    if (!is_3d) {
        glDisable(GL_DEPTH_TEST);
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
        if (_center2D0) {
            glm::mat4 cTranslate =  glm::translate(glm::mat4(1.0f), glm::vec3(((float)virtualWidth) / 2.0f, 0.0f, 0.0f));
            ViewMatrix = ViewTranslate * ViewScale * cTranslate;
        }
        ProjMatrix = glm::ortho(0.0f, (float)mWindowWidth, 0.0f, (float)mWindowHeight);  // this must match prepare2DViewport call
        ProjViewMatrix = ProjMatrix * ViewMatrix;

        prepare2DViewport(0, mWindowHeight, mWindowWidth, 0);
        LOG_GL_ERRORV(glClearColor(0, 0, 0, 0));   // background color
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::SetCamera(ViewMatrix);
        DrawGLUtils::PushMatrix();
        accumulator.PreAlloc(maxVertexCount);
        currentPixelScaleFactor = 1.0;
        if (!allowSelected && virtualWidth > 0 && virtualHeight > 0
            && (virtualWidth != mWindowWidth || virtualHeight != mWindowHeight)) {
            currentPixelScaleFactor = scale2d;
            LOG_GL_ERRORV(glPointSize(calcPixelSize(mPointSize)));
        }
        accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
        accumulator.Finish(GL_TRIANGLES);
    } else {
        /*****************************   3D   ********************************/
        glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(camera3d->GetPosX() + camera3d->GetPanX(), camera3d->GetPosY() + camera3d->GetPanY(), camera3d->GetPosZ() + camera3d->GetPanZ()));
        glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, camera3d->GetDistance() * camera3d->GetZoom()));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleX()), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleY()), glm::vec3(0.0f, 1.0f, 0.0f));
        ViewMatrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewTranslatePan;
        ProjMatrix = glm::perspective(glm::radians(45.0f), (float)translateToBacking(mWindowWidth) / (float)translateToBacking(mWindowHeight), 1.0f, 20000.0f);  // this must match prepare3DViewport call
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
        accumulator3d.PreAlloc(maxVertexCount);
        currentPixelScaleFactor = 1.0;
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

        float i = mBackgroundBrightness;
        float a = mBackgroundAlpha * 255.0f;
        a /= 100;
        accumulator.FinishTextures(GL_TRIANGLES, image->getID(), (uint8_t)a, i);
    }

    // Draw a box around the default area in 2D
    if (!is_3d && allowSelected && _display2DBox) {
        if (_center2D0) {
            float x = -virtualWidth;
            x /= 2.0f;
            accumulator.AddLinesRect(x, 0, x + virtualWidth - 1, virtualHeight - 1, xlGREENTRANSLUCENT);
        } else {
            accumulator.AddLinesRect(0, 0, virtualWidth - 1, virtualHeight - 1, xlGREENTRANSLUCENT);
        }
        accumulator.Finish(GL_LINES);
    }

    return true;
}

void ModelPreview::EndDrawing(bool swapBuffers/*=true*/)
{
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    if (is_3d) {
        if (accumulator3d.count > maxVertexCount) {
            maxVertexCount = accumulator3d.count;
        }
        DrawGLUtils::Draw(view_object_accumulator, DrawGLUtils::xlGLCacheInfo::DrawType::SOLIDS);
        DrawGLUtils::Draw(accumulator3d);
        DrawGLUtils::Draw(view_object_accumulator, DrawGLUtils::xlGLCacheInfo::DrawType::TRANSPARENTS);
    } else {
        if (accumulator.count > maxVertexCount) {
            maxVertexCount = accumulator.count;
        }
        DrawGLUtils::Draw(accumulator);
    }
    DrawGLUtils::PopMatrix();
    if (swapBuffers)
    {
        logger_opengl.debug("About to swap buffers in ModelPreview::EndDrawing.");
        LOG_GL_ERRORV(SwapBuffers());
        logger_opengl.debug("Done swapping buffers in ModelPreview::EndDrawing.");
    }
    view_object_accumulator.Reset();
    accumulator3d.Reset();
    accumulator.Reset();
    mIsDrawing = false;
}
