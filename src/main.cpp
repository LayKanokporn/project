#include <Arduino.h>
#include <lvgl.h>
#include <ATD3.5-S3.h>
#include "gui/ui.h"
#include <ArduinoJson.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AHT20.h>

AHT20 aht20;

const char *ssid = "Layya";
const char *password = "Infinity";

unsigned long lastGetTime = 0;
unsigned long lastConTime = 0;

bool lastVal1 = false;
bool lastVal2 = false;

// motorO
int motor1Pin1 = 1;            // Blue   - 28BYJ48 pin 1
int motor1Pin2 = 2;            // Pink   - 28BYJ48 pin 2
int motor1Pin3 = 7;            // Yellow - 28BYJ48 pin 3
int motor1Pin4 = 6;            // Orange - 28BYJ48 pin 4
int digitalPin1 = 13;          // Declare variable to represent digital pin 4
int motor1Speed = 6;           // Variable to set stepper speed
int threshold1 = 20;           // Threshold value for sensor
bool sensor1Triggered = false; // Flag variable to track sensor state

// motorcap
int motor2Pin1 = 42;           // Blue   - 28BYJ48 pin 1
int motor2Pin2 = 41;           // Pink   - 28BYJ48 pin 2
int motor2Pin3 = 40;           // Yellow - 28BYJ48 pin 3
int motor2Pin4 = 39;           // Orange - 28BYJ48 pin 4
int digitalPin2 = 38;          // Declare variable to represent digital pin 4
int motor2Speed = 6;           // Variable to set stepper speed
int threshold2 = 20;           // Threshold value for sensor
bool sensor2Triggered = false; // Flag variable to track sensor state

const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
    "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
    "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
    "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
    "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
    "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
    "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
    "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
    "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
    "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
    "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
    "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
    "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
    "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
    "MrY=\n"
    "-----END CERTIFICATE-----\n";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600);

void timenow()
{
  static unsigned long lastTimeUpdate = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastTimeUpdate >= 1000)
  {
    lastTimeUpdate = currentTime;

    int hourNow = timeClient.getHours();
    int minuteNow = timeClient.getMinutes();
    int secondNow = timeClient.getSeconds();

    char timeStr[9]; // "HH:MM:SS\0"
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hourNow, minuteNow, secondNow);
    lv_label_set_text(ui_time_Label, timeStr);
  }
}

void updateWiFiIcon()
{
  if (WiFi.isConnected())
  {
    lv_obj_clear_flag(ui_Wifi_icon, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    static unsigned long timer = 0;
    if ((timer == 0) || ((millis() < timer) || ((millis() - timer) > 300)))
    {
      timer = millis();
      if (!lv_obj_has_flag(ui_Wifi_icon, LV_OBJ_FLAG_HIDDEN))
      {
        lv_obj_add_flag(ui_Wifi_icon, LV_OBJ_FLAG_HIDDEN);
      }
      else
      {
        lv_obj_clear_flag(ui_Wifi_icon, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

void httpsPost()
{
  // Your Domain name with URL path or IP address with path
  String serverName = "https://senior-app.azurewebsites.net/api/temp/create";
  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String serverPath = serverName;
    http.begin(serverName, root_ca);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int temp = aht20.getTemperature();
    int humidity = aht20.getHumidity();

    String tempString = String(temp);
    String humidityString = String(humidity);
    int httpResponseCode = http.POST("temp=" + tempString + "&humidity=" + humidityString);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

void httpsGet()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String getserver = "https://senior-app.azurewebsites.net/api/medicine";
    http.begin(getserver);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();

      Serial.println(payload);
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
          
      JsonArray rows = doc["result"]["rows"];
      
      // Process the first row (index 0, A)
      JsonObject rowA = rows[1];
      const char* md_set_A = rowA["md_set"];
      int md_total_A = rowA["md_total"];

      Serial.print("md_set_A = ");
      Serial.println(md_set_A);
      Serial.print("md_total_A = ");
      Serial.println(md_total_A);

      char md_total_A_str[10];
      itoa(md_total_A, md_total_A_str, 10);

      // Set text on LVGL label for A
      lv_label_set_text(ui_A_total_, md_total_A_str);

      // Process the second row (index 1, B)
      JsonObject rowB = rows[0];
      const char* md_set_B = rowB["md_set"];
      int md_total_B = rowB["md_total"];

      Serial.print("md_set_B = ");
      Serial.println(md_set_B);
      Serial.print("md_total_B = ");
      Serial.println(md_total_B);

      char md_total_B_str[10];
      itoa(md_total_B, md_total_B_str, 10);

      // Set text on LVGL label for B
      lv_label_set_text(ui_B_total_, md_total_B_str);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

void clockwise1()
{
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor1Pin3, LOW);
  digitalWrite(motor1Pin4, HIGH);
  delay(motor1Speed);

  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor1Pin3, HIGH);
  digitalWrite(motor1Pin4, LOW);
  delay(motor1Speed);

  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor1Pin3, LOW);
  digitalWrite(motor1Pin4, LOW);
  delay(motor1Speed);

  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor1Pin3, LOW);
  digitalWrite(motor1Pin4, LOW);
  delay(motor1Speed);
}

void clockwise2()
{
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  digitalWrite(motor2Pin3, LOW);
  digitalWrite(motor2Pin4, LOW);
  delay(motor2Speed);

  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  digitalWrite(motor2Pin3, LOW);
  digitalWrite(motor2Pin4, LOW);
  delay(motor2Speed);

  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  digitalWrite(motor2Pin3, HIGH);
  digitalWrite(motor2Pin4, LOW);
  delay(motor2Speed);

  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  digitalWrite(motor2Pin3, LOW);
  digitalWrite(motor2Pin4, HIGH);
  delay(motor2Speed);
}

void sentoutputsensorA()
{

  String serverName = "https://senior-app.azurewebsites.net/api/medicine/balance";
  // Check WiFi connection status
  HTTPClient http;
  String serverPath = serverName;
  http.begin(serverName, root_ca);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("md_set=" + String("ช่อง A") + "&md_output=" + 1);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void sentoutputsensorB()
{

  String serverName = "https://senior-app.azurewebsites.net/api/medicine/balance";
  // Check WiFi connection status
  HTTPClient http;
  String serverPath = serverName;
  http.begin(serverName, root_ca);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("md_set=" + String("ช่อง B") + "&md_output=" + int(1));

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void stopMotor1()
{
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor1Pin3, LOW);
  digitalWrite(motor1Pin4, LOW);
}
void stopMotor2()
{
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  digitalWrite(motor2Pin3, LOW);
  digitalWrite(motor2Pin4, LOW);
}
void setup()
{
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor1Pin3, OUTPUT);
  pinMode(motor1Pin4, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(motor2Pin3, OUTPUT);
  pinMode(motor2Pin4, OUTPUT);

  sensor1Triggered = true;
  sensor2Triggered = true;

  // Setup peripherals
  Display.begin(0); // rotation number 0
  Touch.begin();
  Sound.begin();
  // Card.begin(); // uncomment if you want to Read/Write/Play/Load file in MicroSD Card

  // Map peripheral to LVGL
  Display.useLVGL(); // Map display to LVGL
  Touch.useLVGL();   // Map touch screen to LVGL
  Sound.useLVGL();   // Map speaker to LVGL
  // Card.useLVGL(); // Map MicroSD Card to LVGL File System

  Display.enableAutoSleep(120); // Auto off display if not touch in 2 min

  // Add load your UI function
  ui_init();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  Serial.println(F("AHT20+BMP280 test"));

  Wire.begin();
  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1)
      ;
  }
  Serial.println(F("AHT20 OK"));
}

void loop()
{
  float temperature = aht20.getTemperature(); // Get temperature from sensor
  float humidity = aht20.getHumidity();       // Get humidity from sensor

  unsigned long currentMillis = millis(); // เวลาปัจจุบัน

  // if (currentMillis - lastGetTime >= 10000)
  // {                              // ทุก 1 ชั่วโมง (3600000 มิลลิวินาที)
  //   httpsGet();                  // เรียกใช้ฟังก์ชัน httpsGet()
  //   lastGetTime = currentMillis; // ปรับปรุงเวลาของการรับข้อมูลล่าสุด
  // }
  // Display.loop();

  // if (currentMillis - lastConTime >= 10000)
  // {

  //   Serial.printf("Temperature: %.02f *C\n", temperature);
  //   Serial.printf("Humidity: %.02f %%RH\n", humidity);

  //   // Convert float values to strings
  //   char tempStr[10];
  //   char humiStr[10];
  //   dtostrf(temperature, 5, 2, tempStr);
  //   dtostrf(humidity, 5, 2, humiStr);

  //   lv_label_set_text(ui_temp_Label_value, tempStr); // Set temperature value on LVGL label
  //   lv_label_set_text(ui_humi_Label_value, humiStr); // Set humidity value on LVGL label
  //   httpsPost();                 // เรียกใช้ฟังก์ชัน httpscon()

  //   lastConTime = currentMillis; // ปรับปรุงเวลาของการเชื่อมต่อล่าสุด
  // }
  updateWiFiIcon();
  timenow();
  timeClient.update();
  Display.loop(); // Keep GUI work
}