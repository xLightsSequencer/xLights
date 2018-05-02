#include "wx/wx.h"
#include "wx/sizer.h"
#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
//#ifdef _MSC_VER
//#include "GL/glut.h"
//#else
#include <GL/gl.h>
//#endif
#endif

#include "ModelPreview.h"
#include "models/Model.h"
#include "PreviewPane.h"
#include "DrawGLUtils.h"
#include "osxMacUtils.h"
#include "ColorManager.h"
#include "LayoutGroup.h"
#include "xLightsMain.h"
#include <wx/artprov.h>

BEGIN_EVENT_TABLE(ModelPreview, xlGLCanvas)
EVT_MOTION(ModelPreview::mouseMoved)
EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
EVT_LEFT_UP(ModelPreview::mouseLeftUp)
EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
EVT_RIGHT_DOWN(ModelPreview::rightClick)
//EVT_KEY_DOWN(ModelPreview::keyPressed)
//EVT_KEY_UP(ModelPreview::keyReleased)
EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
EVT_PAINT(ModelPreview::render)
END_EVENT_TABLE()

void ModelPreview::mouseMoved(wxMouseEvent& event) {
	if (m_mouse_down) {
		int delta_x = event.GetPosition().x - m_last_mouse_x;
		int delta_y = event.GetPosition().y - m_last_mouse_y;
		SetCameraView(delta_x, delta_y, false);
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
    event.ResumePropagation(1);
    event.Skip(); // continue the event
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
            if( is_3d )
                (*PreviewModels)[i]->DisplayModelOnWindow(this, accumulator3d, true, color, allowSelected);
            else
                (*PreviewModels)[i]->DisplayModelOnWindow(this, accumulator, false, color, allowSelected);
        }
    }
}

void ModelPreview::Render(const unsigned char *data, bool swapBuffers/*=true*/) {
    if (StartDrawing(mPointSize)) {
        if (PreviewModels != nullptr) {
            for (int m = 0; m < PreviewModels->size(); m++) {
                int NodeCnt = (*PreviewModels)[m]->GetNodeCount();
                for (size_t n = 0; n < NodeCnt; ++n) {
                    int start = (*PreviewModels)[m]->NodeStartChannel(n);
                    (*PreviewModels)[m]->SetNodeChannelValues(n, &data[start]);
                }
                if( is_3d )
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
        wxMenu mnuSelectPreview;
        wxMenuItem* item = mnuSelectPreview.Append(0x1001, "3D", wxEmptyString, wxITEM_CHECK);
        item->Check(is_3d);
        mnuSelectPreview.AppendSeparator();
        mnuSelectPreview.Append(1, "House Preview");
        int index = 2;
        for (auto it = LayoutGroups->begin(); it != LayoutGroups->end(); ++it) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            mnuSelectPreview.Append(index++, grp->GetName());
        }
        mnuSelectPreview.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ModelPreview::OnPopup, nullptr, this);
        PopupMenu(&mnuSelectPreview);
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
    Refresh();
    Update();
}

void ModelPreview::keyPressed(wxKeyEvent& event) {}
void ModelPreview::keyReleased(wxKeyEvent& event) {}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame* xlights_, std::vector<Model*> &models, std::vector<LayoutGroup *> &groups, bool a, int styles, bool apc)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", true),
      PreviewModels(&models), HouseModels(&models), LayoutGroups(&groups), allowSelected(a), allowPreviewChange(apc), xlights(xlights_), m_mouse_down(false), is_3d(false)
{
    maxVertexCount = 5000;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    virtualWidth = 0;
    virtualHeight = 0;
    image = nullptr;
    sprite = nullptr;
    _model = nullptr;
    cameraAngleX = 20;
    cameraAngleY = 5;
	cameraDistance = -2000.0f;
    cameraPosX = -500;
    cameraPosY = 0;
    zoom = 1.0f;
    zoom2D = 1.0f;
    panx = 0.0f;
    pany = 0.0f;
    panx2D = 0.0f;
    pany2D = 0.0f;
    zoom_corrx2D = 0.0f;
    zoom_corry2D = 0.0f;
}

ModelPreview::ModelPreview(wxPanel* parent)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ModelPreview", true), PreviewModels(nullptr), allowSelected(false), image(nullptr)
{
    _model = nullptr;
    maxVertexCount = 5000;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    virtualWidth = 0;
    virtualHeight = 0;
    image = nullptr;
    sprite = nullptr;
    xlights = nullptr;
}

ModelPreview::~ModelPreview()
{
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
        LOG_GL_ERRORV(glClearColor(0.8f, 0.8f, 0.8f, 1.0f)); // Black Background
    }
    else {
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
   if(mBackgroundBrightness < 0 || mBackgroundBrightness > 100)
   {
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

void ModelPreview::SetCameraView(int camerax, int cameray, bool latch)
{
	static int last_offsetx = 0;
	static int last_offsety = 0;
	static int latched_x = cameraAngleX;
	static int latched_y = cameraAngleY;

	if (latch) {
		cameraAngleX = latched_x + last_offsetx;
		cameraAngleY = latched_y + last_offsety;
		latched_x = cameraAngleX;
		latched_y = cameraAngleY;
        last_offsetx = 0;
        last_offsety = 0;
    }
	else {
		cameraAngleX = latched_x + cameray;
		cameraAngleY = latched_y + camerax;
		last_offsetx = cameray;
		last_offsety = camerax;
	}
}

void ModelPreview::SetCameraPos(int camerax, int cameray, bool latch)
{
	static int last_offsetx = 0;
	static int last_offsety = 0;
	static int latched_x = cameraPosX;
	static int latched_y = cameraPosY;

	if (latch) {
		cameraPosX = latched_x + last_offsetx;
		cameraPosY = latched_y + last_offsety;
		latched_x = cameraPosX;
		latched_y = cameraPosY;
	}
	else {
		cameraPosX = latched_x + camerax;
		cameraPosY = latched_y + cameray;
		last_offsetx = camerax;
		last_offsety = cameray;
	}
}

void ModelPreview::SetZoomDelta(float delta)
{
    if (is_3d) {
        zoom *= 1.0f + delta;
    }
    else {
        zoom2D *= 1.0f - delta;
        zoom_corrx2D = ((mWindowWidth * zoom2D) - mWindowWidth) / 2.0f;
        zoom_corry2D = ((mWindowHeight * zoom2D) - mWindowHeight) / 2.0f;
    }
}

void ModelPreview::SetPan(float deltax, float deltay)
{
    if (is_3d) {
        panx += deltax;
        pany += deltay;
    }
    else {
        panx2D += deltax;
        pany2D += deltay;
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

    if (is_3d) {
        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(cameraPosX + (panx * zoom), cameraPosY + (pany * zoom), cameraDistance * zoom));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cameraAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
        ViewMatrix = ViewTranslate * ViewRotateX * ViewRotateY;
        ProjMatrix = glm::perspective(glm::radians(45.0f), (float)(mWindowWidth - 0.0f) / (float)(mWindowHeight - 0.0f), 1.0f, 10000.0f);
    }
    else {
        glm::mat4 ViewScale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom2D, zoom2D, 1.0f));
        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(panx2D*zoom2D - zoom_corrx2D, (-(float)virtualHeight + pany2D)*zoom2D + zoom_corry2D, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 ViewRotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ViewMatrix = ViewRotateZ * ViewRotateY * ViewTranslate * ViewScale;
        ProjMatrix = glm::ortho((float)0, (float)mWindowWidth, (float)mWindowHeight, (float)0);
    }

    if (!allowSelected && virtualWidth > 0 && virtualHeight > 0
        && (virtualWidth != mWindowWidth || virtualHeight != mWindowHeight)) {
		if (is_3d)
		{
            prepare3DViewport(0, 0, mWindowWidth, mWindowHeight);
			LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
			DrawGLUtils::PushMatrix();
            glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(cameraPosX, cameraPosY, cameraDistance * zoom));
            glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(cameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(cameraAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
            ViewMatrix = ViewTranslate * ViewRotateX * ViewRotateY;
            DrawGLUtils::SetCamera(ViewMatrix);
            accumulator.PreAlloc(maxVertexCount);
			currentPixelScaleFactor = 1.0;
			accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
			accumulator.Finish(GL_TRIANGLES);
			drawGrid(mWindowWidth, mWindowWidth / 40);
		}
		else
		{
			prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
            DrawGLUtils::SetCamera(glm::mat4(1.0));
			LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
			DrawGLUtils::PushMatrix();
			// Rotate Axis and translate
			DrawGLUtils::Rotate(180,0,0,1);
			DrawGLUtils::Rotate(180,0,1,0);
			accumulator.PreAlloc(maxVertexCount);
			currentPixelScaleFactor = 1.0;
			int i = (int)mWindowHeight;
			DrawGLUtils::Translate(0,-i,0);
			double scaleh= double(mWindowHeight) / double(virtualHeight);
			double scalew = double(mWindowWidth) / double(virtualWidth);
			DrawGLUtils::Scale(scalew, scaleh, 1.0);

			if (scalew < scaleh) {
				scaleh = scalew;
			}
			currentPixelScaleFactor = scaleh;
			LOG_GL_ERRORV(glPointSize(calcPixelSize(mPointSize)));
			accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
			accumulator.Finish(GL_TRIANGLES);

		}
    } else if (virtualWidth == 0 && virtualHeight == 0) {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
        DrawGLUtils::SetCamera(glm::mat4(1.0));

        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::PushMatrix();
        // Rotate Axis and translate
        DrawGLUtils::Rotate(180,0,0,1);
        DrawGLUtils::Rotate(180,0,1,0);
        accumulator.PreAlloc(maxVertexCount);
        currentPixelScaleFactor = 1.0;
        int i = (int)mWindowHeight;
        DrawGLUtils::Translate(0, -i, 0);
    } else {
        if (is_3d) {
            prepare3DViewport(0,0,mWindowWidth, mWindowHeight);
            LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
            DrawGLUtils::PushMatrix();
            DrawGLUtils::SetCamera(ViewMatrix);
		    accumulator.PreAlloc(maxVertexCount);
            currentPixelScaleFactor = 1.0;
		    accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
		    accumulator.Finish(GL_TRIANGLES);
		    drawGrid(mWindowWidth, mWindowWidth / 40);
        }
        else {
            prepare2DViewport(0, 0, mWindowWidth, mWindowHeight);
            LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
            DrawGLUtils::PushMatrix();
            DrawGLUtils::SetCamera(ViewMatrix);
            accumulator.PreAlloc(maxVertexCount);
            currentPixelScaleFactor = 1.0;
            accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
            accumulator.Finish(GL_TRIANGLES);
        }
	}

    if (mBackgroundImageExists)
    {
        if (image == nullptr)
        {
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
            }
            else {
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
	DrawGLUtils::Draw(gridlines, xlColor(0, 128, 0), GL_LINES);
	DrawGLUtils::Draw(accumulator);
	DrawGLUtils::Draw(accumulator3d);
	DrawGLUtils::PopMatrix();
	 if (swapBuffers)
	 {
		 LOG_GL_ERRORV(SwapBuffers());
	 }
	 gridlines.Reset();
	 accumulator3d.Reset();
	 accumulator.Reset();
    mIsDrawing = false;
}

///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xz plane
///////////////////////////////////////////////////////////////////////////////
void ModelPreview::drawGrid(float size, float step)
{
	gridlines.PreAlloc(size/step * 24);

	for (float i = 0; i <= size; i += step)
	{
		gridlines.AddVertex(-size, 0, i);   // lines parallel to X-axis
		gridlines.AddVertex(size, 0, i);
		gridlines.AddVertex(-size, 0, -i);   // lines parallel to X-axis
		gridlines.AddVertex(size, 0, -i);

		gridlines.AddVertex(i, 0, -size);   // lines parallel to Z-axis
		gridlines.AddVertex(i, 0, size);
		gridlines.AddVertex(-i, 0, -size);   // lines parallel to Z-axis
		gridlines.AddVertex(-i, 0, size);
	}

	// x-axis
	//glColor3f(0.5f, 0, 0);
	//gridlines.AddVertex(-size, 0, 0);
	//gridlines.AddVertex(size, 0, 0);

	// z-axis
	//glColor3f(0, 0, 0.5f);
	//glVertex3f(0, 0, -size);
	//glVertex3f(0, 0, size);
}
