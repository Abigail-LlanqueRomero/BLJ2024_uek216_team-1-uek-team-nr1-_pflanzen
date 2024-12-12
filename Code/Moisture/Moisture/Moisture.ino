#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_SDA 21
#define OLED_SCL 22

#define MOISTURE_PIN 34 
#define RED_PIN 5      
#define GREEN_PIN 18   
#define BLUE_PIN 19    

const char* device_id = "abigail";
const char* ssid = "GuestWLANPortal";
const char* mqtt_server = "10.10.2.127";
const char* topic1 = "zuerich/pflanzen/moisture/in";
const char* topic2 = "zuerich/pflanzen/moisture/out"; 

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

  if (strcmp(topic, topic2) == 0) {
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
    } else {
      delay(500);
      Serial.print(".");
    }
  }
}

void controlRGB(int moisturePercentage) {
  if (moisturePercentage >= 66) {
   
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
  } else if (moisturePercentage >= 45) {
    
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
  } else {

    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int moisture_reading = analogRead(MOISTURE_PIN);

  int moisture_percentage = map(moisture_reading, 0, 4095, 100, 0);

  controlRGB(moisture_percentage);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Soil Sensor Status");
  display.print("Moisture: ");
  display.print(moisture_percentage);
  display.println("%");
  display.println("----------");
  display.display();

 
  char moisture_str[10];
  itoa(moisture_percentage, moisture_str, 10);
  client.publish(topic1, moisture_str);
  client.publish(topic2, moisture_str); 

  delay(1000);
}
