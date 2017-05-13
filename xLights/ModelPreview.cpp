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

BEGIN_EVENT_TABLE(ModelPreview, xlGLCanvas)
EVT_MOTION(ModelPreview::mouseMoved)
EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
EVT_LEFT_UP(ModelPreview::mouseLeftUp)
EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
EVT_RIGHT_DOWN(ModelPreview::rightClick)
//EVT_KEY_DOWN(ModelPreview::keyPressed)
//EVT_KEY_UP(ModelPreview::keyReleased)
//EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
EVT_PAINT(ModelPreview::render)
END_EVENT_TABLE()

void ModelPreview::mouseMoved(wxMouseEvent& event) {
    if (_model != nullptr)
    {
        wxString tip =_model->GetNodeNear(this, event.GetPosition());
        SetToolTip(tip);
    }
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftDown(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::rightClick(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftUp(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftWindow(wxMouseEvent& event) {
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::render( wxPaintEvent& event )
{
    if(mIsDrawing) return;

    //if(!mIsInitialized) { InitializeGLCanvas(); }
    //SetCurrentGLContext();
    //wxPaintDC(this);

    if (_model != nullptr) {
        _model->DisplayEffectOnWindow(this, 2);
    } else {
        if(!StartDrawing(mPointSize)) return;
        Render();
        EndDrawing();
    }
}

void ModelPreview::Render()
{
    if (PreviewModels != nullptr)
    {
        bool isModelSelected = false;
        for (int i = 0; i < PreviewModels->size(); ++i) {
            if (((*PreviewModels)[i])->Selected || ((*PreviewModels)[i])->GroupSelected) {
                isModelSelected = true;
                break;
            }
        }

        for (int i=0; i<PreviewModels->size(); ++i) {
			const xlColor *color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
			if (((*PreviewModels)[i])->Selected) {
				color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
			} else if (((*PreviewModels)[i])->GroupSelected) {
				color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
			} else if (((*PreviewModels)[i])->Overlapping && isModelSelected) {
				color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_OVERLAP);
			}
            if (!allowSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
            }
            (*PreviewModels)[i]->DisplayModelOnWindow(this, accumulator, color, allowSelected);
        }
    }
}

void ModelPreview::Render(const unsigned char *data) {
    if (StartDrawing(mPointSize)) {
        if (PreviewModels != nullptr) {
            for (int m=0; m<PreviewModels->size(); m++) {
                int NodeCnt=(*PreviewModels)[m]->GetNodeCount();
                for(int n=0; n<NodeCnt; n++) {
                    int start = (*PreviewModels)[m]->NodeStartChannel(n);
                    (*PreviewModels)[m]->SetNodeChannelValues(n, &data[start]);
                }
                (*PreviewModels)[m]->DisplayModelOnWindow(this, accumulator);
            }
        }
        EndDrawing();
    }
}

void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {}
//void ModelPreview::rightClick(wxMouseEvent& event) {}
void ModelPreview::keyPressed(wxKeyEvent& event) {}
void ModelPreview::keyReleased(wxKeyEvent& event) {}

ModelPreview::ModelPreview(wxPanel* parent, std::vector<Model*> &models, bool a, int styles)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", true), PreviewModels(&models), allowSelected(a)
{
    maxVertexCount = 5000;
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    virtualWidth = 0;
    virtualHeight = 0;
    image = nullptr;
    sprite = nullptr;
    _model = nullptr;
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
void ModelPreview::InitializePreview(wxString img,int brightness)
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
        mBackgroundImageExists = wxFileExists(mBackgroundImage)&&wxIsReadable(mBackgroundImage)?true:false;
    }
    mBackgroundBrightness = brightness;
}

void ModelPreview::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    if (allowSelected) {
        LOG_GL_ERRORV(glClearColor(0.8f, 0.8f, 0.8f, 1.0f)); // Black Background
    } else {
        LOG_GL_ERRORV(glClearColor(0.0, 0.0, 0.0, 1.0f)); // Black Background
    }
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));

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

void ModelPreview::SetActive(bool show) {
    if( show ) {
        mPreviewPane->Show();
    } else {
        mPreviewPane->Hide();
    }
}

bool ModelPreview::StartDrawing(wxDouble pointSize)
{
    if(!IsShownOnScreen()) return false;
    if(!mIsInitialized) { InitializeGLCanvas(); }
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    SetCurrentGLContext();
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);

    LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
    DrawGLUtils::PushMatrix();
    // Rotate Axis and translate
    DrawGLUtils::Rotate(180,0,0,1);
    DrawGLUtils::Rotate(180,0,1,0);
    accumulator.PreAlloc(maxVertexCount);
    currentPixelScaleFactor = 1.0;
    if (!allowSelected && virtualWidth > 0 && virtualHeight > 0
        && (virtualWidth != mWindowWidth || virtualHeight != mWindowHeight)) {
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
    } else if (virtualWidth == 0 && virtualHeight == 0) {
        int i = (int)mWindowHeight;
        DrawGLUtils::Translate(0, -i, 0);
    } else {
        DrawGLUtils::Translate(0, -virtualHeight, 0);
        accumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
        accumulator.Finish(GL_TRIANGLES);
    }
    if(mBackgroundImageExists)
    {
        if (image == NULL)
        {
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
        accumulator.AddTextureVertex(0, 0, tx1, -0.5/(image->textureHeight));
        accumulator.AddTextureVertex(virtualWidth * scalew, 0, tx2, -0.5/(image->textureHeight));
        accumulator.AddTextureVertex(0, virtualHeight * scaleh, tx1, image->tex_coord_y);

        accumulator.AddTextureVertex(0, virtualHeight * scaleh, tx1, image->tex_coord_y);
        accumulator.AddTextureVertex(virtualWidth * scalew, 0, tx2, -0.5/(image->textureHeight));
        accumulator.AddTextureVertex(virtualWidth * scalew, virtualHeight *scaleh, tx2, image->tex_coord_y);

        int i = mBackgroundBrightness * 255 / 100;
        accumulator.FinishTextures(GL_TRIANGLES, image->getID(), i);
    }
    return true;
}

void ModelPreview::EndDrawing()
{
    if (accumulator.count > maxVertexCount) {
        maxVertexCount= accumulator.count;
    }
    DrawGLUtils::Draw(accumulator);
    DrawGLUtils::PopMatrix();
    LOG_GL_ERRORV(SwapBuffers());
    accumulator.Reset();
    mIsDrawing = false;
}

