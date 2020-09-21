Sonoff S20 testing firmware with ACS712 support
----

This repo contains the esp8266-arduino firmware for the Sonoff S20 which supports
to read the ADC pin on which I've connected an ACS712-5A. This firmware is part
of my blog post [here](https://www.stupid-projects.com/hacking-a-sonoff-s20-and-adding-an-acs712-current-sensor/)

If you use a different ACS712 then you need to change the `ACS712_MVA` in the firmware
with the one that corresponds to your IC. In this case I'm using the ACS712-5A, which
can sense 5A max and with a 185mV/A resolution. Therefore the `ACS712_MVA` is set
to `185`.

## Using the firmware
To build the firmware you need Platform IO. I've tested the firmware on my Ubuntu
desktop (18.04.5) using Visual Studio Code and the PIO plugin.

To enable the `CALIBRATION_MODE` you need to remove the comment from the definition.
Using this mode you can calibrate the no-load ADC reading. Read more about it in the
post [here](https://www.stupid-projects.com/hacking-a-sonoff-s20-and-adding-an-acs712-current-sensor/).

To flash the firmware you need to press the Sonoff S20 device and then apply power.
Then the ESP8266 gets in flashing mode and you can upload the firmware.

> Be extremely careful because the device is connected to mains power!

When power is applied to the device then the green LED flashes every 500ms until the
wifi is connected. You need to specify the `ssid` and `password` in the code first.
By default and for safety reasons the relay is always OFF when the code starts.

You can use the aREST API to read the current RMS value and to set the relay status.
You can also use the physical button on the device to toggle the relay output. In the
following commands I assume that the Sonoff IP address is `192.168.0.76`.

To read the current RMS value open your web browser and browse to this URL
```
http://192.168.0.76/rms_value
```

The result should be similar to this:
```yaml
{"rms_value": 0.06, "id": "", "name": "", "hardware": "esp8266", "connected": true}
```

The `rms_value` is the real-time current RMS value. You need to refresh the page to get
a new reading.

To turn on the relay use this URL:
```
http://192.168.0.76/relay?params=1
```

The result should be similar to this:
```yaml
{"return_value": 1, "id": "", "name": "", "hardware": "esp8266", "connected": true}
```

In this case the `return_value` is the status of the relay, which is this case is on.
Now, to turn off the device use this URL:

To turn on the relay use this URL:
```
http://192.168.0.76/relay?params=0
```

The result should be similar to this:
```yaml
{"return_value": 0, "id": "", "name": "", "hardware": "esp8266", "connected": true}
```

As you can see, the `params` variable in the relay URL defines the state of the relay
and 0 is OFF and 1 is ON. The return value of the server's response is the actual value
of the `relay_status` in the code.

For more details read the post [here](https://www.stupid-projects.com/hacking-a-sonoff-s20-and-adding-an-acs712-current-sensor/)

## Maintainer
Dimitris Tassopoulos <dimtass@gmail.com>