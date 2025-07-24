#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "" //replace with your SSID
#define WIFI_PASSWORD ""//replace with your password
#define API_KEY ""//replace with api key from firebase
#define DATABASE_URL ""//replace with your database URL

// Define sensor pins
#define TRIG_PIN 5          // HC-SR04 Trigger pin
#define ECHO_PIN 18         // HC-SR04 Echo pin
#define PH_PIN 34           // Analog pH sensor pin
#define TDS_PIN 35          // Analog TDS sensor pin

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
long duration, distance;
float pHValue = 0.0;
int tdsValue = 0;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signup OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize HC-SR04 pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Read HC-SR04 distance
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = (duration / 2) / 29.1;

    // Read pH sensor value
    pHValue = analogRead(PH_PIN) * (5.0 / 1023.0);  // Assuming 5V reference for analog pin

    // Read TDS sensor value
    tdsValue = analogRead(TDS_PIN); 

    // Upload data to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "sensor/distance", distance)) {
      Serial.print("Distance: ");
      Serial.println(distance);
      Serial.print("Successfully saved to: ");
      Serial.println(fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")");
    } else {
      Serial.println("Distance upload failed: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "sensor/ph_value", pHValue)) {
      Serial.print("pH Value: ");
      Serial.println(pHValue);
      Serial.print("Successfully saved to: ");
      Serial.println(fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")");
    } else {
      Serial.println("pH Value upload failed: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "sensor/tds_value", tdsValue)) {
      Serial.print("TDS Value: ");
      Serial.println(tdsValue);
      Serial.print("Successfully saved to: ");
      Serial.println(fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")");
    } else {
      Serial.println("TDS Value upload failed: " + fbdo.errorReason());
    }
  }
}
