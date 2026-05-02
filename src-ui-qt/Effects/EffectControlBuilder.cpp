#include "EffectControlBuilder.h"

#include "controls/CheckRow.h"
#include "controls/ComboRow.h"
#include "controls/FilePickerRow.h"
#include "controls/SliderSpinRow.h"
#include "controls/TextRow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <spdlog/spdlog.h>

namespace EffectControlBuilder {

static QString jstr(const nlohmann::json& j, const char* key, const char* fallback = "") {
    if (j.contains(key) && j[key].is_string())
        return QString::fromStdString(j[key].get<std::string>());
    return fallback;
}

void build(QWidget* parent,
           QVBoxLayout* layout,
           const nlohmann::json& properties,
           QVariantMap& settings,
           std::function<void()> onChange) {
    if (!properties.is_array()) return;

    for (const auto& prop : properties) {
        if (!prop.is_object()) continue;

        QString id          = jstr(prop, "id");
        QString label       = jstr(prop, "label", id.toStdString().c_str());
        QString controlType = jstr(prop, "controlType");

        if (id.isEmpty() || controlType.isEmpty()) continue;

        // Helper: return pre-populated value if present, else the JSON default.
        // Pre-population comes from parseRawSettings() when a block is selected.
        auto initStr = [&](const char* key, const char* fallback = "") -> QString {
            return settings.contains(id) ? settings[id].toString()
                                         : jstr(prop, key, fallback);
        };
        auto initDbl = [&](double defV) -> double {
            return settings.contains(id) ? settings[id].toDouble() : defV;
        };
        auto initBool = [&](bool defV) -> bool {
            if (settings.contains(id)) {
                const QString s = settings[id].toString();
                return s == "1" || s.toLower() == "true";
            }
            return defV;
        };

        if (controlType == "slider") {
            int    minV    = prop.value("min",     0);
            int    maxV    = prop.value("max",     100);
            double divisor = prop.value("divisor", 1.0);
            double jsonDef = prop.contains("default") && prop["default"].is_number()
                             ? prop["default"].get<double>() : double(minV);
            double initV   = initDbl(jsonDef);

            settings[id] = initV;
            auto* row = new SliderSpinRow(label, minV, maxV, divisor, initV, parent);
            layout->addWidget(row);
            QObject::connect(row, &SliderSpinRow::valueChanged, parent, [id, &settings, onChange](double v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "checkbox") {
            bool initV = initBool(prop.value("default", false));
            settings[id] = initV;
            auto* row = new CheckRow(label, initV, parent);
            layout->addWidget(row);
            QObject::connect(row, &CheckRow::valueChanged, parent, [id, &settings, onChange](bool v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "choice") {
            QStringList opts;
            if (prop.contains("options") && prop["options"].is_array())
                for (const auto& o : prop["options"])
                    if (o.is_string()) opts << QString::fromStdString(o.get<std::string>());

            if (opts.isEmpty()) continue;
            QString initV = initStr("default");
            // If the pre-populated value isn't in the option list, fall back to JSON default.
            if (!initV.isEmpty() && !opts.contains(initV))
                initV = jstr(prop, "default");
            settings[id] = initV;
            auto* row = new ComboRow(label, opts, initV, parent);
            layout->addWidget(row);
            QObject::connect(row, &ComboRow::valueChanged, parent, [id, &settings, onChange](const QString& v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "text") {
            QString initV = initStr("default");
            settings[id] = initV;
            auto* row = new TextRow(label, initV, parent);
            layout->addWidget(row);
            QObject::connect(row, &TextRow::valueChanged, parent, [id, &settings, onChange](const QString& v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "spin") {
            int    minV  = prop.value("min", 0);
            int    maxV  = prop.value("max", 100);
            int    initV = int(initDbl(prop.value("default", minV)));
            settings[id] = initV;
            auto* row = new SliderSpinRow(label, minV, maxV, 1.0, double(initV), parent);
            layout->addWidget(row);
            QObject::connect(row, &SliderSpinRow::valueChanged, parent, [id, &settings, onChange](double v) {
                settings[id] = int(v);
                onChange();
            });

        } else if (controlType == "filepicker") {
            QString filter = jstr(prop, "fileFilter", "All Files (*)");
            QString initV  = initStr("default");
            settings[id]   = initV;
            auto* row = new FilePickerRow(label, filter, parent);
            row->setValue(initV);
            layout->addWidget(row);
            QObject::connect(row, &FilePickerRow::valueChanged, parent, [id, &settings, onChange](const QString& v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "fontpicker") {
            QString initV = initStr("default");
            settings[id] = initV;
            auto* row = new TextRow(label + " (font)", initV, parent);
            layout->addWidget(row);
            QObject::connect(row, &TextRow::valueChanged, parent, [id, &settings, onChange](const QString& v) {
                settings[id] = v;
                onChange();
            });

        } else if (controlType == "custom") {
            // Effect-specific complex controls — placeholder label for now.
            auto* lbl = new QLabel("<i>" + label + " (custom control)</i>", parent);
            lbl->setStyleSheet("color: #888; padding: 4px 124px;");
            layout->addWidget(lbl);
            spdlog::debug("EffectControlBuilder: custom control '{}' skipped (not yet implemented)", id.toStdString());

        } else {
            spdlog::warn("EffectControlBuilder: unknown controlType '{}' for '{}'",
                         controlType.toStdString(), id.toStdString());
        }
    }

    layout->addStretch();
}

} // namespace EffectControlBuilder
