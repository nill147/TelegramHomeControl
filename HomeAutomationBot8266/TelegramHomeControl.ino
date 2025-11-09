#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Servo.h>

// WiFi credentials
const char* ssid = "WIFI NAME"; // Write Your wifi SSID 
const char* password = "PASSWORD";// Write Your wifi SSID Password

// Telegram credentials
#define BOT_TOKEN "Telegram bot id" // Write Your New Created Telegram Bot ID
#define CHAT_ID "Telegram chat id"// Write Your Chat ID

// Pins
#define DHTPIN D4
#define DHTTYPE DHT11
#define RAIN_SENSOR_PIN D5
#define BUZZER_PIN D6
#define SERVO_PIN D7
#define LIGHT1_PIN D2
#define LIGHT2_PIN D3
#define FAN_PIN D8
#define ALERT_RED_PIN D0
#define ALERT_BLUE_PIN D1

// Creator of the code @oseem_pran_to_nill
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
Servo rainServo;

unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 1000;

bool highTempAlertSent = false;
bool lowTempAlertSent = false;
bool rainAlertSent = false;
bool fanStatus = false;
int fanSpeed = 0;

// Alert System
bool alertActive = false;
unsigned long lastAlertBlink = 0;
bool alertBlinkState = false;

// -------------------- Sound Feedback --------------------
void beepCommand() {
  tone(BUZZER_PIN, 1000, 100);
  delay(150);
  noTone(BUZZER_PIN);
}

void toneLowTemp() {
  tone(BUZZER_PIN, 500, 300); delay(350);
  tone(BUZZER_PIN, 400, 300); delay(350);
  noTone(BUZZER_PIN);
}

void toneHighTemp() {
  tone(BUZZER_PIN, 1000, 300); delay(350);
  tone(BUZZER_PIN, 1200, 300); delay(350);
  noTone(BUZZER_PIN);
}

void toneRain() {
  tone(BUZZER_PIN, 300, 150); delay(200);
  tone(BUZZER_PIN, 300, 150); delay(200);
  tone(BUZZER_PIN, 600, 300); delay(350);
  noTone(BUZZER_PIN);
}

void toneSunny() {
  tone(BUZZER_PIN, 900, 150); delay(200);
  tone(BUZZER_PIN, 1200, 200); delay(250);
  tone(BUZZER_PIN, 1500, 250); delay(300);
  noTone(BUZZER_PIN);
}

void toneMusic() {
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
  for (int i = 0; i < 8; i++) {
    tone(BUZZER_PIN, melody[i], 150);
    delay(200);
  }
  noTone(BUZZER_PIN);
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LIGHT1_PIN, OUTPUT);
  pinMode(LIGHT2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ALERT_RED_PIN, OUTPUT);
  pinMode(ALERT_BLUE_PIN, OUTPUT);

  digitalWrite(LIGHT1_PIN, LOW);
  digitalWrite(LIGHT2_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(ALERT_RED_PIN, LOW);
  digitalWrite(ALERT_BLUE_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  secured_client.setInsecure();
  Serial.println("Bot is ready!");

  rainServo.attach(SERVO_PIN);
  rainServo.write(0);
}

// -------------------- Reconnect WiFi --------------------
void reconnectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnecting to WiFi");
    WiFi.begin(ssid, password);
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 20) {
      delay(500);
      Serial.print(".");
      count++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" Reconnected!");
    } else {
      Serial.println(" Failed to reconnect!");
    }
  }
}

// -------------------- Fan --------------------
void controlFan() {
  if (fanStatus) {
    if (fanSpeed == 1) analogWrite(FAN_PIN, 85);
    else if (fanSpeed == 2) analogWrite(FAN_PIN, 170);
    else if (fanSpeed == 3) analogWrite(FAN_PIN, 255);
  } else {
    digitalWrite(FAN_PIN, LOW);
  }
}

// -------------------- Alert --------------------
void handleAlertBlink() {
  if (!alertActive) {
    digitalWrite(ALERT_RED_PIN, LOW);
    digitalWrite(ALERT_BLUE_PIN, LOW);
    noTone(BUZZER_PIN);
    return;
  }

  if (millis() - lastAlertBlink > 200) {
    alertBlinkState = !alertBlinkState;
    digitalWrite(ALERT_RED_PIN, alertBlinkState);
    digitalWrite(ALERT_BLUE_PIN, !alertBlinkState);
    lastAlertBlink = millis();
  }

  tone(BUZZER_PIN, 1000); // continuous tone
}

// -------------------- Keyboard --------------------
String generateKeyboard() {
  String keyboard = "[[\"Temperature\", \"Humidity\"], [\"Status\", \"Weather\"], ";

  if (fanStatus) {
    keyboard += "[\"Fan Off\"";
    if (fanSpeed != 1) keyboard += ", \"Fan Low\"";
    if (fanSpeed != 2) keyboard += ", \"Fan Medium\"";
    if (fanSpeed != 3) keyboard += ", \"Fan High\"";
    keyboard += "], ";
  } else {
    keyboard += "[\"Fan On\", \"Fan Low\", \"Fan Medium\", \"Fan High\"], ";
  }

  keyboard += "[\"" + String(digitalRead(LIGHT1_PIN) ? "Light 1 Off" : "Light 1") + "\", ";
  keyboard += "\"" + String(digitalRead(LIGHT2_PIN) ? "Light 2 Off" : "Light 2") + "\"], ";

  int hoodPosition = rainServo.read();
  if (hoodPosition < 45) keyboard += "[\"Open Hood\"]";
  else keyboard += "[\"Close Hood\"]";

  keyboard += ", [\"Alert On\", \"Alert Off\"], [\"Music\"]]";
  return keyboard;
}

// -------------------- Loop --------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }

  handleAlertBlink();

  if (millis() - lastCheckTime > checkInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String text = bot.messages[i].text;
        String chat_id = bot.messages[i].chat_id;

        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float f = dht.readTemperature(true);
        int rainState = digitalRead(RAIN_SENSOR_PIN);

        String humid = "💧 Humidity: " + String(h) + " %";
        String temp  = "🌡️ Temperature: " + String(t) + "°C / " + String(f) + "°F";

        if (text == "/start" || text == "Help" || text == "Command") {
          bot.sendMessageWithReplyKeyboard(chat_id, "👋 Here’s the updated control panel:", "", generateKeyboard(), true);
        } else if (text == "Humidity") bot.sendMessage(chat_id, humid, "");
        else if (text == "Temperature") bot.sendMessage(chat_id, temp, "");
        else if (text == "Status") {
          String weatherMsg = (rainState == LOW) ? "🌧️ It's currently raining." : "🌤️ It's currently dry.";
          bot.sendMessage(chat_id, humid + "\n" + temp + "\n" + weatherMsg, "");
        } else if (text == "Weather") {
          String weatherMsg = (rainState == LOW) ? "🌧️ It's currently raining." : "🌤️ It's currently dry.";
          bot.sendMessage(chat_id, weatherMsg, "");
        } else if (text == "Music") {
          bot.sendMessage(chat_id, "🎵 Playing a melody for you!", "");
          toneMusic();
        } else if (text == "Open Hood") {
          bot.sendMessage(chat_id, "🌧️ Opening hood...", "");
          rainServo.write(90);
        } else if (text == "Close Hood") {
          bot.sendMessage(chat_id, "🌤️ Closing hood...", "");
          rainServo.write(0);
        }

        // Lights
        else if (text == "Light 1") {
          digitalWrite(LIGHT1_PIN, HIGH);
          bot.sendMessage(chat_id, "🔴 Red light turned ON.", "");
        } else if (text == "Light 1 Off") {
          digitalWrite(LIGHT1_PIN, LOW);
          bot.sendMessage(chat_id, "🔴 Red light turned OFF.", "");
        } else if (text == "Light 2") {
          digitalWrite(LIGHT2_PIN, HIGH);
          bot.sendMessage(chat_id, "🟡 Yellow light turned ON.", "");
        } else if (text == "Light 2 Off") {
          digitalWrite(LIGHT2_PIN, LOW);
          bot.sendMessage(chat_id, "🟡 Yellow light turned OFF.", "");
        }

        // Fan
        else if (text == "Fan On") {
          fanStatus = true;
          controlFan();
          bot.sendMessage(chat_id, "🌀 Fan turned ON with previous speed.", "");
        } else if (text == "Fan Off") {
          fanStatus = false;
          controlFan();
          bot.sendMessage(chat_id, "🌀 Fan turned OFF.", "");
        } else if (text == "Fan Low") {
          fanSpeed = 1;
          if (fanStatus) controlFan();
          bot.sendMessage(chat_id, "🌀 Fan speed set to LOW.", "");
        } else if (text == "Fan Medium") {
          fanSpeed = 2;
          if (fanStatus) controlFan();
          bot.sendMessage(chat_id, "🌀 Fan speed set to MEDIUM.", "");
        } else if (text == "Fan High") {
          fanSpeed = 3;
          if (fanStatus) controlFan();
          bot.sendMessage(chat_id, "🌀 Fan speed set to HIGH.", "");
        }

        // Alert system
        else if (text == "Alert On") {
          alertActive = true;
          bot.sendMessage(chat_id, "🚨 Alert mode ACTIVATED!", "");
        } else if (text == "Alert Off") {
          alertActive = false;
          bot.sendMessage(chat_id, "✅ Alert mode DEACTIVATED.", "");
        }

        bot.sendMessageWithReplyKeyboard(chat_id, "✅ Updated Control Panel:", "", generateKeyboard(), true);
        beepCommand();
      }

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    float tempC = dht.readTemperature();
    int rainState = digitalRead(RAIN_SENSOR_PIN);

    // Auto temperature alert
    if (tempC >= 57 && !alertActive) {
      bot.sendMessage(CHAT_ID, "🔥 Temperature has reached 57°C! Activating ALERT MODE!", "");
      alertActive = true;
    }

    if (tempC > 38 && !highTempAlertSent) {
      bot.sendMessage(CHAT_ID, "🔥 Temperature is rising! It's over 40°C!", "");
      toneHighTemp();
      highTempAlertSent = true;
      lowTempAlertSent = false;
    } else if (tempC < 22 && !lowTempAlertSent) {
      bot.sendMessage(CHAT_ID, "❄️ Temperature is dropping! It's below 20°C!", "");
      toneLowTemp();
      lowTempAlertSent = true;
      highTempAlertSent = false;
    } else if (tempC >= 22 && tempC <= 38) {
      highTempAlertSent = false;
      lowTempAlertSent = false;
    }

    if (rainState == LOW && !rainAlertSent) {
      bot.sendMessage(CHAT_ID, "🌧️ It's raining! Deploying hood...", "");
      toneRain();
      rainServo.write(90);
      rainAlertSent = true;
    } else if (rainState == HIGH && rainAlertSent) {
      bot.sendMessage(CHAT_ID, "🌤️ Rain stopped. Retracting hood...", "");
      toneSunny();
      rainServo.write(0);
      rainAlertSent = false;
    }

    lastCheckTime = millis();
  }
}
