#pragma once

#include <QVariantMap>
#include <nlohmann/json.hpp>

class QVBoxLayout;
class QWidget;

// Reads the "properties" array from an effect's metadata JSON and creates
// one control row per property inside parent/layout.
// Each control emits a signal that EffectPanelWidget connects to its debounce timer.
// settings is pre-populated with defaults and updated live as controls change.
namespace EffectControlBuilder {

void build(QWidget* parent,
           QVBoxLayout* layout,
           const nlohmann::json& properties,
           QVariantMap& settings,
           std::function<void()> onChange);

} // namespace EffectControlBuilder
