#include <Wire.h>
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Servo.h>
#include <SparkFunLSM6DSO.h>
#include <Adafruit_AHTX0.h>
#include "Adafruit_CAP1188.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const int ldrPin = 36;
const int servoPin = 17;
const int buzzerPin = 33;

char ssid[50];
char pass[50];

const int kNetworkTimeout = 30 * 1000;
const int kNetworkDelay = 1000;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

LSM6DSO myIMU;
Adafruit_AHTX0 aht;
Servo myservo;
Adafruit_CAP1188 touchSensor;
TFT_eSPI tft;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

/*void setupWiFiCredentials() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    return;
  }
  Serial.println("Done");

  Serial.printf("Updating ssid/pass in NVS ... ");
  const char ssid[] = "Coincide";
  const char pass[] = "Webaholic";
  err = nvs_set_str(my_handle, "ssid", ssid);
  err |= nvs_set_str(my_handle, "pass", pass);
  Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

  Serial.printf("Committing updates in NVS ... ");
  err = nvs_commit(my_handle);
  Serial.printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

  nvs_close(my_handle);
}*/

void nvs_access() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err == ESP_OK) {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");

    size_t ssid_len = 0;
    err = nvs_get_str(my_handle, "ssid", NULL, &ssid_len);
    if (err == ESP_OK && ssid_len > 0) {
      nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    } else {
      Serial.printf("Failed to get SSID length: (%s)\n", esp_err_to_name(err));
    }

    size_t pass_len = 0;
    err = nvs_get_str(my_handle, "pass", NULL, &pass_len);
    if (err == ESP_OK && pass_len > 0) {
      nvs_get_str(my_handle, "pass", pass, &pass_len);
    } else {
      Serial.printf("Failed to get password length: (%s)\n", esp_err_to_name(err));
    }

    if (err == ESP_OK) {
      Serial.println("Credentials retrieved from NVS.");
    } else {
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }

    nvs_close(my_handle);
  } else {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
}

void initializeDevices() {
  myservo.attach(servoPin);
  pinMode(buzzerPin, OUTPUT);
  analogReadResolution(12);
  pinMode(ldrPin, INPUT);

  if (!aht.begin() || !myIMU.begin() || !touchSensor.begin()) {
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 50);
    tft.print("Initialization Error!");
    while (1);
  }

  myIMU.initialize(BASIC_SETTINGS);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void connectToWiFi() {
  Serial.printf("Connecting to %s...\n", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendDataToServer(int statusCode, const String &info) {
  int err = 0;
  WiFiClient c;
  HttpClient http(c);

  String url = "/api/upload?statusCode=" + String(statusCode) + "&info=" + info;

  err = http.get("18.118.15.226", 5000, url.c_str());
  if (err == 0) {
    Serial.println("Started HTTP GET request successfully.");
    int responseCode = http.responseStatusCode();
    if (responseCode >= 0) {
      Serial.printf("Server response status code: %d\n", responseCode);
    } else {
      Serial.printf("Error in getting response: %d\n", responseCode);
    }
  } else {
    Serial.printf("Connection failed: %d\n", err);
  }

  http.stop();
}

void initializeBLE() {
  BLEDevice::init("ESP32_Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("BLE Initialized and Advertising...");
}

void sendBLEData(const String &data) {
  if (deviceConnected) {
    pCharacteristic->setValue(data.c_str());
    pCharacteristic->notify();
    Serial.println("Sent via BLE: " + data);
  }
}

void updateDisplay(int touchCount, int ldrValue, float temperature, float humidity, float x, float y, float z) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 5); tft.printf("Fan lvl: %s", touchCount > 0 ? String(touchCount).c_str() : "off");
  tft.setCursor(10, 20); tft.printf("LDR: %d", ldrValue);
  tft.setCursor(10, 35); tft.printf("Temp: %.1f C", temperature);
  tft.setCursor(10, 50); tft.printf("Hum: %.1f %%", humidity);
  tft.setCursor(10, 65); tft.printf("X: %.2f", x);
  tft.setCursor(10, 80); tft.printf("Y: %.2f", y);
  tft.setCursor(10, 95); tft.printf("Z: %.2f", z);
}

int getTouchCount(uint8_t touched) {
  int touchCount = 0;
  for (int i = 0; i < 8; i++) {
    if (touched & (1 << i)) {
      touchCount++;
    }
  }
  return touchCount;
}

void controlServo(int touchCount) {
  myservo.write(touchCount > 0 ? map(touchCount, 1, 8, 0, 180) : 90);
}

bool checkAlerts(float temperature, float humidity, float x, float y, float z, int ldrValue, String &alertDetails) {
  if (temperature < 5) return (alertDetails = "Low temperature!", true);
  if (humidity < 5) return (alertDetails = "Low humidity!", true);
  if (x < -0.3 || x > 0.3) return (alertDetails = "Abnormal X-axis motion!", true);
  if (y < -0.3 || y > 0.3) return (alertDetails = "Abnormal Y-axis motion!", true);
  if (z < 0.8) return (alertDetails = "Abnormal Z-axis motion!", true);
  if (ldrValue < 150) return (alertDetails = "Low light level!", true);
  return false;
}

void setup() {
  Serial.begin(9600);
  initializeDevices();
  initializeBLE();
  //setupWiFiCredentials();
  nvs_access();
  connectToWiFi();
}

void loop() {
  float temperature, humidity, x, y, z;
  uint8_t touched = touchSensor.touched();
  int touchCount = getTouchCount(touched);
  int ldrValue = analogRead(ldrPin);

  sensors_event_t temp_event, humidity_event;
  aht.getEvent(&humidity_event, &temp_event);
  temperature = temp_event.temperature;
  humidity = humidity_event.relative_humidity;
  x = myIMU.readFloatAccelX();
  y = myIMU.readFloatAccelY();
  z = myIMU.readFloatAccelZ();

  updateDisplay(touchCount, ldrValue, temperature, humidity, x, y, z);
  controlServo(touchCount);

  String realTimeData = String("Fan-lvl:") + touchCount + ",LDR:" + ldrValue +
                        ",Temp:" + temperature + ",Hum:" + humidity +
                        ",X:" + x + ",Y:" + y + ",Z:" + z;
  sendBLEData(realTimeData);
  sendDataToServer(200,realTimeData);

  String alertDetails;
  if (checkAlerts(temperature, humidity, x, y, z, ldrValue, alertDetails)) {
    sendBLEData("ALERT: " + alertDetails);
    sendDataToServer(500, alertDetails);
    tone(buzzerPin, 300, 1000);
  }

  delay(1000);
}