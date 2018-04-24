#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_ADS1015.h>

#include <WEMOS_SHT3X.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
// #include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <ESP.h>

#include <BootstrapWebSite.h>
#include <BootstrapWebPage.h>

#include "config.h"

extern "C" {
  #include "user_interface.h"
};


#ifdef IFTTT_KEY
#include <IFTTTWebhook.h>
#endif


WiFiClient client;

#ifdef IFTTT_KEY
IFTTTWebhook ifttt(IFTTT_KEY, IFTTT_EVENT_NAME);
#endif

ESP8266WebServer server(80);
BootstrapWebSite ws("en");

#ifdef AIO_KEY
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish presence(&mqtt, AIO_USERNAME "/feeds/presence");
Adafruit_MQTT_Publish mq2(&mqtt, AIO_USERNAME "/feeds/mq2");
Adafruit_MQTT_Publish mq137(&mqtt, AIO_USERNAME "/feeds/mq137");
Adafruit_MQTT_Publish mq4(&mqtt, AIO_USERNAME "/feeds/mq4");
Adafruit_MQTT_Publish pir_feed(&mqtt, AIO_USERNAME "/feeds/pir");
Adafruit_MQTT_Publish humidity_feed(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish temp_feed(&mqtt, AIO_USERNAME "/feeds/temperature");

void mqtt_connect(void);
#endif

Adafruit_ADS1015 adc;
SHT3X sht30(0x45);

ADC_MODE(ADC_VCC);

unsigned long start_time;

void handleInfo(), handleIndex(), handleESP(), handleNotFound();

void setup() {
  start_time = millis();

  Wire.begin();

  pinMode(PIR_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("cat poop monitor");

  Serial.println();
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  Serial.println();
  Serial.println();

  WiFi.persistent(0);
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

#ifdef IFTTT_KEY
  ifttt.trigger("boot");

  String reason = ESP.getResetReason();
  reason.replace(' ', '+');

  // from https://www.espressif.com/sites/default/files/documentation/esp8266_reset_causes_and_common_fatal_exception_causes_en.pdf
  char buffer[500] = "";
  struct rst_info* reset_info = system_get_rst_info();

  // cause: 0 - invalid command
  //        6 - division by zero
  //        9 - unaligned read/write operation
  //        28/29 - access to invalid address
  
  if(reset_info->reason == REASON_EXCEPTION_RST) {
    snprintf(buffer, 500, "+cause+%d,+epc1+0x%08x,+epc2+0x%08x,+epc3+0x%08x,+excvaddr+0x%08x,+depc+0x%08x\n",
	     reset_info->exccause, reset_info->epc1, reset_info->epc2, reset_info->epc3, reset_info->excvaddr, reset_info->depc);
  }

  ifttt.trigger("boot", reason.c_str(), buffer);
#endif

  
#ifdef AIO_KEY
  Serial.println("MQTT");
  mqtt_connect();
#endif

  Serial.println("ADC");
  adc.begin();
  adc.setGain(GAIN_ONE);

  if (MDNS.begin(MDNS_NAME)) {
    Serial.println("MDNS responder started");
  }

  ws.addPageToNav("😸💩", "/");
  ws.addPageToNav("Info", "/info");
  ws.addPageToNav("ESP", "/esp");

  server.on("/", handleIndex);
  server.on("/info", handleInfo);
  server.on("/esp", handleESP);

  server.onNotFound(handleNotFound);

  server.begin();

  ArduinoOTA.begin();
}

int16_t adc0, adc1, adc2, adc3, pir, ftemp, ctemp, humidity;
unsigned long next_sample_time = 0;

void loop() {

#ifdef AIO_KEY
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      mqtt_connect();
  }
#endif

  if(sht30.get() == 0){
    ftemp = sht30.fTemp;
    ctemp = sht30.cTemp;
    humidity = sht30.humidity;

    Serial.print("Temperature in Celsius : ");
    Serial.println(sht30.cTemp);
    Serial.print("Temperature in Fahrenheit : ");
    Serial.println(sht30.fTemp);
    Serial.print("Relative Humidity : ");
    Serial.println(sht30.humidity);
    Serial.println();
  } else {
    Serial.println("Temperature/Humidity Error!");
  }

  pir = digitalRead(PIR_PIN);
  Serial.printf("PIR is %d\n", pir);

  adc0 = adc.readADC_SingleEnded(0);
  Serial.printf("ADC 0 [voltage] is %d\n", adc0);

  adc1 = adc.readADC_SingleEnded(1);
  Serial.printf("ADC 1 [mq2 - methane] is %d\n", adc1);

  adc2 = adc.readADC_SingleEnded(2);
  Serial.printf("ADC 2 [mq4 - methane] is %d\n", adc2);

  adc3 = adc.readADC_SingleEnded(3);
  Serial.printf("ADC 3 [mq137 - ammonia] is %d\n", adc3);

  //  Serial.printf("SAMPLE DELAY %d, current time %ld\n", MQ2_SAMPLE_DELAY,  millis() - start_time);

#ifdef AIO_KEY
  if(((millis() - start_time) > MQ_FIRST_SAMPLE_DELAY) && (millis() > next_sample_time)) {
    if(adc1 != -1)
      if(!mq2.publish(adc1))
	Serial.println("publish 1 failed! :(");

    if(adc2 != -1)
      if(!mq4.publish(adc2))
	Serial.println("publish 2 failed! :(");

    if(adc3 != -1)
      if(!mq137.publish(adc3))
	Serial.println("publish 3 failed! :(");

    if(!pir_feed.publish(pir))
      Serial.println("publish pir failed! :(");

    if(!temp_feed.publish(ftemp))
      Serial.println("publish ftemp failed! :(");

    if(!humidity_feed.publish(humidity))
      Serial.println("publish humidity failed! :(");

    next_sample_time += AIO_SAMPLE_DELAY;
  }
#endif

  server.handleClient();
  ArduinoOTA.handle();
}

#ifdef AIO_KEY
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
#endif

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

  // from https://www.espressif.com/sites/default/files/documentation/esp8266_reset_causes_and_common_fatal_exception_causes_en.pdf
  char buffer[500] = "";
  struct rst_info* reset_info = system_get_rst_info();

  if(reset_info->reason == REASON_EXCEPTION_RST) {
    snprintf(buffer, 500, " cause %d, epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
	     reset_info->exccause, reset_info->epc1, reset_info->epc2, reset_info->epc3, reset_info->excvaddr, reset_info->depc);
  }

  page.addHeading(String("ESP"), 1);
  page.addList(String("SDK ") + ESP.getSdkVersion() + String(", Core ") + ESP.getCoreVersion(),
	       String("VCC ") + ESP.getVcc(),
               String("Free heap ") + ESP.getFreeHeap(),
               String("Chip ID ") + ESP.getChipId() ,
               String("Flash chip ID ") + ESP.getFlashChipId(),
               String("Flash chip size ") + ESP.getFlashChipSize(),
               String("Flash chip speed ") + ESP.getFlashChipSpeed(),
               String("Sketch Size ") + ESP.getSketchSize(),
               String("Free Sketch Space ") + ESP.getFreeSketchSpace(),
	       String("Reset reason ") + ESP.getResetReason() + buffer);

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

