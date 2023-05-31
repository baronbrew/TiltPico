# RPi-Pico-W-iBeacon-Tilt-Scanner
 
## Initial Purpose

Tilt Pico is firmware for Raspberry Pi's Pico W that syncs with Baron Brew's Tilt 2 iOS and Android app as well as supports Small Brew Companion's (SBC) Tilt 4 Windows (T4W) PC app. The idea came up when Raspberry Pi's became difficult to purchase due to supply chain constraints and customer interest increased for a Windows PC app. Since Bluetooth 4.0+ / BLE doesn't work well with early versions of Windows, i.e. Windows 7 and 8, the Tilt Pico could potentially enable that functionality, similar to Tilt Pi / Raspberry Pi 3 or 4, though for a lower cost.

## HTTP GET commands for configuring Tilt Pico

In it's basic form, the Tilt Pico runs a web server, serving a list of Tilt hydrometers as JSON objects. The key : value pairs are compatible with with the Tilt Pi allowing compatibility with T4W and potentially other software.

# List of Commands (query string)

Note: The Tilt Pico will use mDNS to assist with finding the Tilt Pico on the network. The default name is "tiltpico" and additional name's are tiltpico-02, tiltpico-03, tiltpico-04, etc.

Note: Since the Tilt 2 app can't scan for mac address, (iOS and library limitation), the Tilt Pico will default to the closes Tilt with specified UUID/color.

Set configuration example: `http://tiltpico.local/set?Color=ORANGE:F6:BD:D3:46:3A:05&Beer=Untitled&unCalSGPoints=-0.001,1.000,1.009,1.063,10.000&actualSGPoints=-0.001,1.000,1.0120,1.0800,10.000&unCalTempPoints=-1000,1000&actualTempPoints=-1000,1000&defaultcloudURL=https://script.google.com/a/baronbrew.com/macros/s/AKfycbydNOcB-_3RB3c-7sOTI-ZhTnN43Ye1tt0EFvvMxTxjdbheaw/exec&customCloudURL1=https://example1.com&customCloudURL2=https://example2.com&loggingInterval=15`


Tilt object (JSON)

First 6 lines are from iBeacon scan
Remaining lines come from user configuration or are default values.

Color key:
```
{
"RED" : "a495bb10c5b14b44b5121370f02d74de"
"GREEN" : "a495bb20c5b14b44b5121370f02d74de"
"BLACK" : "a495bb30c5b14b44b5121370f02d74de"
"PURPLE" : "a495bb40c5b14b44b5121370f02d74de"
"ORANGE" : "a495bb50c5b14b44b5121370f02d74de"
"BLUE" : "a495bb60c5b14b44b5121370f02d74de"
"YELLOW" : "a495bb70c5b14b44b5121370f02d74de"
"PINK" : "a495bb80c5b14b44b5121370f02d74de"
}
```

```
{
	"uuid": "a495bb50c5b14b44b5121370f02d74de",
	"major": 33,
	"minor": 1012,
	"tx_power": 90,
	"rssi": -28,
	"mac": "f9:15:82:6f:de:27",
	"Color": "ORANGE:f9:15:82:6f:de:27",
	
	(user config data below)
	"Beer": "Untitled,123456",
	"actualSGPoints":"1.0000,1.0125,1.0642,1.1200",
	"unCalSGPoints":"-0.001,1.000,1.0120,1.0800,10.000",
	"actualTempPoints": "-1000,1000",
	"unCalTempPoints": "-1000,1000",
	"defaultcloudURL": "https://script.google.com/a/baronbrew.com/macros/s/AKfycbydNOcB-_3RB3c-7sOTI-ZhTnN43Ye1tt0EFvvMxTxjdbheaw/exec",
	"customCloudURL1": "https://example2.com",
	"customCloudURL2": "https://example2.com",
	"loggingInterval": "15",
	"Timepoint": 45032.57079114584,
	"SG": "1.012",
	"hd": "false",
	"Temp": "33",
```

Example of sending HTTP POST to Google Sheets:

```
content-type: "application/x-www-form-urlencoded; charset=utf-8"

"Timepoint=43331.79300001158&Temp=65.0&SG=1.01&Beer=Munich%20Lager%20#1&Color=PINK&Comment="

```

Timepoint is an Excel/Google Sheets decimal format for the logging device’s local time which is calculated from JavaScript time and includes the time zone offset in the calculation. Temp is in degrees F and SG is specific gravity (at 60 F/15.6 C). Beer is the beer name, Color is the Tilt color, Comment is usually blank but can be used to add comments to specific logging points as well as start new logs and send email alerts.
