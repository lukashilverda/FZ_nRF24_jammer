# Builden voor Momentum Firmware

Deze instructies bouwen de app als FAP tegen de Momentum Firmware release.

## Vereisten

- Linux, macOS of WSL
- Git
- Python 3
- `tar` en `curl`
- Een Momentum-compatibele Flipper Zero

## 1. Momentum ophalen

Clone Momentum met alle submodules:

```bash
git clone --depth 1 --branch release --recurse-submodules --jobs 8 \
  https://github.com/Next-Flip/Momentum-Firmware.git
cd Momentum-Firmware
```

De eerste FBT-opdracht downloadt automatisch de benodigde ARM-toolchain.

## 2. App toevoegen

Kopieer de app naar `applications_user`:

```bash
rm -rf applications_user/fz_nrf24_jammer
cp -a /pad/naar/FZ_nRF24_jammer/src/nRF24_jammer \
  applications_user/fz_nrf24_jammer
```

Controleer in `applications_user/fz_nrf24_jammer/application.fam` minimaal:

```python
apptype=FlipperAppType.EXTERNAL,
requires=["gui"],
stack_size=2 * 1024,
fap_category="GPIO/NRF24",
```

## 3. Schoon bouwen

Verwijder eerst alleen de buildresultaten van deze app en bouw daarna volgens de Momentum FBT-route:

```bash
./fbt -c build APPSRC=applications_user/fz_nrf24_jammer
./fbt build APPSRC=applications_user/fz_nrf24_jammer
```

Een geslaagde build eindigt met:

```text
APPCHK build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap
```

De FAP staat daarna op:

```text
build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap
```

## 4. Checksum controleren

Maak de checksum zichtbaar voordat je de FAP kopieert:

```bash
sha256sum build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap
```

Kopieer de FAP eventueel naar de projectmap:

```bash
cp build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap \
  /pad/naar/FZ_nRF24_jammer/fz_nrf24_jammer.fap
sha256sum /pad/naar/FZ_nRF24_jammer/fz_nrf24_jammer.fap
```

De twee hashes moeten gelijk zijn. Een nieuwe build met exact dezelfde bron kan dezelfde hash hebben; dat is normaal bij een deterministische build.

## 5. Installeren op de Flipper

Gebruik qFlipper of een andere SD-kaartverbinding en kopieer de FAP exact naar:

```text
/ext/apps/GPIO/NRF24/fz_nrf24_jammer.fap
```

Open de app via:

```text
GPIO > NRF24 > [NRF24] Jammer
```

Start de Flipper opnieuw op wanneer de categorie of app niet direct verschijnt.

## Direct bouwen en starten via USB

Wanneer een Flipper via USB is aangesloten en qFlipper gesloten is:

```bash
./fbt launch APPSRC=applications_user/fz_nrf24_jammer
```

Dit bouwt de app en probeert hem direct op het apparaat te starten.

## Problemen oplossen

### De app verschijnt niet

Controleer deze punten:

1. De bestandsnaam eindigt op `.fap`.
2. De FAP staat in `/ext/apps/GPIO/NRF24/`.
3. Er staat geen oude kopie met een andere naam in dezelfde map.
4. De Flipper is opnieuw gestart.
5. De build eindigde met `APPCHK` zonder foutmelding.

### `APPCHK` meldt ontbrekende symbolen

Bouw de app tegen dezelfde Momentum-release die op de Flipper staat. Gebruik niet een FAP die tegen de officiële Flipper-SDK of een andere Momentum-branch is gebouwd.

### De build gebruikt oude objectbestanden

Voer de schoonmaakstap opnieuw uit:

```bash
./fbt -c build APPSRC=applications_user/fz_nrf24_jammer
```

Bouw daarna opnieuw met `./fbt build APPSRC=applications_user/fz_nrf24_jammer`.
