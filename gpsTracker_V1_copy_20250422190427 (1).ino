#include <WiFi.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <DHT.h>

// Replace with your network credentials
#define WIFI_SSID "Danger"
#define WIFI_PASSWORD "EverGreen@8991"
// #define API_KEY "AIzaSyChlmUVHMpFiCwi0zC5uj8FsKiWCw0-o6s"
#define API_KEY "AIzaSyDUQSE_sgxdKTPVPcIFIW7bXXGvI5NimA0"
// #define DATABASE_URL "https://pet-tracking-system-f1cae-default-rtdb.firebaseio.com/"
#define DATABASE_URL "https://pettrackingsystem-8e0de-default-rtdb.firebaseio.com/"

// DHT settings
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// GPS setup
HardwareSerial gpsSerial(1);  // RX, TX pins
TinyGPSPlus gps;

// Firebase setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;  // send every 5 seconds

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // Adjust RX, TX pins for GPS

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }


  Serial.println("\nConnected.");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  dht.begin();

  Serial.println("Firebase initialized.");
}

void loop() {

  // Serial.print("Temperature: ");
  // Serial.println(temp);
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (millis() - lastSendTime > sendInterval && gps.location.isValid()) {
    lastSendTime = millis();

    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    float temp = dht.readTemperature();

    // float temp = getTemperature();  // Simulated for now

    Serial.printf("Lat: %.6f, Lng: %.6f, Temp: %.2f\n", latitude, longitude, temp);

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.setString(&fbdo, "/tracking_history/1234/latitude", latitude)) {
        Serial.println();
        Serial.print(latitude);
        Serial.print(" - successfully saved to: " + fbdo.dataPath());
        Serial.println(" (" + fbdo.dataType() + ")");
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/tracking_history/1234/longitude", longitude)) {
        Serial.println();
        Serial.print(longitude);
        Serial.print(" - successfully saved to: " + fbdo.dataPath());
        Serial.println(" (" + fbdo.dataType() + ")");
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "/tracking_history/1234/temperature", temp)) {
        Serial.println();
        Serial.print(temp);
        Serial.print(" - successfully saved to: " + fbdo.dataPath());
        Serial.println(" (" + fbdo.dataType() + ")");
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }
    }
  }
}


// Simulated temperature reading
float getTemperature() {
  // Replace this with your actual temperature sensor reading
  return 25.0 + random(-100, 100) / 100.0;
}
