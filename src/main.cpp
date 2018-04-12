#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_ADS1015.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP.h>

#include <BootstrapWebSite.h>
#include <BootstrapWebPage.h>

#include "config.h"

WiFiClient client;

ESP8266WebServer server(80);
BootstrapWebSite ws("en");

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish presence(&mqtt, AIO_USERNAME "/feeds/presence");
Adafruit_MQTT_Publish mq2(&mqtt, AIO_USERNAME "/feeds/mq2");
Adafruit_MQTT_Publish mq8(&mqtt, AIO_USERNAME "/feeds/mq8");
Adafruit_MQTT_Publish mq4(&mqtt, AIO_USERNAME "/feeds/mq4");
Adafruit_MQTT_Publish pir_feed(&mqtt, AIO_USERNAME "/feeds/pir");

Adafruit_ADS1015 adc;

void mqtt_connect(void);

unsigned long start_time;

void handleInfo(), handleIndex(), handleESP(), handleNotFound();

void setup() {
  start_time = millis();

  Wire.begin();

  pinMode(PIR_PIN, INPUT);

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


  if (MDNS.begin(MDNS_NAME)) {
    Serial.println("MDNS responder started");
  }

  //  ws.addBranding(branding_image_base64, "image/jpeg");

  ws.addPageToNav("Info", "/info");
  ws.addPageToNav("ESP", "/esp");

  server.on("/", handleIndex);
  server.on("/info", handleInfo);
  server.on("/esp", handleESP);

  server.onNotFound(handleNotFound);

  server.begin();
}

int16_t adc0, adc1, adc2, adc3, pir;

void loop() {
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      mqtt_connect();
  }

  pir = digitalRead(PIR_PIN);
  Serial.printf("PIR is %d\n", pir);

  adc0 = adc.readADC_SingleEnded(0);
  Serial.printf("ADC 0 [voltage] is %d\n", adc0);

  adc1 = adc.readADC_SingleEnded(1);
  Serial.printf("ADC 1 [mq2 - methane] is %d\n", adc1);

  adc2 = adc.readADC_SingleEnded(2);
  Serial.printf("ADC 2 [mq137 - ammonia] is %d\n", adc2);

  adc3 = adc.readADC_SingleEnded(3);
  Serial.printf("ADC 3 [mq4 - methane] is %d\n", adc3);

  //  Serial.printf("SAMPLE DELAY %d, current time %ld\n", MQ2_SAMPLE_DELAY,  millis() - start_time);

  if((millis() - start_time) > MQ2_SAMPLE_DELAY) {
    if(!mq2.publish(adc1))
      Serial.println("publish 1 failed! :(");

    if(!mq8.publish(adc2))
      Serial.println("publish 2 failed! :(");

    if(!mq4.publish(adc3))
      Serial.println("publish 3 failed! :(");

    if(!pir_feed.publish(pir))
      Serial.println("publish pir failed! :(");
  }

  server.handleClient();

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

void handleIndex() {
  BootstrapWebPage page(&ws);

  page.addHeading("Cat Poop Sensor!");
  page.addList(String("Presence ") + pir,
	       String("Methane/Butane/LPG/Smoke ") + adc1,
	       String("Methane/CNG ") + adc2,
	       String("Ammonia ") + adc3,
	       String("Voltage ") + adc0);

  server.send(200, "text/html", page.getHTML());
}

void handleInfo() {
  BootstrapWebPage page(&ws);

  page.addHeading("Info");
  page.addList(String("Uptime ") + String((millis() - start_time) / 1000) + " seconds",
	       String("IP address ") + WiFi.localIP().toString(),
	       String("Hostname ") + WiFi.hostname(),
	       String("MAC address ") + WiFi.macAddress(),
	       String("Subnet mask ") + WiFi.subnetMask().toString(),
	       String("router IP ") + WiFi.gatewayIP().toString(),
	       String("first DNS server ") + WiFi.dnsIP(0).toString(),
	       String("SSID ") + WiFi.SSID(),
	       String("RSSi ") + WiFi.RSSI());
	 
  server.send(200, "text/html", page.getHTML());
}

void handleESP() {
  BootstrapWebPage page(&ws);

  page.addHeading(String("ESP"), 1);
  page.addList(String("VCC ") + ESP.getVcc(),
               String("Free heap ") + ESP.getFreeHeap(),
               String("Chip ID ") + ESP.getChipId() ,
               String("Flash chip ID ") + ESP.getFlashChipId(),
               String("Flash chip size ") + ESP.getFlashChipSize(),
               String("Flash chip speed ") + ESP.getFlashChipSpeed(),
               String("Sketch Size ") + ESP.getSketchSize(),
               String("Free Sketch Space ") + ESP.getFreeSketchSpace());

  server.send(200, "text/html", page.getHTML());
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

