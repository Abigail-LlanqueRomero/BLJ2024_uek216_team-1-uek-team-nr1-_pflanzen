#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 

#define OLED_SDA 21
#define OLED_SCL 22

#define MOISTURE_PIN 15 
#define RED_PIN 5
#define GREEN_PIN 18
#define BLUE_PIN 19

#define THRESHOLD 530




#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 

#define OLED_SDA 21
#define OLED_SCL 22

#define LIGHT_PIN 34
#define MOISTURE_PIN 15 

const char* device_id = "abigail";
const char* ssid = "GuestWLANPortal";
const char* mqtt_server = "10.10.2.127";
const char* topic1 = "zuerich/pflanzen/Temperatur/in";
const char* topic2 = "zuerich/pflanzen/moisture/in";
const char* topic3 = "zuerich/pflanzen/Temperatur/out"; 

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_AHTX0 aht;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_PAGEADDR, OLED_SDA, OLED_SCL)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  setup_aht();
  display.setTextColor(SSD1306_WHITE);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(LIGHT_PIN, INPUT);
  pinMode(MOISTURE_PIN, INPUT);

  Serial.println("Initializing sensors...");

  if (!aht.begin()) {
    Serial.println("Failed to find AHT10/AHT20 sensor!");
    display.println("AHT sensor failed!");
    display.display();
  } else {
    Serial.println("AHT10/AHT20 sensor initialized.");
    display.println("AHT10/AHT20 initialized.");
    display.display();
  }
}

void setup_aht() {
 Serial.print("Searching AHT10 / AHT20...");
 while(!aht.begin()) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("done!");
}

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, topic2) == 0) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    Serial.print("Received Message: ");
    Serial.println(message);
  }
}

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  while (!client.connected()) {
    if (client.connect(device_id)) {
      Serial.println("Connected to MQTT broker!");
      client.subscribe(topic1);
      client.subscribe(topic2);
    } else {
      delay(500);
      Serial.print(".");
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float light_reading = analogRead(LIGHT_PIN);
  Serial.print("Light Reading: ");
  Serial.println(light_reading);

  int moisture_reading = analogRead(MOISTURE_PIN);
  Serial.print("Soil Moisture: ");
  String moisture_status;
  if (moisture_reading > THRESHOLD) {
    moisture_status = "DRY";
    Serial.print("DRY (");
  } else {
    moisture_status = "WET";
    Serial.print("WET (");
  }
  Serial.print(moisture_reading);
  Serial.println(")");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Soil Sensor Status");
  display.print("Moisture: ");
  display.print(moisture_status);
  display.println();
  display.print("Moisture Val: ");
  display.println(moisture_reading);
  display.print("Light: ");
  display.println(light_reading);

  if (moisture_reading > THRESHOLD) {
    display.println("WATER NEEDED!");
  }

  display.display();

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  char temp_str[8];
  dtostrf(temp.temperature, 1, 2, temp_str);  
  client.publish(topic1, temp_str); 

  char moisture_str[8];
  itoa(moisture_reading, moisture_str, 10); 
  client.publish(topic2, moisture_str); 

  delay(10000);  
}
