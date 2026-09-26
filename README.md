# FZ nRF24 Jammer

Flipper Zero application for nRF24 modules, built for Momentum Firmware. The app supports multiple modules and several test modes for Bluetooth, BLE, WiFi, Zigbee, drones, and custom channel ranges.

Use RF equipment only where legally permitted and in a way that does not interfere with other users.

## Project structure

```text
application.fam          Momentum application manifest
nRF24_jammer.c           App logic, menu input, and radio control
nRF24_jammer_ui.h        UI declarations
lib/nrf24/nrf24.c        nRF24 hardware library
lib/nrf24/nrf24.h        nRF24 hardware interface
images/                  Application assets
icon.png                 Application icon
dist/                    Built FAP files
```

## Build for Momentum

Requirements:

- Linux, macOS, or WSL
- Git
- Python 3
- A Momentum Firmware checkout

Clone Momentum from the release branch with its submodules:

```bash
git clone --depth 1 --branch release --recurse-submodules --jobs 8 \
  https://github.com/Next-Flip/Momentum-Firmware.git
cd Momentum-Firmware
```

Copy this app into `applications_user`:

```bash
rm -rf applications_user/fz_nrf24_jammer
cp -a /path/to/FZ_nRF24_jammer \
  applications_user/fz_nrf24_jammer
```

Clean-build only this application:

```bash
./fbt -c build APPSRC=applications_user/fz_nrf24_jammer
./fbt build APPSRC=applications_user/fz_nrf24_jammer
```

A successful build ends with `APPCHK`. The FAP is written to:

```text
build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap
```

Optionally copy the FAP back into the project:

```bash
mkdir -p /path/to/FZ_nRF24_jammer/dist
cp build/f7-firmware-C/.extapps/fz_nrf24_jammer.fap \
  /path/to/FZ_nRF24_jammer/dist/fz_nrf24_jammer.fap
sha256sum /path/to/FZ_nRF24_jammer/dist/fz_nrf24_jammer.fap
```

You can also build and launch the app directly over USB:

```bash
./fbt launch APPSRC=applications_user/fz_nrf24_jammer
```

## Install

Using qFlipper, copy `dist/fz_nrf24_jammer.fap` to:

```text
/ext/apps/GPIO/NRF24/fz_nrf24_jammer.fap
```

Open the app through:

```text
GPIO > NRF24 > [NRF24] Jammer
```

## Controls

- `Up` and `Down`: previous/next item
- `Left` and `Right`: change a setting or adjust a channel
- `OK`: open a menu or start an action
- `Back`: go back or stop an active action
- Hold a channel control: adjust more quickly

## Wiring

The app supports configurations with one, two, or four nRF24 modules. See the schematics in the repository for GPIO connections and use a suitable external 3.3 V power supply for the modules.

## Build verification

After building, run at least:

```bash
git diff --check
sha256sum dist/fz_nrf24_jammer.fap
```

Build the FAP against the same Momentum release that runs on the target Flipper Zero.
