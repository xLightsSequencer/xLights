#pragma once
#include <QString>
#include <QStringList>

// Extracts B_CUSTOM_SubBuffer percentages from a raw "key=val,..." xsq string.
// Format: "left%xbottom%xright%xtop%x0x0" using 'x' as delimiter.
// Values are floating-point; negative values and values > 100 occur in the wild.
// Defaults to (0, 0, 100, 100) = full buffer when the key is absent or empty.
inline void parseSubBuffer(const QString& rawSettings,
                            double& left, double& bottom, double& right, double& top) {
    left = 0; bottom = 0; right = 100; top = 100;
    for (const QString& part : rawSettings.split(',', Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq <= 0) continue;
        if (part.left(eq).trimmed() != "B_CUSTOM_SubBuffer") continue;
        QString sb = part.mid(eq + 1).trimmed();
        sb.replace("&comma;", ",");
        if (sb.isEmpty()) return;
        sb.replace("Max", "___");
        const QStringList ps = sb.split('x');
        auto toDbl = [&](int idx, double def) -> double {
            if (idx >= ps.size()) return def;
            bool ok; double v = ps[idx].toDouble(&ok); return ok ? v : def;
        };
        left   = toDbl(0, 0.0);
        bottom = toDbl(1, 0.0);
        right  = toDbl(2, 100.0);
        top    = toDbl(3, 100.0);
        return;
    }
}
