import QtQuick
import QtQuick.Particles
import Qt5Compat.GraphicalEffects

Item
{
    id: root
    required property var effect

    // The big question is whether we should just draw the icon too
    Item {
        id: cursor
        x: root.effect.cursorPos.x - root.effect.cursorHotSpot.x
        y: root.effect.cursorPos.y - root.effect.cursorHotSpot.y
        width: 24 * root.effect.targetMagnification
        height: 24 * root.effect.targetMagnification
        property url source: "file://opt/kde6/share/icons/breeze_cursors/cursors_scalable/default/default.svg"
        visible: false
    }

    // // this would look better with a smarter shader that's a bit fuzzy
    // Glow {
    //     source: cursor
    //     anchors.fill: cursor
    //     radius: 3 * root.effect.targetMagnification
    //     spread: 0.4
    //     samples: 17
    //     color: "#9fff400f"
    // }

    ImageParticle {
        id: img
        source: "qrc:///particleresources/glowdot.png"
        color: "#11111111"
        system: sys
        groups: ["smoke"]
    }

    Emitter {
        id: smoke
        emitRate: 400
        enabled: root.effect.targetMagnification > 4
        anchors.fill: cursor

        lifeSpan: 3000
        lifeSpanVariation: 1000
        size: 80
        endSize: 130
        sizeVariation: 20
        acceleration: PointDirection { y: -30; x: 10}
        velocity: AngleDirection { angle: 270; magnitude: 80; angleVariation: 22; magnitudeVariation: 15 }

        system: sys


        shape: MaskShape {
            id: maskShape
            source: cursor.source
        }

        group: "smoke"
    }

    ParticleSystem {
        id: sys
        anchors.fill: parent
        enabled: root.effect.targetMagnification > 0 // we should probably just clean up the view when 0
    }

    // Dave, does this do anything?
    Turbulence {
        anchors.fill: parent
        strength: 20
        groups: ["smoke"]
    }

    Emitter {
        id: fire
        emitRate: 600
        enabled: root.effect.targetMagnification > 7
        // updating the mask with a new size is really really expensive
        // we can do it whilst targetMagnification is an int, but if someone changes it and things go slow, check here first
        anchors.fill: cursor

        lifeSpan: 300
        size: 30
        endSize: 10
        sizeVariation: 20

        group: "fire"

        velocity: AngleDirection { angle: 0; magnitude: root.effect.targetMagnification; angleVariation: 360; magnitudeVariation: 15 }

        system: sys
        shape: maskShape

        ImageParticle {
            source: "qrc:///particleresources/glowdot.png"
            color: "#11ff400f"
            colorVariation: 0.1
            system: sys
            groups: ["fire"]
        }
    }
}
