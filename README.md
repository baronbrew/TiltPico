# RPi-Pico-W-iBeacon-Tilt-Scanner
 
## Initial Purpose

Tilt Pico is firmware for Raspberry Pi's Pico W that syncs with Baron Brew's Tilt 2 iOS and Android app as well as supports Small Brew Companion's (SBC) Tilt 4 Windows (T4W) PC app. The idea came up when Raspberry Pi's became difficult to purchase due to supply chain constraints and customer interest increased for a Windows PC app. Since Bluetooth 4.0+ / BLE doesn't work well with early versions of Windows, i.e. Windows 7 and 8, the Tilt Pico could potentially enable that functionality, similar to Tilt Pi / Raspberry Pi 3 or 4, though for a lower cost.

## HTTP GET commands for configuring Tilt Pico

In it's basic form, the Tilt Pico runs a web server, serving a list of Tilt hydrometers as JSON objects. The key : value pairs are compatible with with the Tilt Pi allowing compatibility with T4W and potentially other software.

# List of Commands (query string)

Note: The Tilt Pico will use mDNS to assist with finding the Tilt Pico on the network. The default name is "tiltpico" and additional name's are tiltpico-02, tiltpico-03, tiltpico-04, etc.

Note: Since the Tilt 2 app can't scan for mac address, (iOS and library limitation), the Tilt Pico will default to the closes Tilt with specified UUID/color.

Set configuration example: http://tiltpico:1880/set?mac=closest&color=ORANGE&Beer=Kegerator%20Temperature%2C84884&actualSGPoints=1.0000%2C1.0125%2C1.0642%2C1.1200&unCalSGPoints=0.9980%2C1.0121%2C1.0653%2C1.1171&actualTempPoints=31.5%2C67.1&unCalTempPoints=32.0%2C68.3&Timepoint=45032.57079114584&timeStamp=1681677716355&tempunits=%C2%B0F&fermunits=


Tilt object (JSON)

First 6 lines are from iBeacon scan
Remaining lines come from user configuration or are default values.

```
{
	"uuid": "a495bb50c5b14b44b5121370f02d74de",
	"major": 332,**
	"minor": 10121,**
	"tx_power": 90,**
	"rssi": -28,**
	"mac": "f9:15:82:6f:de:27",
	"Color": "ORANGE:f9:15:82:6f:de:27",
	(user config data below)
	"Beer": ["Kegerator Temperature", "84884"],
	"actualSGPoints":"1.0000,1.0125,1.0642,1.1200",
	"unCalSGPoints":"0.9980,1.0121,1.0653,1.1171",
	"actualTempPoints": "31.5,67.1",
	"unCalTempPoints": "32.0,68.3",
	(NTP time server data below)
	"timeStamp": 1681677716355,
	"formatteddate": "4/16/2023, 13:41:56",
	"Timepoint": 45032.57079114584,
	(calculated data)
	"SG": 1.0121,
	"hd": true,
	"Temp": "33.2",
	(additional user config data)
	"tempunits": "°F",
	"fermunits": "",
	"customcloudURL": "https://script.google.com/macros/s/AKfycbwNXh6rEWoULd0vxWxDylG_PJwQwe0dn5hdtSkuC4k3D9AXBSA/exec",
	"defaultcloudURL": ["https://script.google.com/macros/s/AKfycbwNXh6rEWoULd0vxWxDylG_PJwQwe0dn5hdtSkuC4k3D9AXBSA/exec", false],
	"logCloudDataCheck": true,
	"logLocalDataCheck": true,
	"localloggingInterval": 38.6,
	"loggingInterval": 30
}
```
