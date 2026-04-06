/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelPropertyManager.h"
#include "ModelPropertyAdapter.h"
#include "adapters/MatrixPropertyAdapter.h"
#include "adapters/WreathPropertyAdapter.h"
#include "adapters/SingleLinePropertyAdapter.h"
#include "adapters/IciclesPropertyAdapter.h"
#include "adapters/WindowFramePropertyAdapter.h"
#include "adapters/CirclePropertyAdapter.h"
#include "adapters/ArchesPropertyAdapter.h"
#include "adapters/CandyCanePropertyAdapter.h"
#include "adapters/SpinnerPropertyAdapter.h"
#include "adapters/StarPropertyAdapter.h"
#include "adapters/ChannelBlockPropertyAdapter.h"
#include "adapters/TreePropertyAdapter.h"
#include "adapters/SpherePropertyAdapter.h"
#include "adapters/CubePropertyAdapter.h"
#include "adapters/ImagePropertyAdapter.h"
#include "adapters/CustomPropertyAdapter.h"
#include "adapters/PolyLinePropertyAdapter.h"
#include "adapters/MultiPointPropertyAdapter.h"
#include "adapters/DmxPropertyAdapter.h"
#include "adapters/DmxGeneralPropertyAdapter.h"
#include "adapters/DmxFloodlightPropertyAdapter.h"
#include "adapters/DmxMovingHeadPropertyAdapter.h"
#include "adapters/DmxMovingHeadAdvPropertyAdapter.h"
#include "adapters/DmxSkullPropertyAdapter.h"
#include "adapters/DmxServoPropertyAdapter.h"
#include "adapters/DmxServo3dPropertyAdapter.h"
#include "adapters/SubModelPropertyAdapter.h"
#include "models/Model.h"
#include "models/SubModel.h"
#include "models/DisplayAsType.h"

std::unique_ptr<ModelPropertyAdapter> ModelPropertyManager::CreateAdapter(Model& model) {
    switch (model.GetDisplayAs()) {
        case DisplayAsType::Matrix:
            return std::make_unique<MatrixPropertyAdapter>(model);
        case DisplayAsType::Wreath:
            return std::make_unique<WreathPropertyAdapter>(model);
        case DisplayAsType::SingleLine:
            return std::make_unique<SingleLinePropertyAdapter>(model);
        case DisplayAsType::Icicles:
            return std::make_unique<IciclesPropertyAdapter>(model);
        case DisplayAsType::WindowFrame:
            return std::make_unique<WindowFramePropertyAdapter>(model);
        case DisplayAsType::Circle:
            return std::make_unique<CirclePropertyAdapter>(model);
        case DisplayAsType::Arches:
            return std::make_unique<ArchesPropertyAdapter>(model);
        case DisplayAsType::CandyCanes:
            return std::make_unique<CandyCanePropertyAdapter>(model);
        case DisplayAsType::Spinner:
            return std::make_unique<SpinnerPropertyAdapter>(model);
        case DisplayAsType::Star:
            return std::make_unique<StarPropertyAdapter>(model);
        case DisplayAsType::ChannelBlock:
            return std::make_unique<ChannelBlockPropertyAdapter>(model);
        case DisplayAsType::Tree:
            return std::make_unique<TreePropertyAdapter>(model);
        case DisplayAsType::Sphere:
            return std::make_unique<SpherePropertyAdapter>(model);
        case DisplayAsType::Cube:
            return std::make_unique<CubePropertyAdapter>(model);
        case DisplayAsType::Image:
            return std::make_unique<ImagePropertyAdapter>(model);
        case DisplayAsType::Custom:
            return std::make_unique<CustomPropertyAdapter>(model);
        case DisplayAsType::PolyLine:
            return std::make_unique<PolyLinePropertyAdapter>(model);
        case DisplayAsType::MultiPoint:
            return std::make_unique<MultiPointPropertyAdapter>(model);
        case DisplayAsType::DmxFloodArea:
        case DisplayAsType::DmxFloodlight:
            return std::make_unique<DmxFloodlightPropertyAdapter>(model);
        case DisplayAsType::DmxGeneral:
            return std::make_unique<DmxGeneralPropertyAdapter>(model);
        case DisplayAsType::DmxMovingHead:
            return std::make_unique<DmxMovingHeadPropertyAdapter>(model);
        case DisplayAsType::DmxMovingHeadAdv:
            return std::make_unique<DmxMovingHeadAdvPropertyAdapter>(model);
        case DisplayAsType::DmxSkull:
            return std::make_unique<DmxSkullPropertyAdapter>(model);
        case DisplayAsType::DmxServo:
            return std::make_unique<DmxServoPropertyAdapter>(model);
        case DisplayAsType::DmxServo3d:
            return std::make_unique<DmxServo3dPropertyAdapter>(model);
        case DisplayAsType::SubModel:
            return std::make_unique<SubModelPropertyAdapter>(static_cast<SubModel&>(model));
        default:
            // Fallback: delegate to the model's own property grid methods
            return std::make_unique<ModelPropertyAdapter>(model);
    }
}
