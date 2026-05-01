# Modifications à faire dans mainwindow.cpp

## Remplacer la fonction handleArduinoPortLine() (vers ligne 5824)

**REMPLACER CETTE FONCTION EN ENTIER:**

```cpp
void MainWindow::handleArduinoPortLine(const QString& line)
{
    const QString msgOrig = line.trimmed();
    
    // Parse DISTANCE from sensor: "DISTANCE:<cm>"
    if (msgOrig.startsWith("DISTANCE:")) {
        bool ok = false;
        int distCm = msgOrig.mid(9).toInt(&ok);
        if (!ok || distCm < -1) return;

        m_lastKnownDistanceCm = distCm;

        if (distCm < 0) {
            if (m_portConsole) m_portConsole->appendPlainText("[SENSOR] Error");
            return;
        }

        if (m_portConsole) {
            m_portConsole->appendPlainText(QStringLiteral("[SENSOR] %1cm").arg(distCm));
        }

        // State machine based on distance thresholds
        const int DETECT = 50;    // Boat detected
        const int DOCK = 20;      // Boat at dock
        const int DEPART = 80;    // Boat left

        // ARRIVAL DETECTION
        if (distCm < DETECT && !m_boatPresent) {
            m_boatPresent = true;
            m_boatDocked = false;
            m_arrivalDetectedAt = millis();
            m_currentAssignedQuaiId = -1;

            if (m_portConsole) m_portConsole->appendPlainText("[STATE] ARRIVAL");
            if (m_arduino) m_arduino->writeLine("BATEAU_DETECTE");

            const int quaiId = findAndReserveFreeQuai();
            if (quaiId > 0) {
                m_currentAssignedQuaiId = quaiId;
                if (m_arduino) m_arduino->writeLine(QStringLiteral("QUAI:%1").arg(quaiId));
            } else {
                if (m_arduino) m_arduino->writeLine("COMPLET");
            }
            return;
        }

        // DOCKING DETECTION
        if (distCm <= DOCK && m_boatPresent && !m_boatDocked) {
            m_boatDocked = true;
            if (m_portConsole) m_portConsole->appendPlainText("[STATE] DOCKED");
            if (m_arduino) m_arduino->writeLine("BATEAU_GARE");
            return;
        }

        // DEPARTURE DETECTION
        if (distCm > DEPART && m_boatPresent) {
            m_boatPresent = false;
            m_boatDocked = false;
            if (m_portConsole) m_portConsole->appendPlainText("[STATE] DEPARTED");
            if (m_arduino) m_arduino->writeLine("BATEAU_PARTI");

            if (m_currentAssignedQuaiId > 0) {
                markQuaiFree(m_currentAssignedQuaiId);
                m_currentAssignedQuaiId = -1;
            }
            return;
        }
    }
}
```

---

## Compilation

Assurez-vous que toutes les variables membres sont déclarées dans mainwindow.h:
- `int m_lastKnownDistanceCm = -1;`
- `int m_currentAssignedQuaiId = -1;`
- `bool m_boatPresent = false;`
- `bool m_boatDocked = false;`
- `unsigned long m_arrivalDetectedAt = 0;`

Ces variables ont déjà été ajoutées via les modifications antérieures.

---

## Flot complet

1. **Arduino envoie distances brutes** → `DISTANCE:45`
2. **Qt reçoit dans handleArduinoPortLine()**
3. **Qt analyse la distance** et détermine état (ARRIVEE/DOCK/DEPART)
4. **Qt query la DB** → `findAndReserveFreeQuai()` ou `markQuaiFree()`
5. **Qt envoie commandes LCD à Arduino** → `QUAI:263001`, `COMPLET`, `BATEAU_GARE`, `BATEAU_PARTI`
6. **Arduino affiche sur LCD**
7. **Console Qt** affiche logs (`[SENSOR]`, `[STATE]`, `[DB]`, etc.)
