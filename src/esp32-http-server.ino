#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <TM1637Display.h>
#include <RTClib.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

Servo servo;
int echoPin = 16;
int trigPin = 17;
int CLK = 19;
int DIO = 4;
bool near = false;
bool currentNear = false;
int ledBarPins[] = {25,26,33};
int photoPin = 35;
int current_slide_status = 0;
int slide_switch = 34;
int potential = 32;
bool servoStatus = false;
int servo_pin = 18;
int button_1 = 26;
int button_2 = 27;
int led = 23;
int pixelRingStatus = 0;
bool ledStatus = false;
const int DHT_PIN = 15; 
DHTesp dht;
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org";
int pixel_ring = 5;
int lednumber = 40;
Adafruit_NeoPixel ring = Adafruit_NeoPixel(lednumber, pixel_ring);
TM1637Display display(CLK, DIO); 

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;
int ledTimer = 0;
int servoTimer = 0;
int lightTimer = 0;
bool ledTimerStatus = false;
bool servoTimerStatus = false;
bool lightTimerStatus = false;

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    if (String(topic) == "device/led/command") {
        if (message == "ON") {
            digitalWrite(led, HIGH);
            ledStatus = true;
        } else if (message == "OFF") {
            digitalWrite(led, LOW);
            ledStatus = false;
        }
        client.publish("device/led/status", String(ledStatus).c_str());
    }

    if (String(topic) == "device/servo/command") {
        if (message == "OPEN") {
            openServo();
        } else if (message == "CLOSE") {
            closeServo();
        }
        client.publish("device/servo/status", String(servoStatus).c_str());
    }

    if (String(topic) == "device/light/command") {
        if (message == "ON") {
            pixelRingStatus = 2;
            client.publish("device/light/status", String(pixelRingStatus).c_str());
        } else if (message == "OFF") {
            pixelRingStatus = 0;
            client.publish("device/light/status", String(pixelRingStatus).c_str());
        }
        client.publish("device/light/status", String(pixelRingStatus).c_str());

    }

    if (String(topic) == "device/led/timer") {
        if (message != NULL) {
            ledTimer = message.toInt();
            ledTimerStatus = true;
        }
        
    }

    if (String(topic) == "device/servo/timer") {
        if (message != NULL) {
            servoTimer = message.toInt();
            servoTimerStatus = true;
        }
    }

    if (String(topic) == "device/light/timer") {
        if (message != NULL) {
            lightTimer = message.toInt();
            lightTimerStatus = true;
        }
    }

}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("Connected");
            client.subscribe("device/led/status");
            client.subscribe("device/servo/status");
            client.subscribe("device/light/status");
            client.subscribe("device/led/command");
            client.subscribe("device/servo/command");
            client.subscribe("device/light/command");
            client.subscribe("device/led/timer");
            client.subscribe("device/servo/timer");
            client.subscribe("device/light/timer");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    pinMode(2, OUTPUT);
    pinMode(button_1, INPUT);    
    pinMode(button_2, INPUT);    
    pinMode(led, OUTPUT);
    pinMode(slide_switch, INPUT);
    pinMode(potential, INPUT);
    pinMode(echoPin, INPUT);
    pinMode(trigPin, OUTPUT);
    for (int i : ledBarPins) pinMode(i, OUTPUT);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.setup(DHT_PIN, DHTesp::DHT22);
    lcd.init();
    lcd.backlight();
    ring.begin();
    servo.attach(servo_pin, 500, 2400);
    display.setBrightness(0x0a);
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
        
    }
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set the time
    }
}

void onPixelRing() {
    for (int i = 0; i < lednumber; i++) {
        ring.setPixelColor(i, ring.Color(255, 255, 255));
        ring.show();
    }
}

void offPixelRing() {
    for (int i = 0; i < lednumber; i++) {
        ring.setPixelColor(i, ring.Color(0, 0, 0));
        ring.show();
    }
}

int getTime() {
    DateTime now = rtc.now();
    int displaytime = (now.hour() * 100) + now.minute();
    Serial.println("Time Now:");
    Serial.println(displaytime);
    long millisecond = now.hour() * 3600000 + now.minute() * 60000 + now.second() * 1000;
    display.showNumberDec(displaytime, true);
    return millisecond;
}

void openServo() {
    servoStatus = true;
    servo.write(180);
    delay(1000);
}

void closeServo() {
    servoStatus = false;
    servo.write(90);
    delay(1000);
}

int getDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 0.034 / 2;
    return distance;
}

int currentPotValue = analogRead(potential);

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    int currentTime = getTime();

    if (ledTimerStatus) {
        if (ledTimer <= currentTime && currentTime <= ledTimer + 5000) {
            digitalWrite(led, HIGH);
            ledStatus = true;
            ledTimerStatus = false;
        }
    }

    if (servoTimerStatus) {
        if (currentTime >= servoTimer && currentTime <= servoTimer + 5000) {
            openServo();
            servoTimerStatus = false;
        }
    }
    
    if (lightTimerStatus) {
        if (currentTime >= lightTimer && currentTime <= lightTimer + 5000) {
            pixelRingStatus = 2;
            lightTimerStatus = false;
        }
    }

    if (digitalRead(slide_switch) != current_slide_status) {
      if (ledStatus == false) {
        digitalWrite(led, HIGH);
        ledStatus = true;
      } 
      else {
        digitalWrite(led, LOW);
        ledStatus = false;
      }
      current_slide_status = digitalRead(slide_switch);
    }
    if (digitalRead(button_2) == HIGH) {
        if (servoStatus == false) openServo();
        else closeServo();
    }

    int photoValue = analogRead(photoPin);
    int ledBarValue = map(photoValue, 32, 4096, 0, 3);
    for (int i = 0; i < 3; i++) {
        if (i < ledBarValue) {
            digitalWrite(ledBarPins[i], HIGH);
        } else {
            digitalWrite(ledBarPins[i], LOW);
        }
    }
    int potValue = analogRead(potential);
    int lightValue = map(potValue, 0, 4096, 0, 255);
    if (currentPotValue != potValue) {
        pixelRingStatus = 1;
        currentPotValue = potValue;
    }
    if (pixelRingStatus == 1) {
        for (int i = 0; i < lednumber; i++) {
            ring.setPixelColor(i, ring.Color(lightValue, lightValue, lightValue));
            ring.show();
        }

    }
    else if (pixelRingStatus == 2) {
        onPixelRing();
    }
    else if (pixelRingStatus == 0) {
        offPixelRing();
    }
    
    if (getDistance() <= 50) near = true;
    else near = false;
    if (near != currentNear) {
        onPixelRing();
        pixelRingStatus = 2;
        currentNear = near;
    }
    
    unsigned long now = millis();
    if (now - lastMsg > 1000) {
        lastMsg = now;
        TempAndHumidity data = dht.getTempAndHumidity();

        String temp = String(data.temperature, 2);
        client.publish("sensor/dht22/temp", temp.c_str());
        String hum = String(data.humidity, 1);
        client.publish("sensor/dht22/hum", hum.c_str());

        int phValue = map(photoValue, 32, 4096, 0, 14);
        client.publish("sensor/photo/ph", String(phValue).c_str());
        client.publish("device/light/status", String(pixelRingStatus).c_str());
        client.publish("device/led/status", String(ledStatus).c_str());
        client.publish("device/servo/status", String(servoStatus).c_str());

        lcd.setCursor(0, 0);
        lcd.print("Temperature: ");
        lcd.print(temp);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(hum);
        lcd.print("%");
        lcd.setCursor(0, 2);
        lcd.print("pH Value: ");
        if (phValue < 10) lcd.print("0");
        lcd.print(phValue);
    }
}
