import QtQuick
import QtQuick.Particles
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects

Rectangle
{
    width: 1000
    height: 800
    color: "navy"

    MouseArea {
        id: ma
        anchors.fill: parent
        cursorShape: Qt.BlankCursor
        hoverEnabled: true
        property bool lastDir: false
        property real energy: 1.0
        property real lastX
        onMouseXChanged: function() {
            var delta = mouseX - lastX;
            var dir = delta > 0;
            if (dir != lastDir) {
                energy += 0.5;
                resetTimer.restart();
            }
        }
    }

    Timer {
        id: resetTimer
        interval: 1000
        onTriggered: ma.energy = 1.0
    }

    Image {
        id: cursor
        source: "/opt/kde6/share/icons/breeze_cursors/cursors_scalable/default/default.svg"
        width: ma.energy < 40 ? 24 : + ma.energy
        height: ma.energy < 40 ? 24 : + ma.energy
        sourceSize: Qt.size(100,100)
        x: ma.mouseX
        y: ma.mouseY
        z: 1

        Behavior on width { SmoothedAnimation { velocity: 200 } }
        Behavior on height { SmoothedAnimation { velocity: 200 } }
/*
        SequentialAnimation on rotation {
            running: ma.energy > 150
            loops: Animation.Infinite
            NumberAnimation { from: -10; to: 10; duration: 400; easing.type: Easing.InOutQuad }
        }*/

    }

    Glow {
        enabled: ma.energy > 40
        source: cursor
        anchors.fill: cursor
        radius: 30
        spread: 0.4
        samples: 17
        color: "#9fff400f"
    }

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
        enabled: ma.energy > 60

        anchors.centerIn: cursor
        width: 24 + 60
        height: 24 + 60

        lifeSpan: 3000
        lifeSpanVariation: 1000
        size: 80
        endSize: 130
        sizeVariation: 20
        acceleration: PointDirection { y: -30; x: 10}
        velocity: AngleDirection { angle: 270; magnitude: 80; angleVariation: 22; magnitudeVariation: 15 }

        system: sys


        shape: MaskShape {
            source: "/opt/kde6/share/icons/breeze_cursors/cursors_scalable/default/default.svg"
        }

        group: "smoke"
    }

    ParticleSystem {
        id: sys
        anchors.fill: parent
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
        enabled: ma.energy > 100

        // whilst it seems sensible to follow the cursor size
        // updating the mask with a new size is really really expensive
        anchors.centerIn: cursor
        width: 24 + 80
        height: 24 + 80

        lifeSpan: 300
        size: 30
        endSize: 10
        sizeVariation: 20

        group: "fire"

        velocity: AngleDirection { angle: 0; magnitude: ma.energy; angleVariation: 360; magnitudeVariation: 15 }

        system: sys
        shape: MaskShape {
            source: "/opt/kde6/share/icons/breeze_cursors/cursors_scalable/default/default.svg"
        }

        ImageParticle {
            source: "qrc:///particleresources/glowdot.png"
            color: "#11ff400f"
            colorVariation: 0.1
            system: sys
            groups: ["fire"]
        }
    }
}
