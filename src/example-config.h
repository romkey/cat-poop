// WiFi parameters
#define WLAN_SSID       "YOUR WIFI NETWORK"
#define WLAN_PASS       "YOUR WIFI PASSWORD"
 
// IFTTT
// comment out the IFTTT_KEY #define if you don't want to use IFTTT
#define IFTTT_KEY        "YOUR IFTTT KEY"
#define IFTTT_EVENT_NAME "cat-poop_update"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR ADAFRUIT IO USERNAME"
#define AIO_KEY         "YOUR ADAFRUIT IO API KEY"

// MQ2 Sensor
// ignore the first MQ2_SAMPLE_DELAY ms of samples
#define MQ_SAMPLE_DELAY 5000

// AIO allows 60 data points per minute, we upload 4 at a time
// so we need a delay of 15 seconds
#define AIO_SAMPLE_DELAY 15*1000

#define PIR_PIN           D7
