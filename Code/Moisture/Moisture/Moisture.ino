#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>

#define MOISTURE_PIN 34
#define RED_PIN 5
#define GREEN_PIN 18
#define YELLOW_PIN 19
#define BUZZER_PIN 26

const char* device_id = "abigail";
const char* ssid = "GuestWLANPortal";
const char* mqtt_server = "10.10.2.127";
const char* topic1 = "zuerich/pflanzen/moisture/in";
const char* topic2 = "zuerich/pflanzen/moisture/out";

WiFiClient espClient;
PubSubClient client(espClient);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int happyMelody[] = { 523, 523, 587, 587, 523, 523, 523, 0, 523, 523, 587, 587, 523, 523, 523 };
int happyDurations[] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };

int calmMelody[] = { 440, 440, 440, 440, 0, 440, 440, 440, 440 };
int calmDurations[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8 };

int sadMelody[] = { 220, 220, 220, 220, 220 };
int sadDurations[] = { 8, 8, 8, 8, 8 };

void setup() {
  Serial.begin(115200);
  
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1) delay(1);
  }

  pinMode(MOISTURE_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  Serial.println("Setup complete");
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
    Serial.println("Received command on topic2 (moisture/out).\n");
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

void playMelody(int* melody, int* durations, int size) {
  for (int i = 0; i < size; i++) {
    int noteDuration = 1000 / durations[i];
    tone(BUZZER_PIN, melody[i], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.3;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

void controlRGB(int moisturePercentage) {
  if (moisturePercentage >= 66) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, HIGH);  
    playMelody(calmMelody, calmDurations, sizeof(calmDurations) / sizeof(int)); 
  } else if (moisturePercentage >= 45) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(YELLOW_PIN, LOW);  
    playMelody(happyMelody, happyDurations, sizeof(happyDurations) / sizeof(int)); 
  } else {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);  
    playMelody(sadMelody, sadDurations, sizeof(sadDurations) / sizeof(int)); 
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
  
  display.print("Moisture: ");
  display.print(moisture_percentage);
  display.println(" %");

  if (moisture_percentage >= 66) {
    display.println("Too Much Water");
  } else if (moisture_percentage >= 45) {
    display.println("Perfect Water Level");
  } else {
    display.println("Add Water");
  }

  display.display();

  char moisture_str[10];
  itoa(moisture_percentage, moisture_str, 10);
  client.publish(topic1, moisture_str);
  client.publish(topic2, moisture_str); 

  delay(1000);
}
