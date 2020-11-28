/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 16
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;
double tempC;
double presC;
double humiC;
double humiS;
String jsonP;
char msg[1300];
int value = 0;
String mainCommand;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "*******"
#define WLAN_PASS       "*******"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "hairdresser.cloudmqtt.com"
#define AIO_SERVERPORT  15489
#define AIO_USERNAME    "*******"
#define AIO_KEY         "*******"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");


// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/


StaticJsonBuffer<1300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();


// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c); // Указываем адрес дисплея
    pinMode(D5, OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println(F("Adafruit MQTT start"));

    if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
    }

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      mainCommand = (String)((char *)onoffbutton.lastread);
      Serial.println(mainCommand);
      //Serial.println((char *)onoffbutton.lastread);
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\nSending json = "));
  getTemperature();
  getHumidity();
  getPressure();
  analogSense();
  Serial.print(tempC);
  Serial.print(humiC);
  Serial.print(presC);
  Serial.print(humiS);
  //outDisplay();

  readCommandFromOperator();

  ++value;
  JSONencoder["id"] = value;
  JSONencoder["pressure"] = presC;
  JSONencoder["air_temperature"] = tempC;
  JSONencoder["air_humidity"] = humiC;
  JSONencoder["soil_humidity"] = humiS;
  JSONencoder["date"] = "28-11-2020";

  char JSONmessageBuffer[500];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
  
  Serial.print("...");
  if (! temperature.publish(JSONmessageBuffer))            // Здесь указываем переменную, которую хотим отправить 
  {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void getTemperature()
{
  tempC = bme.readTemperature();
  delay(100);
}

void getHumidity()
{
  presC = bme.readHumidity();
  delay(100);
}

void getPressure()
{
  humiC = bme.readPressure() / 100.0F;
  delay(100);
}


void analogSense(){
  float s1 = analogRead(A0);
  humiS =((1024-s1)/1024)*100;
}

void outDisplay()
{
  display.clearDisplay();                   // Очищаем дисплей
  display.setTextSize(1);                   // Устанавливаем размер шрифта
  display.setTextColor(WHITE);              // Цвет фона
  display.setCursor(0,0);                   // Установить позицию x, y для отображения
  display.println(" OLED 0.96 TESTER ");    // Отправка текста
  display.setCursor(0,10);                  // Установить позицию x, y для отображения 
  display.setTextSize(1);                   // Устанавливаем размер шрифта
  display.setTextColor(WHITE);              // Установить белый текст, черный фон
  display.println(humiS);                   // Отправка текста
  display.display();     
}

void readCommandFromOperator() {
  Serial.println(mainCommand);
  if(mainCommand == "forced") {
    pompOn();
    delay(2000);
    pompOff();
    mainCommand = "empty";
  }
  if(mainCommand == "empty"){
      if(humiS<=85){ // <= 45 Pompa ON
        pompOn();
      } else if(humiS>85){ // > 45 Pompa Off
        pompOff();
      }
  } else if(mainCommand == "disable") {
    mainCommand = "disable";
  }

  if(mainCommand == "resume" || mainCommand != "disable") {
    mainCommand = "empty";
  }
}

void pompOff()
{
  digitalWrite(D5, LOW);
}

void pompOn()
{
  digitalWrite(D5, HIGH);
}
