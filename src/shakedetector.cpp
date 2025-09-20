/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shakedetector.h"
#include "input_event.h"

#include <cmath>

namespace KWin
{

ShakeDetector::ShakeDetector()
{
}

quint64 ShakeDetector::interval() const
{
    return m_interval.count();
}

void ShakeDetector::setInterval(quint64 interval)
{
    m_interval = std::chrono::milliseconds(interval);
}

qreal ShakeDetector::sensitivity() const
{
    return m_sensitivity;
}

void ShakeDetector::setSensitivity(qreal sensitivity)
{
    m_sensitivity = sensitivity;
}

static inline bool sameSign(qreal a, qreal b)
{
    constexpr qreal tolerance = 1;
    // movements less than tolerance count as movements in any direction
    return (a >= -tolerance && b >= -tolerance) || (a <= tolerance && b <= tolerance);
}

void ShakeDetector::reset()
{
    m_history.clear();
}

bool ShakeDetector::update(PointerMotionEvent *event)
{
    // Prune the old entries in the history.
    auto it = m_history.begin();
    for (; it != m_history.end(); ++it) {
        if (event->timestamp - it->timestamp < m_interval) {
            break;
        }
    }
    if (it != m_history.begin()) {
        m_history.erase(m_history.begin(), it);
    }

    if (m_history.size() >= 2) {
        HistoryItem &last = m_history[m_history.size() - 1];
        const HistoryItem &prev = m_history[m_history.size() - 2];
        if (sameSign(last.position.x() - prev.position.x(), event->position.x() - last.position.x()) && sameSign(last.position.y() - prev.position.y(), event->position.y() - last.position.y())) {
            last = HistoryItem{
                .position = event->position,
                .timestamp = event->timestamp,
            };
            return false;
        }
    }

    m_history.emplace_back(HistoryItem{
        .position = event->position,
        .timestamp = event->timestamp,
    });

    qreal left = m_history[0].position.x();
    qreal top = m_history[0].position.y();
    qreal right = m_history[0].position.x();
    qreal bottom = m_history[0].position.y();
    qreal distance = 0;

    for (size_t i = 1; i < m_history.size(); ++i) {
        // Compute the length of the mouse path.
        const qreal deltaX = m_history[i].position.x() - m_history[i - 1].position.x();
        const qreal deltaY = m_history[i].position.y() - m_history[i - 1].position.y();
        distance += std::sqrt(deltaX * deltaX + deltaY * deltaY);

        // Compute the bounds of the mouse path.
        left = std::min(left, m_history[i].position.x());
        top = std::min(top, m_history[i].position.y());
        right = std::max(right, m_history[i].position.x());
        bottom = std::max(bottom, m_history[i].position.y());
    }

    const qreal boundsWidth = right - left;
    const qreal boundsHeight = bottom - top;
    const qreal diagonal = std::sqrt(boundsWidth * boundsWidth + boundsHeight * boundsHeight);
    if (diagonal < 100) {
        return false;
    }

    const qreal shakeFactor = distance / diagonal;
    if (shakeFactor > m_sensitivity) {
        m_history.clear();
        return true;
    }

    return false;
}

} // namespace KWin
