# B15F - Board 15 Famulus Edition

## Dokumentation
Die **Projekt-Dokumentation** befindet sich hier: [https://devfix.github.io/b15f/](https://devfix.github.io/b15f/).

## Über diesen Fork
Dieser Fork maintained **nicht** das originale b15f repository. Der controller und firmware code wird von mir nicht weiter verändert oder entwickelt. Dieser Fork stellt lediglich ein C binding für die API des B15 zur verfügung.

## TODO / Ideen
- [X] CLI: Exception catchen, set global error message, raise SIGINT --> ncurses wird richtig beendet
- [x] Main Menu: Informationen ergänzen
- [X] Selbsttest bei discard richtig beenden (momentan wird wahrscheinlich WDT angeschmissen, besser global bool für selbsttest-loop)
- [x] Lizenz
- [x] gitignore checken
- [x] readme schreiben
- [ ] globale strings / msg klasse f?r treiber, ui (z.B. B15F info)
- [x] drv: requests als array organisieren
- [x] drv/usart überarbeiten, evtl iotl select
- [ ] Kommentieren
- [x] Servo ansteuert
- [x] Interrupt Counter
- [x] mem16 testen
- [x] autocheck request size
- [x] Raspberry Pi test

