#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "aiBase.h"
#include "aiType.h"

#include <wx/bitmap.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <string>

class OpenAIImageGenerator : public aiBase::AIImageGenerator {
private:
    std::string base_url;
    std::string token;
    std::string image_model;

public:
    OpenAIImageGenerator(const std::string& base_url_, const std::string& token_, const std::string& modelName) :
        token(token_), image_model(modelName), base_url(base_url_) {
    }

    virtual ~OpenAIImageGenerator() {
    }

    void generateImage(const std::string& prompt,
                               const std::function<void(const wxBitmap&, const std::string&)>& cb) override;
};