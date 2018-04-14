// WiFi parameters
#define WLAN_SSID       "YOUR WIFI NETWORK"
#define WLAN_PASS       "YOUR WIFI PASSWORD"
 
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
