#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "A2EMS_TP-LINK"
#define WIFI_PASSWORD "1981Azra1981Efe"
#define API_KEY "AIzaSyDpLn4Tdt0TnxCNsnXKp3Y0ePLNnz1nYuM"
#define DATABASE_URL "triteam-proje-default-rtdb.firebaseio.com"
#define MOTOR_PIN1 D1  // Connect to one of the input pins of the L298N
#define MOTOR_PIN2 D0  // Connect to the other input pin of the L298N
String SULEVEL; // Change SULEVEL to String
int SUYUZDE;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
const int sensorPin = A0;
int sensorLevel = 0; // Initialize sensorLevel
const int threshold = 500;

void tokenStatusCallback(bool tokenStatus) {
  Serial.println(tokenStatus ? "Token is ready" : "Token has expired");
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 750 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (sensorLevel >= 100) {
      SULEVEL = "Su var";
      if (Firebase.RTDB.setString(&fbdo, "/SULEVEL", SULEVEL)) {
      }
    } else if (sensorLevel <= 20) {
      SULEVEL = "Su yok";
      if (Firebase.RTDB.setString(&fbdo, "/SULEVEL", SULEVEL)) {
      }
    }
      SUYUZDE = sensorLevel;
      if (Firebase.RTDB.setString(&fbdo, "/SUYUZDE", SUYUZDE)) {}

    if (Firebase.RTDB.getInt(&fbdo, "/SULAMA")) {
      if (fbdo.dataType() == "int") {
        int pumpStatus = fbdo.intData();
        int sensorValue = analogRead(sensorPin);
        sensorLevel = map(sensorValue, 4095, 1613, 0, 100);
        Serial.println(sensorValue);
        Serial.println(sensorLevel);

        // Control the water pump based on the received data
        if (pumpStatus == 1) {
          // Turn on the water pump
          digitalWrite(MOTOR_PIN1, HIGH);
          digitalWrite(MOTOR_PIN2, LOW);
        } else {
          // Turn off the water pump
          digitalWrite(MOTOR_PIN1, LOW);
          digitalWrite(MOTOR_PIN2, LOW);
        }
      }
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}
