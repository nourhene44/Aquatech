Arduino detection bateau

Fichiers inclus :
- `Arduino_Detection_Bateau_QtControle.ino` : version a utiliser avec l'integration Qt ajoutee dans `mainwindow.cpp`. Cette version attend `START` / `STOP` et envoie `PULSE:<microseconds>`.
- `Arduino_Final_Code.ino` : version du zip basee sur `DISTANCE:<cm>`.
- `Arduino_Code_Minimal.ino` : version simplifiee du zip pour test rapide.

Materiel attendu :
- Capteur ultrason : `TRIG=9`, `ECHO=10`
- Ecran LCD I2C `0x27`
- Vitesse serie : `9600`

Notes :
- Repartition voulue dans le projet :
  `COM7 = RFID`, `COM8 = detection bateau`, `COM5 = temperature`.
- Si `COM7`, `COM8` et `COM5` sont presents, le code Qt les choisit en priorite dans cet ordre.
- Les trois ports sont geres separement pour pouvoir fonctionner en meme temps.
