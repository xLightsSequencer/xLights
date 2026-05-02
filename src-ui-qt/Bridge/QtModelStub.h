#pragma once
#include "../../src-core/models/SingleLineModel.h"
#include "../../src-core/models/ModelManager.h"
#include <string>

// Minimal model for constructing a RenderBuffer without a full show setup.
// RenderBuffer calls GetFullName() and GetDisplayAs() during construction;
// everything else is inherited from SingleLineModel with default/zero values.
class QtModelStub : public SingleLineModel {
public:
    QtModelStub(const ModelManager& mm, const std::string& name, int w, int h)
        : SingleLineModel(mm), _name(name)
    {
        // Override buffer dimensions so RenderBuffer gets the right size.
        BufferWi = w;
        BufferHt = h;
    }

    std::string GetFullName() const override { return _name; }

private:
    std::string _name;
};
