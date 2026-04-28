# CLAUDE.md — BKOS-blanco

Firmware kiezer voor de ESP32-S3 boordcomputer.
Doel: WiFi-verbinding instellen en via OTA een van de beschikbare BKOS versies installeren.

Actieve repo: `https://github.com/brennyc86/BKOS-blanco`

---

## Werkwijze

Na elk afgerond stuk werk: committen en pushen naar `main`. GitHub Actions compileert dan de firmware.bin.

```bash
git add BKOS_blanco/<bestanden>
git commit -m "vX: korte omschrijving"
git push
```

Versienummer bijwerken in `app_state.h` (`BLANCO_VERSIE`) en `versie.txt` bij elke push.

### Versienummer formaat: `MAJOR.MINOR.YYMMDD.I`

---

## Compileren

- **Arduino IDE** + **ESP32 Arduino Core versie 2.x**
- Board: `ESP32S3 Dev Module`
- PSRAM: OPI PSRAM
- Partition scheme: `8M Flash (3MB APP / 2MB SPIFFS)`
- Libraries: `Arduino_GFX_Library`, `TAMC_GT911`

---

## Architectuur

### Bestandsstructuur in `BKOS_blanco/`

```
BKOS_blanco.ino     ← entry point: setup() → hw_setup(), loop() → hw_loop()
hardware.h/.ino     ← coordinatie: init, touch dispatch, scherm-switch
app_state.h/.ino    ← globale staat: actief_scherm, scherm_bouwen, wifi_verbonden
ui_colors.h         ← RGB565 kleurpalet (#defines, geen runtime pallette)
hw_scherm.h/.ino    ← display init + idle dimming (800×480, 16MHz pclk)
hw_touch.h/.ino     ← GT911 touchscreen init en uitlezen
ui_draw.h/.ino      ← ui_knop(), ui_tekst_midden(), ui_header()
wifi.h/.ino         ← WiFi verbinden, Preferences opslaan, wifi_reset()
screen_wifi.h/.ino  ← WiFi scherm: netwerk scan + volledig toetsenbord (HOOFD/klein/SYM)
screen_kies.h/.ino  ← Firmware kiesscherm: 4 kaarten met versie-check + INSTALLEREN
screen_flash.h/.ino ← Download + OTA flash met voortgangsbalk
versie.txt          ← actuele versie (zelfde als BLANCO_VERSIE in app_state.h)
firmware.bin        ← gegenereerd door GitHub Actions
```

### Schermen

| Enum         | Beschrijving |
|---|---|
| SCHERM_KIES  | Hoofdscherm: 4 firmware-kaarten, VERSIES + WIFI knoppen in header |
| SCHERM_WIFI  | WiFi scan + volledig toetsenbord voor wachtwoord |
| SCHERM_FLASH | Voortgangsscherm tijdens download + flash |

### Firmware tabel (screen_kies.ino)

| Naam | Auteur | Versie URL | Bin URL |
|---|---|---|---|
| BKOS-NUI | Claude Code | `brennyc86/BKOS-NUI/.../versie.txt` | `.../firmware.bin` |
| BKOS4 | Brendan Koster | `BrendanKoster86/BKOS4/main/versie.txt` | `.../firmware.bin` |
| BKOS5a | Agent Zero | `brennyc86/BKOS5a/main/versie.txt` | `.../firmware.bin` |
| BKOS-blanco | Claude Code | `brennyc86/BKOS-blanco/.../versie.txt` | `.../firmware.bin` |

> **Let op**: de URLs voor BKOS4 en BKOS5a zijn aannames. Controleer de map-structuur
> in die repo's als de versie-fetch een fout geeft (knop VERSIES → kaart toont "fout 404").

### Toetsenbord (screen_wifi.ino)

Drie modi:
- **HOOFD**: hoofdletters (standaard)
- **klein**: kleine letters
- **SYM**: speciale tekens voor WiFi-wachtwoorden (`!"#$%&...`)

Knoppen: DEL · CLR · HOOFD/klein · SYM/ABC · SPATIE · VERBINDEN · ANNULEER

---

## Conventies

- Taal in code: Nederlands
- Naamgeving: `screen_X_teken()` / `screen_X_run()` voor schermen
- Versieformaat: `MAJOR.MINOR.YYMMDD.I`
- Geen Serial.print in productie
- Compileer altijd met 8MB partitieschema

## GitHub Actions — FQBN

```
esp32:esp32:esp32s3:PSRAM=opi,PartitionScheme=default_8MB,FlashSize=16M,FlashMode=qio,FlashFreq=80,CDCOnBoot=cdc
```

Controleer de exacte FQBN via Arduino IDE verbose output en pas `build.yml` aan als het afwijkt.
