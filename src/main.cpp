#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <Bounce2.h>

// #define CALIBRATION_MODE
#define CALIBRATION_VALUE 431
#define ACS712_MVA    0.185   // This means 185mV/A, see the datasheet for the correct value
#define R1            220.0
#define R2            56.0
#define ADC_MAX_VAL   1023.0

#ifdef CALIBRATION_MODE
  #define NUM_SAMPLES 5000
#else
  #define NUM_SAMPLES 1000
#endif
#define NUM_BUTTONS 1
#define SERVER_PORT 80

//Yout Wifi SSID
const char* ssid = "";
//Your Wifi Key
const char* password = "";

char msg[256];

enum en_GPIOS {
  GPIO_LED = 13,
  GPIO_RELAY = 12,
  GPIO_ADC = A0,
  GPIO_BUTTON = 0
};

/* Buttons */
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {GPIO_BUTTON};

/* Relay status */
int relay_status = 0;

/* ADC variables */
unsigned long next_sample = 0;
unsigned long avg_sample = 0;
unsigned long cur_sample = 0;
long result = 0;
float rms_value = 0.0;
const float k_const = (1/ADC_MAX_VAL) * ((R1 + R2) / R2) * (1/ACS712_MVA);
const unsigned long num_of_samples = NUM_SAMPLES;

aREST rest = aREST();
WiFiServer * server;
Bounce * buttons = new Bounce[NUM_BUTTONS];

int restapi_relay_control(String command);

void SetRelay(bool onoff)
{
  if (onoff) {
    relay_status = 1;
    digitalWrite(GPIO_RELAY, HIGH);
  }
  else {
    relay_status = 0;
    digitalWrite(GPIO_RELAY, LOW);
  }
  Serial.println("Relay status: " + String(relay_status));
}

ICACHE_RAM_ATTR void setup() {
  /* setup LED */
  pinMode(GPIO_LED, OUTPUT);
  digitalWrite(GPIO_LED, LOW);
  /* Setup relay. Default is OFF */
  pinMode(GPIO_RELAY, OUTPUT);
  SetRelay(false);
  /* Setup ADC */
  pinMode(GPIO_ADC, INPUT);
  /* Setup button */
  pinMode(GPIO_BUTTON, INPUT);

  /*Setup serial */
  Serial.begin(115200);
	delay(1000);

  /* Connect to wifi */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(GPIO_LED, HIGH);
    delay(500);
    digitalWrite(GPIO_LED, LOW);
    Serial.println("Connecting..");
  }
  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());

  digitalWrite(GPIO_LED, LOW);    // Enable LED

  /* Setup aREST */
  rest.function(const_cast<char*>("relay"), restapi_relay_control);
  rest.variable("rms_value", &rms_value);

  /* Start the server */
  server = new WiFiServer(SERVER_PORT);
  server->begin();
  Serial.println("Server started");

  /* Setup button bounce */
  buttons[GPIO_BUTTON].attach( BUTTON_PINS[0] , INPUT_PULLUP  );       //setup the bounce instance for the current button
  buttons[GPIO_BUTTON].interval(25);              // interval in ms
 
#ifdef CALIBRATION_MODE
  Serial.print("Calibrating with ");
  Serial.print(num_of_samples);
  Serial.println(" samples.");
#endif
}

ICACHE_RAM_ATTR void common_loop()
{
  buttons[GPIO_BUTTON].update();
  // If it fell, flag the need to toggle the LED
  if ( buttons[GPIO_BUTTON].fell() ) {
    Serial.println("Button pressed");
    SetRelay(!relay_status);
  }
}

/** Control relay using aREST
* Turn on: http://192.168.0.76/relay?params=1
* Turn off: http://192.168.0.76/relay?params=0
*/
ICACHE_RAM_ATTR int restapi_relay_control(String command)
{
  SetRelay(command.toInt());
  return relay_status;
}

#ifdef CALIBRATION_MODE
ICACHE_RAM_ATTR void loop()
{
  if (millis() > next_sample + 1) {
    avg_sample += analogRead(GPIO_ADC);
    if ((cur_sample++) >= num_of_samples) {
      result = avg_sample / cur_sample;
      Serial.print("Calibration value: ");
      Serial.println(result);
      result = avg_sample = cur_sample = 0;
    }
    next_sample = millis();
  }
  common_loop();
}
#else
ICACHE_RAM_ATTR void loop()
{
  /* Sample ADC every 1ms */
  if (millis() > next_sample + 1) {
    int adc_val = analogRead(GPIO_ADC);
    avg_sample += sq(adc_val - CALIBRATION_VALUE);
    if ((cur_sample++) >= num_of_samples) {
      rms_value = sqrt(avg_sample/num_of_samples) * k_const;
      Serial.println("RMS value: " + String(rms_value));
      result = avg_sample = cur_sample = 0;

      /* Check for aREST requests */
      WiFiClient client = server->available();
      if (client) {
          while(!client.available()){
              delay(50);
          }
          rest.handle(client);
      }
    }
    next_sample = millis();
  }
  common_loop();
}
#endif