#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_ADS1015.h>

#include "config.h"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish presence(&mqtt, AIO_USERNAME "/feeds/presence");
Adafruit_MQTT_Publish mq2(&mqtt, AIO_USERNAME "/feeds/mq2");
Adafruit_MQTT_Publish mq8(&mqtt, AIO_USERNAME "/feeds/mq8");
Adafruit_MQTT_Publish mq4(&mqtt, AIO_USERNAME "/feeds/mq4");
Adafruit_MQTT_Publish temperature(&mqtt, AIO_USERNAME "/feeds/temperature");

Adafruit_ADS1015 adc;

void mqtt_connect(void);

unsigned long start_time;

void setup() {
  start_time = millis();

  Wire.begin();

  Serial.begin(115200);
  Serial.println("cat poop monitor");

  Serial.println(); Serial.println();
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  Serial.print("mac address: ");
  Serial.println(WiFi.macAddress());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("derp");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("ADC");
  mqtt_connect();

  adc.begin();
  adc.setGain(GAIN_ONE);
}


void loop() {
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      mqtt_connect();
  }

  if(millis() - start_time > MQ2_SAMPLE_DELAY) {
    int16_t adc0 = adc.readADC_SingleEnded(0);
    Serial.printf("ADC 0 is %d\n", adc0);

    int16_t adc1 = adc.readADC_SingleEnded(1);
    Serial.printf("ADC 1 is %d\n", adc1);

    if(!mq2.publish(adc1))
      Serial.println("publish 1 failed! :(");
    else
      Serial.println("publish 1 worked! :)");

    int16_t adc2 = adc.readADC_SingleEnded(2);
    Serial.printf("ADC 2 is %d\n", adc2);

    if(!mq8.publish(adc2))
      Serial.println("publish 2 failed! :(");
    else
      Serial.println("publish 2 worked! :)");

    int16_t adc3 = adc.readADC_SingleEnded(3);
    Serial.printf("ADC 3 is %d\n", adc3);

    if(!mq4.publish(adc3))
      Serial.println("publish 3 failed! :(");
    else
      Serial.println("publish 3 worked! :)");
  }

  delay(10000);
}

void mqtt_connect(void) {
  int8_t ret;

  Serial.print("Connecting to Adafruit IO... ");

  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println("Wrong protocol"); break;
      case 2: Serial.println("ID rejected"); break;
      case 3: Serial.println("Server unavail"); break;
      case 4: Serial.println("Bad user/pass"); break;
      case 5: Serial.println("Not authed"); break;
      case 6: Serial.println("Failed to subscribe"); break;
      default: Serial.println("Connection failed"); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println("Retrying connection...");
    delay(5000);

  }

  Serial.println("Adafruit IO Connected!");

}
