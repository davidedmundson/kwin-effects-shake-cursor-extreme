/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "effect/effect.h"
#include "input_event_spy.h"
#include "shakedetector.h"
#include "scene/cursoritem.h"
#include "utils/cursortheme.h"

#include <QTimer>
#include <QVariantAnimation>

namespace KWin
{

class Cursor;
class CursorItem;
class ShapeCursorSource;
class OffscreenQuickScene;

class ShakeCursorItem : public Item
{
    Q_OBJECT

public:
    ShakeCursorItem(const CursorTheme &theme, Item *parent);

private:
    void refresh();

    std::unique_ptr<ImageItem> m_imageItem;
    std::unique_ptr<ShapeCursorSource> m_source;
};

class ShakeCursorEffect : public Effect, public InputEventSpy
{
    Q_OBJECT
    Q_PROPERTY(qreal targetMagnification READ targetMagnification NOTIFY targetMagnificationChanged FINAL)
    Q_PROPERTY(QPointF cursorPos READ cursorPos NOTIFY cursorPosChanged FINAL)
    Q_PROPERTY(QPointF cursorHotSpot READ cursorHotSpot NOTIFY cursorHotSpotChanged FINAL)

public:
    ShakeCursorEffect();
    ~ShakeCursorEffect() override;

    static bool supported();
    qreal targetMagnification() const;
    QPointF cursorPos() const;
    QPointF cursorHotSpot() const;

    bool isActive() const override;
    void reconfigure(ReconfigureFlags flags) override;
    void pointerMotion(PointerMotionEvent *event) override;
    void paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const QRegion &region, KWin::Output *screen) override;

Q_SIGNALS:
    void targetMagnificationChanged();
    void cursorPosChanged();
    void cursorHotSpotChanged();

private:
    void magnify(qreal magnification);

    void inflate();
    void deflate();
    void animateTo(qreal magnification);

    void initOffscreenViews();
    void cleanupOffscreenViews();

    QTimer m_deflateTimer;
    QVariantAnimation m_scaleAnimation;
    ShakeDetector m_shakeDetector;

    Cursor *m_cursor;
    std::unique_ptr<ShakeCursorItem> m_cursorItem;
    CursorTheme m_cursorTheme;
    qreal m_targetMagnification = 1.0;
    qreal m_currentMagnification = 1.0;
    QTimer m_cooldownTimer;
    std::unordered_map<Output *, std::unique_ptr<OffscreenQuickScene>> m_scenesByScreens;
};

} // namespace KWin
