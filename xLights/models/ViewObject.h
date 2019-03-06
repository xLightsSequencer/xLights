#ifndef VIEWOBJECT_H
#define VIEWOBJECT_H

#include "BaseObject.h"
#include "ModelScreenLocation.h"
#include "ViewObjectManager.h"

class ModelPreview;

class ViewObject : public BaseObject
{
public:
    ViewObject(const ObjectManager &manager);
    virtual ~ViewObject();

    virtual void AddProperties(wxPropertyGridInterface *grid) override;
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) override {};
    void AddSizeLocationProperties(wxPropertyGridInterface *grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);
    void SetFromXml(wxXmlNode* ObjectNode, bool zeroBased=false) override;
    void UpdateXmlWithScale() override;
    virtual void InitModel() = 0;

    bool GetIs3dOnly() { return only_3d; }

    virtual const ModelScreenLocation &GetObjectScreenLocation() const = 0;
    virtual ModelScreenLocation &GetObjectScreenLocation() = 0;

    virtual void Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, DrawGLUtils::xl3Accumulator &tva3, bool allowSelected = false) {};

protected:
    bool active;

private:
    bool only_3d;
};

template <class ScreenLocation>
class ObjectWithScreenLocation : public ViewObject {
public:
    virtual const ModelScreenLocation &GetObjectScreenLocation() const {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetObjectScreenLocation() {
        return screenLocation;
    }
    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() {
        return screenLocation;
    }
protected:
    ObjectWithScreenLocation(const ViewObjectManager &manager) : ViewObject(manager) {}
    virtual ~ObjectWithScreenLocation() {}
    ScreenLocation screenLocation;
};

#endif // VIEWOBJECT_H
