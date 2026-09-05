import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: rootWindow
    width: 1024
    height: 600
    visible: true
    title: qsTr("Automotive Digital Instrument Cluster")
    color: "#06090e"

    // Master Cockpit Chassis Frame
    Rectangle {
        id: cockpitFrame
        anchors.fill: parent
        anchors.margins: 16
        color: "#0a0f18"
        border.color: "#182a40"
        border.width: 2
        radius: 8

        // Top Header Banner
        Rectangle {
            id: topBanner
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 46
            color: "#0e1624"
            border.color: "#182a40"
            border.width: 1
            radius: 8

            Row {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: "◖ SYSTEM READY ◗"
                    font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#00e676"
                }

                Text {
                    text: "AUTOMOTIVE DIGITAL INSTRUMENT CLUSTER"
                    font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                    font.pixelSize: 15
                    font.bold: true
                    color: "#e2e8f0"
                }

                Text {
                    text: "SOME/IP ONLINE"
                    font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#00e5ff"
                }
            }
        }

        // =====================================================================
        // MAIN COCKPIT BODY (THREE COLUMNS: LEFT, CENTER, RIGHT)
        // =====================================================================
        Item {
            anchors.top: topBanner.bottom
            anchors.bottom: warningAnnunciator.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16

            // -----------------------------------------------------------------
            // LEFT PANEL: TRANSMISSION & ENGINE VITALS
            // -----------------------------------------------------------------
            Rectangle {
                id: leftPanel
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 220
                color: "#0c131f"
                border.color: "#1a2c42"
                border.width: 1
                radius: 6

                Column {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 16

                    // Gear Display
                    Item {
                        width: parent.width
                        height: 70

                        Text {
                            text: "GEAR"
                            font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#64748b"
                        }

                        Rectangle {
                            anchors.right: parent.right
                            width: 60
                            height: 60
                            radius: 4
                            color: "#162235"
                            border.color: "#00e5ff"
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: cluster ? cluster.gear : "P"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 36
                                font.bold: true
                                color: "#00e5ff"
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: "#1a2c42" }

                    // Fuel Level
                    Column {
                        width: parent.width
                        spacing: 6

                        Row {
                            width: parent.width
                            Text {
                                text: "FUEL LEVEL"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 11
                                font.bold: true
                                color: "#94a3b8"
                            }
                            Item { width: 10; height: 1 }
                            Text {
                                text: (cluster ? cluster.fuel : 85).toString() + "%"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 13
                                font.bold: true
                                color: (cluster && cluster.lowFuelWarning) ? "#ff3333" : "#38bdf8"
                            }
                        }

                        // Progress bar for fuel
                        Rectangle {
                            width: parent.width
                            height: 10
                            color: "#162235"
                            border.color: "#20344d"
                            radius: 2

                            Rectangle {
                                width: parent.width * Math.min(1.0, Math.max(0.0, (cluster ? cluster.fuel : 85) / 100.0))
                                height: parent.height
                                radius: 2
                                color: (cluster && cluster.lowFuelWarning) ? "#ff3333" : "#38bdf8"
                            }
                        }
                    }

                    // Engine Coolant Temperature
                    Column {
                        width: parent.width
                        spacing: 6

                        Row {
                            width: parent.width
                            Text {
                                text: "COOLANT TEMP"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 11
                                font.bold: true
                                color: "#94a3b8"
                            }
                            Item { width: 10; height: 1 }
                            Text {
                                text: (cluster ? cluster.engineTemp : 75).toString() + "°C"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 13
                                font.bold: true
                                color: (cluster && cluster.engineWarning) ? "#ff3333" : "#38bdf8"
                            }
                        }

                        // Progress bar for temp (-40 to 150 range mapped)
                        Rectangle {
                            width: parent.width
                            height: 10
                            color: "#162235"
                            border.color: "#20344d"
                            radius: 2

                            Rectangle {
                                width: parent.width * Math.min(1.0, Math.max(0.0, ((cluster ? cluster.engineTemp : 75) + 40.0) / 190.0))
                                height: parent.height
                                radius: 2
                                color: (cluster && cluster.engineWarning) ? "#ff3333" : ((cluster && cluster.engineTemp > 90) ? "#f59e0b" : "#10b981")
                            }
                        }
                    }
                }
            }

            // -----------------------------------------------------------------
            // CENTER PANEL: PRIMARY SPEEDOMETER & TACHOMETER (RPM)
            // -----------------------------------------------------------------
            Rectangle {
                id: centerPanel
                anchors.left: leftPanel.right
                anchors.right: rightPanel.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 14
                color: "#0b111b"
                border.color: "#1a2c42"
                border.width: 1
                radius: 6

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    // Primary Speed Readout
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8

                        Text {
                            id: speedDigit
                            text: (cluster ? cluster.speed : 0).toString()
                            font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                            font.pixelSize: 96
                            font.bold: true
                            color: "#00e5ff"
                        }

                        Text {
                            anchors.bottom: speedDigit.bottom
                            anchors.bottomMargin: 18
                            text: "km/h"
                            font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                            font.pixelSize: 24
                            font.bold: true
                            color: "#64748b"
                        }
                    }

                    // Tachometer (RPM Bar Indicator)
                    Column {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6
                        width: 380

                        Row {
                            width: parent.width
                            Text {
                                text: "ENGINE SPEED"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 11
                                font.bold: true
                                color: "#64748b"
                            }
                            Item { width: 10; height: 1 }
                            Text {
                                text: (cluster ? cluster.rpm : 800).toString() + " RPM"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 14
                                font.bold: true
                                color: (cluster && cluster.rpm >= 6000) ? "#ff3344" : "#f1f5f9"
                            }
                        }

                        // Segmented Tachometer Bar (0 to 8000 RPM)
                        Rectangle {
                            width: parent.width
                            height: 18
                            color: "#111a28"
                            border.color: "#1e2f47"
                            radius: 3

                            Rectangle {
                                id: rpmActiveBar
                                width: parent.width * Math.min(1.0, Math.max(0.0, (cluster ? cluster.rpm : 800) / 8000.0))
                                height: parent.height
                                radius: 3
                                color: (cluster && cluster.rpm >= 6000) ? "#ff3344" : ((cluster && cluster.rpm >= 4500) ? "#f59e0b" : "#00e5ff")
                            }

                            // Redline marker at 6000 RPM (75% mark)
                            Rectangle {
                                anchors.left: parent.left
                                anchors.leftMargin: parent.width * 0.75
                                width: 2
                                height: parent.height
                                color: "#ff3344"
                            }
                        }

                        Row {
                            width: parent.width
                            Text { text: "0"; font.pixelSize: 10; color: "#475569" }
                            Item { width: 80; height: 1 }
                            Text { text: "2k"; font.pixelSize: 10; color: "#475569" }
                            Item { width: 80; height: 1 }
                            Text { text: "4k"; font.pixelSize: 10; color: "#475569" }
                            Item { width: 75; height: 1 }
                            Text { text: "6k [REDLINE]"; font.pixelSize: 10; color: "#ef4444" }
                            Item { width: 40; height: 1 }
                            Text { text: "8k"; font.pixelSize: 10; color: "#475569" }
                        }
                    }
                }
            }

            // -----------------------------------------------------------------
            // RIGHT PANEL: DRIVE MODE & ORIGINAL FEATURE (TRIP COMPUTER)
            // -----------------------------------------------------------------
            Rectangle {
                id: rightPanel
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 220
                color: "#0c131f"
                border.color: "#1a2c42"
                border.width: 1
                radius: 6

                Column {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 16

                    // Original Feature 1: Drive Mode Profile
                    Item {
                        width: parent.width
                        height: 70

                        Text {
                            text: "DRIVE PROFILE"
                            font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#64748b"
                        }

                        Rectangle {
                            anchors.right: parent.right
                            width: 120
                            height: 38
                            radius: 4
                            color: "#162235"
                            border.color: {
                                var mode = cluster ? cluster.driveMode : "COMFORT";
                                if (mode === "ECO") return "#10b981";
                                if (mode === "SPORT") return "#ff9100";
                                return "#00e5ff";
                            }
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: cluster ? cluster.driveMode : "COMFORT"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 15
                                font.bold: true
                                color: {
                                    var mode = cluster ? cluster.driveMode : "COMFORT";
                                    if (mode === "ECO") return "#10b981";
                                    if (mode === "SPORT") return "#ff9100";
                                    return "#00e5ff";
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: "#1a2c42" }

                    // Original Feature 2: Integrated Trip Computer
                    Column {
                        width: parent.width
                        spacing: 8

                        Text {
                            text: "TRIP COMPUTER"
                            font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#94a3b8"
                        }

                        // Trip Distance
                        Row {
                            spacing: 8
                            Text {
                                text: "DIST:"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 13
                                color: "#64748b"
                            }
                            Text {
                                text: (cluster ? cluster.tripDistance.toFixed(2) : "0.00") + " km"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 14
                                font.bold: true
                                color: "#f8fafc"
                            }
                        }

                        // Trip Average Speed
                        Row {
                            spacing: 8
                            Text {
                                text: "AVG: "
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 13
                                color: "#64748b"
                            }
                            Text {
                                text: (cluster ? cluster.avgSpeed.toFixed(0) : "0") + " km/h"
                                font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                                font.pixelSize: 14
                                font.bold: true
                                color: "#f8fafc"
                            }
                        }
                    }
                }
            }
        }

        // =====================================================================
        // BOTTOM ANNUNCIATOR: ACTIVE VEHICLE WARNING LIGHTS
        // =====================================================================
        Rectangle {
            id: warningAnnunciator
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            color: "#0e1624"
            border.color: "#182a40"
            border.width: 1
            radius: 8

            Row {
                anchors.centerIn: parent
                spacing: 24

                // Engine Overheat Annunciator
                Rectangle {
                    width: 150
                    height: 32
                    radius: 4
                    color: (cluster && cluster.engineWarning) ? "#450a0a" : "#131d2b"
                    border.color: (cluster && cluster.engineWarning) ? "#ef4444" : "#243447"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: (cluster && cluster.engineWarning) ? "⚠ ENG OVERHEAT" : "  ENG NORMAL"
                        font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                        font.pixelSize: 12
                        font.bold: true
                        color: (cluster && cluster.engineWarning) ? "#f87171" : "#475569"
                    }
                }

                // ABS Status Annunciator
                Rectangle {
                    width: 150
                    height: 32
                    radius: 4
                    color: (cluster && cluster.absFault) ? "#450a0a" : "#052e16"
                    border.color: (cluster && cluster.absFault) ? "#ef4444" : "#166534"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: (cluster && cluster.absFault) ? "⚠ ABS FAULT" : "✓ ABS OK"
                        font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                        font.pixelSize: 12
                        font.bold: true
                        color: (cluster && cluster.absFault) ? "#f87171" : "#4ade80"
                    }
                }

                // Low Fuel Annunciator
                Rectangle {
                    width: 150
                    height: 32
                    radius: 4
                    color: (cluster && cluster.lowFuelWarning) ? "#451a03" : "#131d2b"
                    border.color: (cluster && cluster.lowFuelWarning) ? "#f59e0b" : "#243447"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: (cluster && cluster.lowFuelWarning) ? "⛽ LOW FUEL" : "  FUEL NORMAL"
                        font.family: "Courier, 'JetBrains Mono', Consolas, monospace"
                        font.pixelSize: 12
                        font.bold: true
                        color: (cluster && cluster.lowFuelWarning) ? "#fbbf24" : "#475569"
                    }
                }
            }
        }
    }
}
