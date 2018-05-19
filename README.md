# Cat Poop Sensor

The Cat Poop Sensor uses two gas sensors - an MQ2 smoke/gas/lpg/butane/methane sensor and an MQ136 hydrogen sulfide sensor.

It includes a PIR sensor so that we can try to corrolate presence and gaseous activity. It also includes an SHT30 temperature and humidity sensor, for good measure.

An ESP8266 polls them and periodically posts their values to an MQTT server (currently io.adafruit.com).
