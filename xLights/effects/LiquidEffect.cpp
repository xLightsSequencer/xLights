/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LiquidEffect.h"
#include "LiquidPanel.h"

#include <Box2D/Box2D.h>
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../UtilFunctions.h"
#include "../models/Model.h"
#include "../Parallel.h"

#include "../../include/liquid-16.xpm"
#include "../../include/liquid-24.xpm"
#include "../../include/liquid-32.xpm"
#include "../../include/liquid-48.xpm"
#include "../../include/liquid-64.xpm"

#include <log4cpp/Category.hh>

//#define LE_INTERPOLATE
#define MAX_PARTICLES 100000

LiquidEffect::LiquidEffect(int id) : RenderableEffect(id, "Liquid", liquid_16, liquid_24, liquid_32, liquid_48, liquid_64)
{
}

LiquidEffect::~LiquidEffect()
{
}

xlEffectPanel *LiquidEffect::CreatePanel(wxWindow *parent) {
    return new LiquidPanel(parent);
}

std::list<std::string> LiquidEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (media == nullptr && (settings.GetBool("E_CHECKBOX_FlowMusic1", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic2", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic3", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic4", false))) {
        res.push_back(wxString::Format("    WARN: Liquid effect cant change flow to music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    int frameInterval = 50;
    if (media != nullptr)
        frameInterval = media->GetFrameInterval();
    int lifetimeFrames = (1.1 * GetValueCurveIntMax("LifeTime", 1000, settings, LIQUID_LIFETIME_MIN, LIQUID_LIFETIME_MAX)) / 100 * frameInterval / 1000; // this is the lifetime in frames
    if (lifetimeFrames == 0) {
        lifetimeFrames = (eff->GetEndTimeMS() - eff->GetStartTimeMS()) / frameInterval;
    }
    lifetimeFrames = std::min(lifetimeFrames, (eff->GetEndTimeMS() - eff->GetStartTimeMS()) / frameInterval);
    int flow1 = GetValueCurveIntMax("Flow1", 100, settings, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX);
    int flow2 = settings.GetBool("E_CHECKBOX_Enabled2", false) ? GetValueCurveIntMax("Flow2", 100, settings, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX) : 0;
    int flow3 = settings.GetBool("E_CHECKBOX_Enabled3", false) ? GetValueCurveIntMax("Flow3", 100, settings, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX) : 0;
    int flow4 = settings.GetBool("E_CHECKBOX_Enabled4", false) ? GetValueCurveIntMax("Flow4", 100, settings, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX) : 0;
    int count = lifetimeFrames * (flow1 + flow2 + flow3 + flow4);

    if (count > MAX_PARTICLES) {
        res.push_back(wxString::Format("    WARN: Liquid effect lifetime * (flow 1 + flow 2 + flow 3 + flow 4) = %d exceeds %d. Particle count will be limited. Model '%s', Start %s", count, MAX_PARTICLES, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    if (settings.GetInt("E_TEXTCTRL_Size", 500) > 1000) {
        res.push_back(wxString::Format("    WARN: Liquid effect particle size > 1000 can slow render times significantly. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void LiquidEffect::SetDefaultParameters()
{
    LiquidPanel* tp = (LiquidPanel*)panel;
    if (tp == nullptr) {
        return;
    }

    SetCheckBoxValue(tp->CheckBox_TopBarrier, false);
    SetCheckBoxValue(tp->CheckBox_BottomBarrier, true);
    SetCheckBoxValue(tp->CheckBox_LeftBarrier, false);
    SetCheckBoxValue(tp->CheckBox_RightBarrier, false);

    SetCheckBoxValue(tp->CheckBox_HoldColor, true);
    SetCheckBoxValue(tp->CheckBox_MixColors, false);
    SetChoiceValue(tp->Choice_ParticleType, "Elastic");

    SetSliderValue(tp->Slider_LifeTime, 10000);
    SetSliderValue(tp->Slider_Despeckle, 0);
    SetSliderValue(tp->Slider_WarmUpFrames, 0);
    SetSliderValue(tp->Slider_Liquid_Gravity, 100);
    SetSliderValue(tp->Slider_Liquid_GravityAngle, 0);

    SetSliderValue(tp->Slider_X1, 50);
    SetSliderValue(tp->Slider_Y1, 100);
    SetSliderValue(tp->Slider_Direction1, 270);
    SetSliderValue(tp->Slider_Flow1, 100);
    SetSliderValue(tp->Slider_Size, 500);
    SetSliderValue(tp->Slider_Velocity1, 100);
    SetSliderValue(tp->Slider_Liquid_SourceSize1, 0);
    SetCheckBoxValue(tp->CheckBox_FlowMusic1, false);

    SetCheckBoxValue(tp->CheckBox_Enabled2, false);
    SetSliderValue(tp->Slider_X2, 0);
    SetSliderValue(tp->Slider_Y2, 50);
    SetSliderValue(tp->Slider_Direction2, 0);
    SetSliderValue(tp->Slider_Flow2, 100);
    SetSliderValue(tp->Slider_Velocity2, 100);
    SetSliderValue(tp->Slider_Liquid_SourceSize2, 0);
    SetCheckBoxValue(tp->CheckBox_FlowMusic2, false);

    SetCheckBoxValue(tp->CheckBox_Enabled3, false);
    SetSliderValue(tp->Slider_X3, 50);
    SetSliderValue(tp->Slider_Y3, 0);
    SetSliderValue(tp->Slider_Direction3, 90);
    SetSliderValue(tp->Slider_Flow3, 100);
    SetSliderValue(tp->Slider_Velocity3, 100);
    SetSliderValue(tp->Slider_Liquid_SourceSize3, 0);
    SetCheckBoxValue(tp->CheckBox_FlowMusic3, false);

    SetCheckBoxValue(tp->CheckBox_Enabled4, false);
    SetSliderValue(tp->Slider_X4, 100);
    SetSliderValue(tp->Slider_Y4, 50);
    SetSliderValue(tp->Slider_Direction4, 180);
    SetSliderValue(tp->Slider_Flow4, 100);
    SetSliderValue(tp->Slider_Velocity4, 100);
    SetSliderValue(tp->Slider_Liquid_SourceSize4, 0);
    SetCheckBoxValue(tp->CheckBox_FlowMusic4, false);

    tp->BitmapButton_LifeTime->SetActive(false);
    tp->BitmapButton_Liquid_Gravity->SetActive(false);
    tp->BitmapButton_Liquid_GravityAngle->SetActive(false);

    tp->BitmapButton_X1->SetActive(false);
    tp->BitmapButton_Y1->SetActive(false);
    tp->BitmapButton_Velocity1->SetActive(false);
    tp->BitmapButton_Direction1->SetActive(false);
    tp->BitmapButton_Flow1->SetActive(false);
    tp->BitmapButton_Liquid_SourceSize1->SetActive(false);

    tp->BitmapButton_X2->SetActive(false);
    tp->BitmapButton_Y2->SetActive(false);
    tp->BitmapButton_Velocity2->SetActive(false);
    tp->BitmapButton_Direction2->SetActive(false);
    tp->BitmapButton_Flow2->SetActive(false);
    tp->BitmapButton_Liquid_SourceSize2->SetActive(false);

    tp->BitmapButton_X3->SetActive(false);
    tp->BitmapButton_Y3->SetActive(false);
    tp->BitmapButton_Velocity3->SetActive(false);
    tp->BitmapButton_Direction3->SetActive(false);
    tp->BitmapButton_Flow3->SetActive(false);
    tp->BitmapButton_Liquid_SourceSize3->SetActive(false);

    tp->BitmapButton_X4->SetActive(false);
    tp->BitmapButton_Y4->SetActive(false);
    tp->BitmapButton_Velocity4->SetActive(false);
    tp->BitmapButton_Direction4->SetActive(false);
    tp->BitmapButton_Flow4->SetActive(false);
    tp->BitmapButton_Liquid_SourceSize4->SetActive(false);
}

void LiquidEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
           SettingsMap.GetBool("CHECKBOX_TopBarrier", false),
           SettingsMap.GetBool("CHECKBOX_BottomBarrier", false),
           SettingsMap.GetBool("CHECKBOX_LeftBarrier", false),
           SettingsMap.GetBool("CHECKBOX_RightBarrier", false),

           GetValueCurveInt("LifeTime", 1000, SettingsMap, oset, LIQUID_LIFETIME_MIN, LIQUID_LIFETIME_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_HoldColor", true),
           SettingsMap.GetBool("CHECKBOX_MixColors", false),
           SettingsMap.GetInt("TEXTCTRL_Size", 500),
           SettingsMap.GetInt("TEXTCTRL_WarmUpFrames", 0),

           GetValueCurveInt("Direction1", 270, SettingsMap, oset, LIQUID_DIRECTION_MIN, LIQUID_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X1", 50, SettingsMap, oset, LIQUID_X_MIN, LIQUID_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y1", 50, SettingsMap, oset, LIQUID_Y_MIN, LIQUID_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity1", 100, SettingsMap, oset, LIQUID_VELOCITY_MIN, LIQUID_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow1", 100, SettingsMap, oset, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize1", 0, SettingsMap, oset, LIQUID_SOURCESIZE_MIN, LIQUID_SOURCESIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic1", false),

           SettingsMap.GetBool("CHECKBOX_Enabled2", false),
           GetValueCurveInt("Direction2", 270, SettingsMap, oset, LIQUID_DIRECTION_MIN, LIQUID_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X2", 50, SettingsMap, oset, LIQUID_X_MIN, LIQUID_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y2", 50, SettingsMap, oset, LIQUID_Y_MIN, LIQUID_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity2", 100, SettingsMap, oset, LIQUID_VELOCITY_MIN, LIQUID_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow2", 100, SettingsMap, oset, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize2", 0, SettingsMap, oset, LIQUID_SOURCESIZE_MIN, LIQUID_SOURCESIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic2", false),

           SettingsMap.GetBool("CHECKBOX_Enabled3", false),
           GetValueCurveInt("Direction3", 270, SettingsMap, oset, LIQUID_DIRECTION_MIN, LIQUID_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X3", 50, SettingsMap, oset, LIQUID_X_MIN, LIQUID_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y3", 50, SettingsMap, oset, LIQUID_Y_MIN, LIQUID_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity3", 100, SettingsMap, oset, LIQUID_VELOCITY_MIN, LIQUID_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow3", 100, SettingsMap, oset, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize3", 0, SettingsMap, oset, LIQUID_SOURCESIZE_MIN, LIQUID_SOURCESIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic3", false),

           SettingsMap.GetBool("CHECKBOX_Enabled4", false),
           GetValueCurveInt("Direction4", 270, SettingsMap, oset, LIQUID_DIRECTION_MIN, LIQUID_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X4", 50, SettingsMap, oset, LIQUID_X_MIN, LIQUID_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y4", 50, SettingsMap, oset, LIQUID_Y_MIN, LIQUID_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity4", 100, SettingsMap, oset, LIQUID_VELOCITY_MIN, LIQUID_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow4", 100, SettingsMap, oset, LIQUID_FLOW_MIN, LIQUID_FLOW_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize4", 0, SettingsMap, oset, LIQUID_SOURCESIZE_MIN, LIQUID_SOURCESIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic4", false),
           SettingsMap.Get("CHOICE_ParticleType", "Elastic"),
           SettingsMap.GetInt("TEXTCTRL_Despeckle", 0),
           GetValueCurveDouble("Liquid_Gravity", 10.0, SettingsMap, oset, LIQUID_GRAVITY_MIN, LIQUID_GRAVITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), LIQUID_GRAVITY_DIVISOR),
           GetValueCurveInt("Liquid_GravityAngle", 0, SettingsMap, oset, LIQUID_GRAVITYANGLE_MIN, LIQUID_GRAVITYANGLE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS())
        );
}

class LiquidRenderCache : public EffectRenderCache {
public:
    LiquidRenderCache() { _world = nullptr; };
    virtual ~LiquidRenderCache() {
        if (_world != nullptr) delete _world;
	};
    b2World* _world;
};

void LiquidEffect::CreateBarrier(b2World* world, float x, float y, float width, float height)
{
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(x, y);
    b2Body* groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(width / 2.0f, height / 2.0f);
    groundBody->CreateFixture((b2Shape*)&groundBox, 0.0f);
}

bool LiquidEffect::LostForever(int x, int y, int w, int h, float gravityX, float gravityY)
{
    // essentially vertical ... at this point it wont come back
    if (gravityX < 0.0001f && gravityX > -0.0001f) {
        if (x < -1 || x > w + 1)
            return true;
    }

    // gravity left and off the screen so it wont come back
    if (gravityX < 0.0001f)
    {
        if (x < -1)
            return true;
    }

    // gravity right and off the screen so it wont come back
    if (gravityX > -0.0001f) {
        if (x > w + 1)
            return true;
    }

    // essentially horizontal
    if (gravityY < 0.0001f && gravityY > -0.0001f) {
        if (y < -1 || y > h + 1)
            return true;
    }

    // gravity down and off the bottom of the screen
    if (gravityY > -0.0001f) {
        if (y < -1)
            return true;
    }

    // gravity up and off the top of the screen
    if (gravityY < 0.0001f) {
        if (y > h + 1)
            return true;
    }

    return false;
}

void LiquidEffect::Draw(RenderBuffer& buffer, b2ParticleSystem* ps, const xlColor& color, bool mixColors, int despeckle, float gravityX, float gravityY)
{
#ifdef LE_INTERPOLATE
    size_t bufsiz = sizeof(size_t) * buffer.BufferWi * buffer.BufferHt;
    size_t* red = (size_t*)malloc(bufsiz);
    size_t* green = (size_t*)malloc(bufsiz);
    size_t* blue = (size_t*)malloc(bufsiz);
    size_t* count = (size_t*)malloc(bufsiz);
    memset(red, 0x00, bufsiz);
    memset(green, 0x00, bufsiz);
    memset(blue, 0x00, bufsiz);
    memset(count, 0x00, bufsiz);

    int32 particleCount = ps->GetParticleCount();
    if (particleCount)
    {
        const b2Vec2* positionBuffer = ps->GetPositionBuffer();
        const b2ParticleColor* colorBuffer = ps->GetColorBuffer();

        for (int i = 0; i < ps->GetParticleCount(); ++i)
        {
            int x = positionBuffer[i].x;
            int y = positionBuffer[i].y;
            if (y < -1 || x < -1 || x > buffer.BufferWi + 1)
            {
                ps->DestroyParticle(i);
            }
            else
            {
                if (x >= 0 && x < buffer.BufferWi && y >= 0 && y < buffer.BufferHt)
                {
                    size_t offset = buffer.BufferWi * y + x;
                    if (mixColors && ps->GetColorBuffer())
                    {
                        auto c = colorBuffer[i].GetColor();
                        *(red + offset) += c.r * 255;
                        *(green + offset) += c.g * 255;
                        *(blue + offset) += c.b * 255;
                        *(count + offset) = *(count + offset) + 1;
                    }
                    else
                    {
                        *(red + offset) += color.red;
                        *(green + offset) += color.green;
                        *(blue + offset) += color.blue;
                        *(count + offset) = *(count + offset) + 1;
                    }
                }
            }
        }
    }

    for (size_t y = 0; y < buffer.BufferHt; ++y)
    {
        for (size_t x = 0; x < buffer.BufferWi; ++x)
        {
            size_t offset = (buffer.BufferWi * y + x);
            size_t ct = *(count + offset);
            if (ct > 0)
            {
                buffer.SetPixel(x, y, xlColor(
                    *(red + offset) / ct,
                    *(green + offset) / ct,
                    *(blue + offset) / ct
                ));
            }
        }
    }

    free(red);
    free(green);
    free(blue);
    free(count);
#else
    int32 particleCount = ps->GetParticleCount();
    if (particleCount > 0)
    {
        const b2Vec2* positionBuffer = ps->GetPositionBuffer();
        const b2ParticleColor* colorBuffer = ps->GetColorBuffer();

        for (int i = 0; i < particleCount; ++i)
        {
            int x = positionBuffer[i].x;
            int y = positionBuffer[i].y;

            if (LostForever(x, y, buffer.BufferWi, buffer.BufferHt, gravityX, gravityY))
            {
                ps->DestroyParticle(i);
            }
            else
            {
                if (mixColors && ps->GetColorBuffer())
                {
                    auto c = colorBuffer[i].GetColor();
                    buffer.SetPixel(positionBuffer[i].x, positionBuffer[i].y, xlColor(c.r * 255, c.g * 255, c.b * 255));
                }
                else
                {
                    buffer.SetPixel(positionBuffer[i].x, positionBuffer[i].y, color);
                }
            }
        }
    }
#endif

    if (despeckle > 0)
    {
        for (size_t y = 0; y < buffer.BufferHt; ++y) {
            for (size_t x = 0; x < buffer.BufferWi; ++x) {
                if (buffer.GetPixel(x, y) == xlBLACK) {
                    buffer.SetPixel(x, y, GetDespeckleColor(buffer, x, y, despeckle));
                }
            }
        }
    }
}

xlColor LiquidEffect::GetDespeckleColor(RenderBuffer& buffer, size_t x, size_t y, int despeckle) const
{
    int red = 0;
    int green = 0;
    int blue = 0;
    int count = 0;

    int startx = x - 1;
    if (startx < 0) startx = 0;

    int starty = y - 1;
    if (starty < 0) starty = 0;

    int endx = x + 1;
    if (endx >= buffer.BufferWi) endx = buffer.BufferWi - 1;

    int endy = y + 1;
    if (endy >= buffer.BufferHt) endy = buffer.BufferHt - 1;

    int blacks = 0;

    for (int yy = starty; yy <= endy; ++yy)
    {
        for (int xx = startx; xx <= endx; ++xx)
        {
            if (yy != y || xx != x) // dont evaluate the pixel itself
            {
                xlColor c = buffer.GetPixel(xx, yy);

                // if any surrounding pixel is also black then we return black ... we only despeckly totally surrounded pixels
                if (c == xlBLACK)
                {
                    ++blacks;
                    if (blacks >= despeckle) return xlBLACK;
                }

                red += c.red;
                green += c.green;
                blue += c.blue;
                ++count;
            }
        }
    }

    if (count == 0) {
        return xlBLACK;
    }
    return xlColor(red / count, green / count, blue / count);
}

void LiquidEffect::CreateParticles(b2ParticleSystem* ps, int x, int y, int direction, int velocity, int flow, bool flowMusic, int lifetime, int width, int height, const xlColor& c, const std::string& particleType, bool mixcolors, float audioLevel, int sourceSize)
{
    static const float pi2 = 6.283185307f;
    float posx = (float)x * (float)width / 100.0;
    float posy = (float)y * (float)height / 100.0;

    float velx = (float)velocity * 10.0 * RenderBuffer::cos(pi2 * (float)direction / 360.0);
    float vely = (float)velocity * 10.0 * RenderBuffer::sin(pi2 * (float)direction / 360.0);

    float velVariation = rand01() * 0.1;
    velVariation -= velVariation / 2.0;

    velx -= velx * velVariation;
    vely -= vely * velVariation;

    // if lifetime is 1000 ... then we live for 10 seconds
    float lt = lifetime / 100.0;

    int count = flow;
    if (flowMusic)
    {
        count *= audioLevel;
    }

    // if we are going to exceed the maximum particles in 2 steps then we need to start flagging the older particles for deletion
    // DestroyOldestParticle does not delete them immediately
    if (ps->GetParticleCount() > MAX_PARTICLES - (2 * count)) {
        for (int i = 0; i < ps->GetParticleCount() - (MAX_PARTICLES - 2 * count); ++i) {
            ps->DestroyOldestParticle(i, true);
        }
    }

    for (int i = 0; i < count && ps->GetParticleCount() < MAX_PARTICLES; ++i)
    {
        b2ParticleDef pd;
        if (particleType == "Elastic")
        {
            pd.flags = b2_elasticParticle;
        }
        else if (particleType == "Powder")
        {
            pd.flags = b2_powderParticle;
        }
        else if (particleType == "Tensile")
        {
            pd.flags = b2_tensileParticle;
        }
        else if (particleType == "Spring")
        {
            pd.flags = b2_springParticle;
        }
        else if (particleType == "Viscous")
        {
            pd.flags = b2_viscousParticle;
        }
        else if (particleType == "Static Pressure")
        {
            pd.flags = b2_staticPressureParticle;
        }
        else if (particleType == "Water")
        {
            pd.flags = b2_waterParticle;
        }
        else if (particleType == "Reactive")
        {
            pd.flags = b2_reactiveParticle;
        }
        else if (particleType == "Repulsive")
        {
            pd.flags = b2_repulsiveParticle;
        }

        if (mixcolors)
        {
            pd.flags |= b2_colorMixingParticle;
        }

        pd.color.Set(c.Red(), c.Green(), c.Blue(), 255);

        if (sourceSize == 0)
        {
            // Randomly pick a position within the emitter's radius.
            const float32 angle = rand01() * 2.0f * b2_pi;

            // Distance from the center of the circle.
            const float32 distance = rand01();
            b2Vec2 positionOnUnitCircle(RenderBuffer::sin(angle), RenderBuffer::cos(angle));

            // Initial position.
            pd.position.Set(
                posx + positionOnUnitCircle.x * distance * 0.5,
                posy + positionOnUnitCircle.y * distance * 0.5);
        }
        else
        {
            // Distance from the center of the circle.
            const float32 distance = rand01() * ((float)sourceSize - (float)sourceSize / 2.0);

            float offx = distance * RenderBuffer::cos(pi2 * ((float)direction + 90.0) / 360.0);
            float offy = distance * RenderBuffer::sin(pi2 * ((float)direction + 90.0) / 360.0);

            // Initial position.
            pd.position.Set(posx + (offx * (float)width / 200.0), posy + (offy * (float)height / 200.0));
        }

        // Send it flying
        pd.velocity.x = velx;
        pd.velocity.y = vely;

        // give it a lifetime
        if (lifetime > 0)
        {
            float randomlt = lt + (lt * 0.2 * rand01()) - (lt *.01);
            pd.lifetime = randomlt;
        }
        ps->CreateParticle(pd);
    }
}

void LiquidEffect::CreateParticleSystem(b2World* world, int lifetime, int size)
{
    b2ParticleSystemDef particleSystemDef;
    auto particleSystem = world->CreateParticleSystem(&particleSystemDef);
    particleSystem->SetRadius((float)size / 1000.0f);
    particleSystem->SetMaxParticleCount(MAX_PARTICLES);
    if (lifetime > 0)
    {
        particleSystem->SetDestructionByAge(true);
    }
}

void LiquidEffect::Step(b2World* world, RenderBuffer &buffer, bool enabled[], int lifetime, const std::string& particleType, bool mixcolors,
    int x1, int y1, int direction1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
    int x2, int y2, int direction2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
    int x3, int y3, int direction3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
    int x4, int y4, int direction4, int velocity4, int flow4, int sourceSize4, bool flowMusic4, float time
)
{
    // move all existing items
    // If frame time is 50ms then time advances by 0.05s
    float32 timeStep = (float)buffer.frameTimeInMs / 1000.0;
    int32 velocityIterations = 6;
    int32 positionIterations = 2;
    int32 particleIterations = 3;
    world->Step(timeStep, velocityIterations, positionIterations, particleIterations);

    // create new particles
    b2ParticleSystem* ps = world->GetParticleSystemList();
    if (ps != nullptr)
    {
        float audioLevel = 0.0001f;
        if (buffer.GetMedia() != nullptr)
        {
            std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr)
            {
                audioLevel = *pf->cbegin();
            }
        }

        int j = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (enabled[i])
            {
                xlColor color;
                buffer.palette.GetColor(j % buffer.GetColorCount(), color, time);

                switch (i)
                {
                case 0:
                    CreateParticles(ps, x1, y1, direction1, velocity1, flow1, flowMusic1, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize1);
                    break;
                case 1:
                    CreateParticles(ps, x2, y2, direction2, velocity2, flow2, flowMusic2, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize2);
                    break;
                case 2:
                    CreateParticles(ps, x3, y3, direction3, velocity3, flow3, flowMusic3, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize3);
                    break;
                case 3:
                    CreateParticles(ps, x4, y4, direction4, velocity4, flow4, flowMusic4, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize4);
                    break;
                }
                ++j;
            }
        }
    }
}

void LiquidEffect::Render(RenderBuffer &buffer,
    bool top, bool bottom, bool left, bool right,
    int lifetime, bool holdcolor, bool mixcolors, int size, int warmUpFrames,
    int direction1, int x1, int y1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
    bool enabled2, int direction2, int x2, int y2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
    bool enabled3, int direction3, int x3, int y3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
    bool enabled4, int direction4, int x4, int y4, int velocity4, int flow4, int sourceSize4, bool flowMusic4,
    const std::string& particleType, int despeckle, float gravity, int gravityAngle)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool enabled[4];
    enabled[0] = true;
    enabled[1] = enabled2;
    enabled[2] = enabled3;
    enabled[3] = enabled4;

    LiquidRenderCache *cache = (LiquidRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new LiquidRenderCache();
        buffer.infoCache[id] = cache;
    }
    b2World*& _world = cache->_world;

    float gravityX = gravity * std::cos(toRadians(360 - (gravityAngle + 90)));
    float gravityY = gravity * std::sin(toRadians(360 - (gravityAngle + 90)));

    b2Vec2 grav(gravityX, gravityY);

    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        if (_world != nullptr)
        {
            delete _world;
            _world = nullptr;
        }

        _world = new b2World(grav);
        if (bottom)
        {
            CreateBarrier(_world, (float)buffer.BufferWi / 2.0, -1.0f, (float)buffer.BufferWi, 0.001f);
        }
        if (top)
        {
            CreateBarrier(_world, (float)buffer.BufferWi / 2.0, buffer.BufferHt + 1.0f, (float)buffer.BufferWi, 0.001f);
        }
        if (left)
        {
            CreateBarrier(_world, -1.0f, (float)buffer.BufferHt / 2.0f, 0.001f, (float)buffer.BufferHt);
        }
        if (right)
        {
            CreateBarrier(_world, (float)buffer.BufferWi + 1.0f, (float)buffer.BufferHt / 2.0f, 0.001f, (float)buffer.BufferHt);
        }

        CreateParticleSystem(_world, lifetime, size);

        for (int i = 0; i < warmUpFrames; ++i)
        {
            Step(_world, buffer, enabled, lifetime, particleType, mixcolors,
                x1, y1, direction1, velocity1, flow1, sourceSize1, flowMusic1,
                x2, y2, direction2, velocity2, flow2, sourceSize2, flowMusic2,
                x3, y3, direction3, velocity3, flow3, sourceSize3, flowMusic3,
                x4, y4, direction4, velocity4, flow4, sourceSize4, flowMusic4, 0.0
            );
        }
    }

    // exit if no world
    if (_world == nullptr) return;

    _world->SetGravity(grav);

    // allow up to 1 times physical memory
    if (IsExcessiveMemoryUsage(1.0))
    {
        logger_base.error("LiquidEffect Render abandoned due to insufficient memory. This is not good. Rendering will be slow.");
        logger_base.error("To reduce memory turn off render caching and/or change liquid effect settings.");

        // delete our world to get all our memory back
        delete _world;
        _world = nullptr;

        wxASSERT(false);
        return;
    }

    Step(_world, buffer, enabled, lifetime, particleType, mixcolors,
        x1, y1, direction1, velocity1, flow1, sourceSize1, flowMusic1,
        x2, y2, direction2, velocity2, flow2, sourceSize2, flowMusic2,
        x3, y3, direction3, velocity3, flow3, sourceSize3, flowMusic3,
        x4, y4, direction4, velocity4, flow4, sourceSize4, flowMusic4, buffer.GetEffectTimeIntervalPosition()
    );

    // create new particles
    b2ParticleSystem* ps = _world->GetParticleSystemList();
    if (ps != nullptr)
    {
         xlColor color;
        buffer.palette.GetColor(0, color);
        Draw(buffer, ps, color, holdcolor || mixcolors, despeckle, gravityX, gravityY);
    }

    // because of memory usage delete our world when rendered the last frame
    if (buffer.curPeriod == buffer.curEffEndPer)
    {
        delete _world;
        _world = nullptr;
    }
}
