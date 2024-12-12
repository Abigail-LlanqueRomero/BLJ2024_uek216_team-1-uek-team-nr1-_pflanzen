#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_SDA 21
#define OLED_SCL 22

#define MOISTURE_PIN 33 
#define RED_PIN 5
#define GREEN_PIN 18
#define BLUE_PIN 19

#define THRESHOLD 530

const char* device_id = "abigail";
const char* ssid = "GuestWLANPortal";
const char* mqtt_server = "10.10.2.127";
const char* topic1 = "zuerich/pflanzen/Temperatur/in";
const char* topic2 = "zuerich/pflanzen/moisture/in";
const char* topic3 = "zuerich/pflanzen/Temperatur/out";
const char* topic4 = "zuerich/pflanzen/moisture/out"; 

WiFiClient espClient;
PubSubClient client(espClient);
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
  display.setTextColor(SSD1306_WHITE);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(MOISTURE_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
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
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Received message on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (strcmp(topic, topic4) == 0) {
    Serial.println("Received command on topic4 (moisture/out).");
  
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(device_id)) {
      Serial.println("Connected to MQTT broker!");
      client.subscribe(topic1);
      client.subscribe(topic2);
      client.subscribe(topic3);
      client.subscribe(topic4); 
    } else {
      delay(500);
      Serial.print(".");
    }
  }
}

void controlRGB(bool isDry) {
  if (isDry) {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  } else {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int moisture_reading = analogRead(MOISTURE_PIN);
  bool isDry = moisture_reading > THRESHOLD;
  controlRGB(isDry);


  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Soil Sensor Status");
  display.print("Moisture: ");
  display.println(isDry ? "DRY" : "WET");
  display.print("Moisture Val: ");
  display.println(moisture_reading);
  display.println("----------");
  display.display();


  char moisture_str[10];
  itoa(moisture_reading, moisture_str, 10);
  client.publish(topic2, moisture_str);
  client.publish(topic4, moisture_str); 

  delay(10000);
}
