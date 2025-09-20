/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shakecursor.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_ENABLED(ShakeCursorEffect,
                              "metadata.json",
                              return ShakeCursorEffect::supported();)

} // namespace KWin

#include "main.moc"
