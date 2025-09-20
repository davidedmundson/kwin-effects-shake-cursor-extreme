/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shakecursor.h"
#include "cursor.h"
#include "cursorsource.h"
#include "effect/effecthandler.h"
#include "effect/offscreenquickview.h"
#include "input_event.h"
#include "shakecursorconfig.h"
#include "pointer_input.h"
#include "scene/imageitem.h"
#include "scene/itemrenderer.h"
#include "scene/workspacescene.h"

namespace KWin
{

// Dave, does it make sense to do this? We're always updating the main buffer anyway
// so planes don't help
// it'd make it easier to split this to a new effect.

// then we can re-add the glow

ShakeCursorItem::ShakeCursorItem(const CursorTheme &theme, Item *parent)
    : Item(parent)
{
    m_source = std::make_unique<ShapeCursorSource>();
    m_source->setTheme(theme);
    m_source->setShape(Qt::ArrowCursor);

    refresh();
    connect(m_source.get(), &CursorSource::changed, this, &ShakeCursorItem::refresh);
    connect(m_source.get(), &CursorSource::changed, this, &ShakeCursorItem::refresh);
}

void ShakeCursorItem::refresh()
{
    if (!m_imageItem) {
        m_imageItem = scene()->renderer()->createImageItem(this);
    }
    m_imageItem->setImage(m_source->image());
    m_imageItem->setPosition(-m_source->hotspot());
    m_imageItem->setSize(m_source->image().deviceIndependentSize());
}

ShakeCursorEffect::ShakeCursorEffect()
    : m_cursor(Cursors::self()->mouse())
{
    input()->installInputEventSpy(this);

    m_deflateTimer.setSingleShot(true);
    connect(&m_deflateTimer, &QTimer::timeout, this, &ShakeCursorEffect::deflate);

    connect(&m_scaleAnimation, &QVariantAnimation::valueChanged, this, [this]() {
        magnify(m_scaleAnimation.currentValue().toReal());
    });

    ShakeCursorConfig::instance(effects->config());
    reconfigure(ReconfigureAll);

    m_cooldownTimer.setSingleShot(true);
    m_cooldownTimer.setInterval(5000);
    connect(&m_cooldownTimer, &QTimer::timeout, this, &ShakeCursorEffect::cleanupOffscreenViews);
}

ShakeCursorEffect::~ShakeCursorEffect()
{
    magnify(1.0);
}

bool ShakeCursorEffect::supported()
{
    return effects->isOpenGLCompositing();
}

qreal ShakeCursorEffect::targetMagnification() const
{
    return m_targetMagnification;
}

QPointF ShakeCursorEffect::cursorPos() const
{
    return m_cursor->pos();
}

QPointF ShakeCursorEffect::cursorHotSpot() const
{
    return m_cursor->hotspot();
}

bool ShakeCursorEffect::isActive() const
{
    return m_currentMagnification != 1.0 || m_cooldownTimer.isActive();
}

void ShakeCursorEffect::reconfigure(ReconfigureFlags flags)
{
    ShakeCursorConfig::self()->read();

    m_shakeDetector.setInterval(ShakeCursorConfig::timeInterval());
    m_shakeDetector.setSensitivity(ShakeCursorConfig::sensitivity());
}

void ShakeCursorEffect::inflate()
{
    qreal magnification;
    if (m_targetMagnification == 1.0) {
        magnification = ShakeCursorConfig::magnification();
    } else {
        magnification = m_targetMagnification + ShakeCursorConfig::overMagnification();
    }

    animateTo(magnification);
}

void ShakeCursorEffect::deflate()
{
    animateTo(1.0);
}

void ShakeCursorEffect::animateTo(qreal magnification)
{
    if (m_targetMagnification != magnification) {
        m_scaleAnimation.stop();

        m_scaleAnimation.setStartValue(m_currentMagnification);
        m_scaleAnimation.setEndValue(magnification);
        m_scaleAnimation.setDuration(200); // ignore animation speed, it's not an animation from user perspective
        m_scaleAnimation.setEasingCurve(QEasingCurve::InOutCubic);
        m_scaleAnimation.start();

        m_targetMagnification = magnification;
        qDebug() << m_targetMagnification;
        Q_EMIT targetMagnificationChanged();
    }
}

void ShakeCursorEffect::initOffscreenViews()
{
    // DAVE, I don't handle screen changes or size changes
    // but hopefully it's a short lived thing
    if (!m_scenesByScreens.empty()) {
        return;
    }
    const auto screens = effects->screens();
    for (const auto output : screens) {
        auto scene = new OffscreenQuickScene();

        scene->loadFromModule(QStringLiteral("org.kde.kwin.shakecursor"), QStringLiteral("Main"), {{QStringLiteral("effect"), QVariant::fromValue(this)}});
        scene->setGeometry(output->geometry());
        connect(scene, &OffscreenQuickView::repaintNeeded, this, [scene] {
            effects->addRepaint(scene->geometry());
        });
        m_scenesByScreens[output].reset(scene);
    }
}

void ShakeCursorEffect::cleanupOffscreenViews()
{
    m_scenesByScreens.clear();
}

void ShakeCursorEffect::pointerMotion(PointerMotionEvent *event)
{
    if (event->buttons != Qt::NoButton || event->warp) {
        m_shakeDetector.reset();
        return;
    }

    if (input()->pointer()->isConstrained()) {
        return;
    }

    if (m_shakeDetector.update(event)) {
        inflate();
        m_deflateTimer.start(2000);
    }
}

void ShakeCursorEffect::magnify(qreal magnification)
{
    if (magnification == 1.0) {
        m_currentMagnification = 1.0;
        if (m_cursorItem) {
            m_cursorItem.reset();
            effects->showCursor();
        }
        m_cooldownTimer.start();
    } else {
        initOffscreenViews();
        m_currentMagnification = magnification;
        m_cooldownTimer.stop();


        if (!m_cursorItem) {
            effects->hideCursor();

            const qreal maxScale = ShakeCursorConfig::magnification() + 8 * ShakeCursorConfig::overMagnification();
            const CursorTheme originalTheme = input()->pointer()->cursorTheme();
            if (m_cursorTheme.name() != originalTheme.name() || m_cursorTheme.size() != originalTheme.size() || m_cursorTheme.devicePixelRatio() != maxScale) {
                m_cursorTheme = CursorTheme(originalTheme.name(), originalTheme.size(), maxScale);
            }

            m_cursorItem = std::make_unique<ShakeCursorItem>(m_cursorTheme, effects->scene()->overlayItem());
            m_cursorItem->setPosition(m_cursor->pos());
            connect(m_cursor, &Cursor::posChanged, m_cursorItem.get(), [this]() {
                m_cursorItem->setPosition(m_cursor->pos());
                Q_EMIT cursorPosChanged();
            });
        }
        m_cursorItem->setTransform(QTransform::fromScale(magnification, magnification));
    }
}

void ShakeCursorEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const QRegion &region, KWin::Output *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, region, screen);

    if (auto it = m_scenesByScreens.find(screen); it != m_scenesByScreens.end()) {
        effects->renderOffscreenQuickView(renderTarget, viewport, it->second.get());
    }
}

} // namespace KWin

#include "moc_shakecursor.cpp"
