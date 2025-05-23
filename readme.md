# TelemetryCore

Part of Texas A&M Formula SAE Telemetry Network.

TelemetryCore is a fast telemetry proxy used to bridge information from the car's network to the base station.

<!--![readthedocs](readthedocs.jpg)-->
<!-- <img src="readthedocs.jpg" width="400px"> -->
![structure](pics/structure.jpg)

<!--Questions: contact Justus <jus@justusl.com>-->

## Quickstart 

Assuming you have `telemtrycore.exe` installed at some path in your system, here is you can get started using the service.

The first step is to ensure that we have the important resources:

1. We need the binary somewhere in the filesystem, for ex. `/path/to/telemetrycore.exe`.
2. We need a valid config file somewhere, for ex. `/path/to/configfile.cfg`.
3. We need an asset directory with the distributable HTML, JavaScript, and CSS for the telemetry client, for ex. `path/to/assetsdir`.

**Note:** you might want to become familiar with the various command line arguments. For usage, run `/path/to/telemtrycore.exe -h`.

Lets review the important arguments that are required to run the service:

* `-s` specify the serial port, this is required.
* `-c` specify the config file path, this is also required.
* `-a` specify the assets directory path, default is `assets` (means `${PWD}/assets`, if you don't understand this google it), but this should probably be required.

There are other important arguments, so run `/path/to/telemtrycore.exe -h` to view them.

## FTDI Driver

You need to install the FTDI Driver for Windows builds. We no longer use serial (some of the legacy code still references it).

The following libraries for your architecture must be in these locations in the project:

* `lib/ftd2xx.lib`  static linked library, you can try:
  * arm: `cp ftdi/arm/ftd2xx.lib lib/ftd2xx.lib`
  * x64: `cp ftdi/x64/ftd2xx.lib lib/ftd2xx.lib`
* `bin/ftd2xx.dll`  dynamic linked library
  * arm: `cp ftdi/arm/ftd2xx.dll bin/ftd2xx.dll`
  * x64: `cp ftdi/x64/ftd2xx.dll bin/ftd2xx.dll`

File candidates are in the `ftdi` directory. 
