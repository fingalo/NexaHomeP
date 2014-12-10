NexaHomeP
=========

NexaHome Pebble app.

(Based on project: mickeprag/telldus-live-pebble).

- The middle key will toggle status of the devices.
- One short keypress will turn devices on / off.
- Long keypress on dimmer device will change dimmer values 25% for each long keypress.
- Long presses on device (not dimmer) will show (toggle) time stamp / event time
- Long press on sensor will show time stamp for 5 seconds (instead of sensor name).
- Another long press during the 5 seconds will toggle display format for sensors (big / small).

Short keypress on middle key in menu Environment and Events will expand/collapse the menu.

Configuratin screen:
Configuration of sensors allow to show the tempaerature and the hunidity on the same line.
In the configuration screen all sensors are listed with the Nexahome device id..
ex. If your temperature sensor ID is 51 and hunidity sensor id is 52 you enter 52 in the configuration field for sensor 51.
Also uncheck the sensor 52 checkbox in the configuraion screen.
![Ex:](https://dl.dropboxusercontent.com/u/29205101/Screenshot_2014-12-10-22-29-34.png)

1.3
- Added events list from Nexahome.
- Confgure show/hide for section sensors, devices and events.
 
1.4 
- Not published.

1.5
- Fixed passwordhandling
- Added timestamps on sensors and devices.

1.6
- Fixed dimmer values when long press.
- Added configuration for sensors, with temp / hum on same line.




