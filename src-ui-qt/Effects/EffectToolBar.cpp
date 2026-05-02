#include "EffectToolBar.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QToolButton>
#include <QWidgetAction>

// ── Icon generation ───────────────────────────────────────────────────────────

QIcon EffectToolBar::makeIcon(const QString& name) {
    constexpr int SZ = 32;
    QPixmap pm(SZ, SZ);
    uint    h  = qHash(name);
    QColor  bg = QColor::fromHsv(int((h * 137u) % 360), 150, 190);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    // Effect-specific pattern, fallback to solid bg
    if (name == "Rainbow" || name == "Colorloop") {
        for (int i = 0; i < SZ; ++i) {
            p.setBrush(QColor::fromHsvF(float(i) / SZ, 1.f, 1.f));
            p.drawRect(i, 0, 1, SZ);
        }
    } else if (name == "Bars") {
        const int bars = 5;
        for (int i = 0; i < bars; ++i) {
            int x = i * SZ / bars;
            p.setBrush(QColor::fromHsv(int((h * 137u + i * 72u) % 360), 200, 220));
            p.drawRect(x, 0, SZ / bars, SZ);
        }
    } else if (name == "Fire") {
        for (int i = 0; i < SZ; ++i) {
            float t = float(i) / SZ;
            p.setBrush(QColor::fromHsvF(t * 0.1f, 1.f, 1.f));
            p.drawRect(i, 0, 1, SZ);
        }
    } else if (name == "Twinkle" || name == "Sparkle") {
        p.fillRect(QRect(0, 0, SZ, SZ), QColor(5, 5, 20));
        p.setPen(Qt::white);
        for (int i = 0; i < 8; ++i) {
            int x = int((h * uint(i + 1) * 6364136u) >> 16) % SZ;
            int y = int((h * uint(i + 1) * 1013904u) >> 16) % SZ;
            p.drawPoint(x, y);
        }
        p.setPen(Qt::NoPen);
    } else if (name == "Chase" || name == "Meteor") {
        p.fillRect(QRect(0, 0, SZ, SZ), QColor(10, 10, 10));
        for (int i = 0; i < SZ; ++i) {
            float t = float(i) / SZ;
            float v = t < 0.2f ? 1.f : qMax(0.f, 1.f - (t - 0.2f) / 0.8f);
            p.setBrush(QColor::fromHsvF(float(h % 360) / 360.f, 0.9f, v));
            p.drawRect(i, 0, 1, SZ);
        }
    } else {
        // Solid bg with a simple highlight stripe
        p.fillRect(QRect(0, 0, SZ, SZ), bg);
        p.setBrush(bg.lighter(150));
        p.drawRect(0, 0, SZ, SZ / 5);
    }

    // Abbreviated label in white — helps when many icons look similar
    p.setPen(QColor(255, 255, 255, 210));
    QFont f;
    f.setBold(true);
    f.setPixelSize(10);
    p.setFont(f);
    QString abbr = name.left(3).toUpper();
    p.drawText(QRect(0, 0, SZ, SZ), Qt::AlignCenter, abbr);

    return QIcon(pm);
}

// ── Construction ──────────────────────────────────────────────────────────────

EffectToolBar::EffectToolBar(QWidget* parent) : QToolBar("Effects", parent) {
    setMovable(false);
    setFloatable(false);
}

void EffectToolBar::populate(const QStringList& names) {
    clear();

    auto* container = new QWidget;
    auto* hbox      = new QHBoxLayout(container);
    hbox->setContentsMargins(2, 1, 2, 1);
    hbox->setSpacing(2);

    for (const QString& name : names) {
        auto* btn = new QToolButton;
        btn->setIcon(makeIcon(name));
        btn->setText(name);
        btn->setToolTip(name);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIconSize({32, 32});
        btn->setFixedSize(62, 54);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        connect(btn, &QToolButton::clicked, this, [this, name] {
            emit effectSelected(name);
        });
        hbox->addWidget(btn);
    }
    hbox->addStretch(1);

    auto* scroll = new QScrollArea;
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setFixedHeight(62);

    auto* wa = new QWidgetAction(this);
    wa->setDefaultWidget(scroll);
    addAction(wa);
}
