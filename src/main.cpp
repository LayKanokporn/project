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
#include <Audio.h>

Audio audio;

AHT20 aht20;

const char *ssid = "Layya";
const char *password = "Infinity";

unsigned long lastGetTime = 0;
bool showAlert = false;
unsigned long alertStartTime = 0;

bool isAlarmActive = false;

bool isPostedA = false;
bool isPostedB = false;

// motorO
int motor1Pin1 = 1;            // Blue   - 28BYJ48 pin 1
int motor1Pin2 = 2;            // Pink   - 28BYJ48 pin 2
int motor1Pin3 = 7;            // Yellow - 28BYJ48 pin 3
int motor1Pin4 = 6;            // Orange - 28BYJ48 pin 4
int motor1Speed = 4;           // Variable to set stepper speed
int digitalPin1 = 38;          // Declare variable to represent digital pin 4
int threshold1 = 20;           // Threshold value for sensor


int stacksensorA = 3;
int stacksensorB = 3;
// motorcap
int motor2Pin1 = 42;           // Blue   - 28BYJ48 pin 1
int motor2Pin2 = 41;           // Pink   - 28BYJ48 pin 2
int motor2Pin3 = 40;           // Yellow - 28BYJ48 pin 3
int motor2Pin4 = 39;           // Orange - 28BYJ48 pin 4
int motor2Speed = 4;           // Variable to set stepper speed
int digitalPin2 = 8;           // Declare variable to represent digital pin 4
int threshold2 = 20;           // Threshold value for sensor


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

// ประกาศตัวแปร global สำหรับเก็บค่า client_name และ timeStr
char client_name[50];
char timeStr[9];

const char *CT1;
const char *CT2;
const char *CT3;
const char *CT4;

const char *selectedClientName; // ประกาศตัวแปร selectedClientName เป็น global

int currentTotalMinutes; // ประกาศตัวแปร currentTotalMinutes สำหรับเก็บค่าเวลาปัจจุบัน
int ct1TotalMinutes;     // ประกาศตัวแปร ct1TotalMinutes สำหรับเก็บค่า CT1
int ct2TotalMinutes;     // ประกาศตัวแปร ct2TotalMinutes สำหรับเก็บค่า CT2
int ct3TotalMinutes;     // ประกาศตัวแปร ct3TotalMinutes สำหรับเก็บค่า CT3
int ct4TotalMinutes;     // ประกาศตัวแปร ct4TotalMinutes สำหรับเก็บค่า CT4

char ct1Formatted[6], ct2Formatted[6], ct3Formatted[6], ct4Formatted[6];

// Function to parse time string (HH:MM) to total minutes
int parseTimeToMinutes(const char *timeStr)
{
  int hours, minutes;
  sscanf(timeStr, "%d:%d", &hours, &minutes);
  return hours * 60 + minutes;
}

// Function to display client CT in HH:MM format
// ฟังก์ชันสำหรับค้นหาและแสดงข้อมูล CT1, CT2, CT3, CT4 ของ client ที่เลือก
// Function to display client CT in HH:MM format
void displayClientCT1(const char *selectedClientName, NTPClient &timeClient, int &ct1TotalMinutes, int &ct2TotalMinutes, int &ct3TotalMinutes, int &ct4TotalMinutes)
{
  // ตรวจสอบสถานะ WiFi ก่อนที่จะดึงข้อมูล
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi Disconnected");
    return;
  }

  // เริ่ม HTTP ร้องขอ
  HTTPClient http;
  String getserver = "https://senior-app.azurewebsites.net/api/client";
  http.begin(getserver);

  // ทำการ GET ข้อมูล
  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0)
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
    return;
  }

  // ดึงข้อมูล JSON และตรวจสอบ
  String payload = http.getString();
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    http.end();
    return;
  }

  // หาข้อมูลของ client ที่เลือก
  JsonArray rows = doc["result"]["rows"];
  for (int i = 0; i < rows.size(); i++)
  {
    JsonObject row = rows[i];
    const char *clientName = row["client_name"];

    // ถ้าพบ client ที่เลือก
    if (strcmp(clientName, selectedClientName) == 0)
    {
      CT1 = row["CT1"];
      CT2 = row["CT2"];
      CT3 = row["CT3"];
      CT4 = row["CT4"];

      const char *T1 = row["T1"];
      const char *T2 = row["T2"];
      const char *T3 = row["T3"];
      const char *T4 = row["T4"];

      // Check if T1 is null
      if (T1 == NULL)
      {
        Serial.println("check1");
        lv_obj_add_state(ui_Checkbox1A, LV_STATE_CHECKED);
        lv_obj_add_state(ui_switch1_select1, LV_STATE_CHECKED);
      }
      else
      {
        // Display T1 value
        Serial.print("T1: ");
        Serial.println(T1);
      }

      // Check if T2 is null
      if (T2 == NULL)
      {
        Serial.println("check2");
        lv_obj_add_state(ui_Checkbox2A, LV_STATE_CHECKED);
        lv_obj_add_state(ui_switch2_select1, LV_STATE_CHECKED);
      }
      else
      {
        // Display T2 value
        Serial.print("T2: ");
        Serial.println(T2);
      }

      // Check if T3 is null
      if (T3 == NULL)
      {
        Serial.println("check3");
        lv_obj_add_state(ui_Checkbox3A, LV_STATE_CHECKED);
        lv_obj_add_state(ui_switch3_select1, LV_STATE_CHECKED);
      }
      else
      {
        // Display T3 value
        Serial.print("T3: ");
        Serial.println(T3);
      }

      // Check if T4 is null
      if (T4 == NULL)
      {
        Serial.println("check4");
        lv_obj_add_state(ui_Checkbox4A, LV_STATE_CHECKED);
        lv_obj_add_state(ui_switch4_select1, LV_STATE_CHECKED);
      }
      else
      {

        // Display T4 value
        Serial.print("T4: ");
        Serial.println(T4);
      }
      // ทำสิ่งที่ต้องการกับชื่อ client ที่ได้รับ เช่น ส่งผ่าน Serial
      Serial.print("******************Selected client: ");
      Serial.println(selectedClientName);

      // Display CT1, CT2, CT3, CT4 values
      Serial.print("CT1: ");
      Serial.println(CT1);
      Serial.print("CT2: ");
      Serial.println(CT2);
      Serial.print("CT3: ");
      Serial.println(CT3);
      Serial.print("CT4: ");
      Serial.println(CT4);

      Serial.println("******************************************************");

      // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามนั้น
      currentTotalMinutes = timeClient.getHours() * 60 + timeClient.getMinutes();

      // แปลง CT1, CT2, CT3, CT4 เป็นนาที
      ct1TotalMinutes = parseTimeToMinutes(CT1);
      ct2TotalMinutes = parseTimeToMinutes(CT2);
      ct3TotalMinutes = parseTimeToMinutes(CT3);
      ct4TotalMinutes = parseTimeToMinutes(CT4);

      int hourNow = timeClient.getHours();
      int minuteNow = timeClient.getMinutes();
      int secondNow = timeClient.getSeconds();
      // แปลงเวลาจากนาทีกลับเป็น HH:MM

      snprintf(ct1Formatted, sizeof(ct1Formatted), "%02d:%02d", ct1TotalMinutes / 60, ct1TotalMinutes % 60);
      snprintf(ct2Formatted, sizeof(ct2Formatted), "%02d:%02d", ct2TotalMinutes / 60, ct2TotalMinutes % 60);
      snprintf(ct3Formatted, sizeof(ct3Formatted), "%02d:%02d", ct3TotalMinutes / 60, ct3TotalMinutes % 60);
      snprintf(ct4Formatted, sizeof(ct4Formatted), "%02d:%02d", ct4TotalMinutes / 60, ct4TotalMinutes % 60);

      char timeStr[7]; // "HH:MM:SS\0"
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hourNow, minuteNow);
      Serial.println("********************************timeStr: " + String(timeStr));

      // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามเงื่อนไข
      if (currentTotalMinutes > ct1TotalMinutes && currentTotalMinutes < ct2TotalMinutes)
      {
        Serial.println("*********CT2: " + String(ct2Formatted));
        lv_label_set_text(ui_time__alarm, ct2Formatted);
        lv_label_set_text(ui_Label10, ct2Formatted);
      }
      else if (currentTotalMinutes >= ct2TotalMinutes && currentTotalMinutes < ct3TotalMinutes)
      {
        Serial.println("**********CT3: " + String(ct3Formatted));
        lv_label_set_text(ui_time__alarm, ct3Formatted);
        lv_label_set_text(ui_Label10, ct3Formatted);
      }
      else if (currentTotalMinutes >= ct3TotalMinutes && currentTotalMinutes < ct4TotalMinutes)
      {
        Serial.println("*********CT4: " + String(ct4Formatted));
        lv_label_set_text(ui_time__alarm, ct4Formatted);
        lv_label_set_text(ui_Label10, ct4Formatted);
      }
      else
      {
        Serial.println("***********CT1: " + String(ct1Formatted));
        lv_label_set_text(ui_time__alarm, ct1Formatted);
        lv_label_set_text(ui_Label10, ct1Formatted);
      }

      // อัพเดท LVGL labels
      lv_label_set_text(ui_time__in1A, CT1);
      lv_label_set_text(ui_time__in2A, CT2);
      lv_label_set_text(ui_time__in3A, CT3);
      lv_label_set_text(ui_time__in4A, CT4);
      lv_label_set_text(ui_A_Label, clientName);
      lv_label_set_text(ui_clientalarm, clientName);

      http.end();
      return; // ออกจากฟังก์ชั่นเมื่อพบ client ที่เลือก
    }
  }

  Serial.println("Selected client not found");
  http.end();
}

// ฟังก์ชันสำหรับเตรียม JSON payload และส่งไปยังเซิร์ฟเวอร์
void Postclienttime()
{
  // Your Domain name with URL path or IP address with path
  String serverName = "https://senior-app.azurewebsites.net/api/client/eating";

  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String serverPath = serverName;
    http.begin(serverName, root_ca);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    StaticJsonDocument<200> jsonPayload;
    jsonPayload["client_name"] = client_name;
    jsonPayload["timeeating"] = timeStr;

    // Serialize JSON payload
    String jsonString;
    serializeJson(jsonPayload, jsonString);

    // Send data to server
    int httpResponseCode = http.POST(jsonString);

    Serial.print("******************Selected client_name: ");
    Serial.println(client_name);
    Serial.print("******************Selected timeStr: ");
    Serial.println(timeStr);

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

static void client_click_handle(lv_event_t *e)
{
  // ดึงชื่อ client ที่เลือก
  lv_dropdown_get_selected_str(ui_P_name, client_name, sizeof(client_name));

  // แสดงข้อมูล CT1, CT2, CT3, CT4 ของ client ที่เลือก
  displayClientCT1(client_name, timeClient, ct1TotalMinutes, ct2TotalMinutes, ct3TotalMinutes, ct4TotalMinutes);
}

static void accept_click_handle(lv_event_t *e)
{
  // Get current time
  int hourNow = timeClient.getHours();
  int minuteNow = timeClient.getMinutes();

  // Format the time string
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hourNow, minuteNow);

  // Send time over Serial
  Serial.print("****************Current time (HH:MM): ");
  Serial.println(timeStr);
  Sound.useLVGL();
  lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0xF2A3F5), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Button1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  stacksensorA = 0;  // Reset first sensor state
  stacksensorB = 0;  // Reset second sensor state
  isPostedA = false; // Reset flag for sensor A
  isPostedB = false; // Reset flag for sensor B

  // ส่งข้อมูล client_name และ timeStr ไปยังฟังก์ชัน Sentclienttime()
  Postclienttime();
}

// ฟังก์ชันสำหรับดึงข้อมูลจากเซิร์ฟเวอร์และแสดงข้อมูล client ใน dropdown
void Getclient()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String getserver = "https://senior-app.azurewebsites.net/api/client";
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

      // Loop through each client and add their name to the dropdown
      for (int i = 0; i < rows.size(); i++)
      {
        JsonObject row = rows[i];
        const char *client_name = row["client_name"];

        // Add client name to the dropdown
        lv_dropdown_add_option(ui_P_name, client_name, i);
      }
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

void PostTempandHumi()
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
    float temp = aht20.getTemperature();
    float humidity = aht20.getHumidity();

    // Convert temperature and humidity to strings
    char tempStr[10];
    char humiStr[10];
    dtostrf(temp, 5, 2, tempStr);
    dtostrf(humidity, 5, 2, humiStr);

    // Set temperature and humidity values on LVGL labels
    lv_label_set_text(ui_temp_Label_value, tempStr); // Set temperature value on LVGL label
    lv_label_set_text(ui_humi_Label_value, humiStr); // Set humidity value on LVGL label

    // Convert temperature and humidity to integer values (multiplying by 100)
    int intTemp = temp;
    int intHumidity = humidity;

    // Convert integers to strings
    String tempString = String(intTemp);
    String humidityString = String(intHumidity);

    // Send data to server
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

void Getmedtotal()
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
      JsonObject rowA = rows[0];
      const char *md_set_A = rowA["md_name"];
      int md_total_A = rowA["md_total"];

      Serial.print("md_set_A = ");
      Serial.println(md_set_A);
      Serial.print("md_total_A = ");
      Serial.println(md_total_A);

      char md_total_A_str[10];
      itoa(md_total_A, md_total_A_str, 10);

      // Set text on LVGL label for A
      lv_label_set_text(ui_A_total_, md_total_A_str);
      lv_label_set_text(ui_A_total_lebel, md_set_A);
      lv_label_set_text(ui_A_alarm, md_set_A);

      // Process the second row (index 1, B)
      JsonObject rowB = rows[1];
      const char *md_set_B = rowB["md_name"];
      int md_total_B = rowB["md_total"];

      Serial.print("md_set_B = ");
      Serial.println(md_set_B);
      Serial.print("md_total_B = ");
      Serial.println(md_total_B);

      char md_total_B_str[10];
      itoa(md_total_B, md_total_B_str, 10);

      // Set text on LVGL label for B
      lv_label_set_text(ui_B_total_, md_total_B_str);
      lv_label_set_text(ui_B_total_lebel, md_set_B);
      lv_label_set_text(ui_B_alarm, md_set_B);
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
void postsensorA()
{
  String serverName = "https://senior-app.azurewebsites.net/api/medicine/balance";

  // สร้าง JsonObject เพื่อเก็บข้อมูลที่จะส่งไปยังเซิร์ฟเวอร์
  StaticJsonDocument<200> doc;
  doc["md_set"] = "ช่อง A";
  doc["md_output"] = 1;

  // แปลง JsonObject เป็น string JSON
  String jsonString;
  serializeJson(doc, jsonString);

  // Check WiFi connection status
  HTTPClient http;
  http.begin(serverName, root_ca);
  http.addHeader("Content-Type", "application/json"); // เปลี่ยน Content-Type เป็น application/json
  int httpResponseCode = http.POST(jsonString);       // ส่งข้อมูล JSON ไปยังเซิร์ฟเวอร์

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
  stacksensorA = 3;
}
void postsensorB()
{
  String serverName = "https://senior-app.azurewebsites.net/api/medicine/balance";

  // สร้าง JsonObject เพื่อเก็บข้อมูลที่จะส่งไปยังเซิร์ฟเวอร์
  StaticJsonDocument<200> doc;
  doc["md_set"] = "ช่อง B";
  doc["md_output"] = 1;

  // แปลง JsonObject เป็น string JSON
  String jsonString;
  serializeJson(doc, jsonString);

  // Check WiFi connection status
  HTTPClient http;
  http.begin(serverName, root_ca);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonString);
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
  stacksensorB = 3;
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

  // Setup peripherals
  Display.begin(0); // rotation number 0
  Touch.begin();
  Sound.begin();
  Card.begin(); // uncomment if you want to Read/Write/Play/Load file in MicroSD Card

  audio.setVolume(15);
  // Map peripheral to LVGL
  Display.useLVGL(); // Map display to LVGL
  Touch.useLVGL();   // Map touch screen to LVGL
  // Sound.useLVGL();   // Map speaker to LVGL

  Card.useLVGL(); // Map MicroSD Card to LVGL File System

  //Display.enableAutoSleep(120); // Auto off display if not touch in 2 min

  // Add load your UI function
  ui_init();

  lv_obj_add_event_cb(ui_P_refresh, [](lv_event_t *e)
                      { audio.connecttoFS(Card, "/audio/sound.mp3"); }, LV_EVENT_CLICKED, NULL);

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

  lv_obj_add_event_cb(ui_P_refresh, client_click_handle, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Button1, accept_click_handle, LV_EVENT_CLICKED, NULL);
  Getclient(); // เรียกใช้ฟังก์ชัน httpsGet()
  Getmedtotal();
  PostTempandHumi();
  lv_disp_load_scr(ui_Index);
}

void loop()
{
  updateWiFiIcon();
  timenow();
  timeClient.update();
   unsigned long currentMillis = millis(); // เวลาปัจจุบัน
  lv_disp_load_scr(ui_Index);
  bool val1 = digitalRead(digitalPin1);
  bool val2 = digitalRead(digitalPin2);
  // Serial.print("val1 = ");
  // Serial.println(val1);
  // Serial.print("val2 = ");
  // Serial.println(val2);


  if (currentMillis - lastGetTime >= 10000) {
    lastGetTime = currentMillis; // ปรับปรุงเวลาของการรับข้อมูลล่าสุด

    Serial.print("CT1: ");
    Serial.println(CT1);
    Serial.print("CT2: ");
    Serial.println(CT2);
    Serial.print("CT3: ");
    Serial.println(CT3);
    Serial.print("CT4: ");
    Serial.println(CT4);

    // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามนั้น
    int currentTotalMinutes = timeClient.getHours() * 60 + timeClient.getMinutes();

    Serial.println("********************************timeStr: " + String(currentTotalMinutes));
    Serial.println("********************************ct1TotalMinutes: " + String(ct1TotalMinutes));

    int ct5TotalMinutes=841;
    int ct6TotalMinutes=843;

    // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามเงื่อนไข
    if (currentTotalMinutes == ct1TotalMinutes || 
        currentTotalMinutes == ct2TotalMinutes || 
        currentTotalMinutes == ct5TotalMinutes || 
        currentTotalMinutes == ct6TotalMinutes) {
      Serial.println("*********Compare: " + String(currentTotalMinutes));
      lv_obj_clear_flag(ui_loading, LV_OBJ_FLAG_HIDDEN); // แสดง ui_loading
      Sound.useLVGL();   // Map speaker to LVGL
      alertStartTime = millis();
      showAlert = true;
    }
  }

  if (showAlert) {
    if (millis() - alertStartTime >= 10000) { // ค้างไว้ 10 วินาที
      lv_obj_add_flag(ui_loading, LV_OBJ_FLAG_HIDDEN); // ซ่อน ui_loading
      lv_disp_load_scr(ui_Index);
      showAlert = false;
      
    }
  }



  // if (stacksensorA == 0)
  // {
  //   clockwise1();
  //   if (val1 == 1)
  //   {
  //     stacksensorA = 1;
  //     Serial.print("******************************************************************AAAAAA");
  //   }
  // }
  // if (stacksensorA == 1 && !isPostedA)
  // {
  //   postsensorA();
  //   isPostedA = true;
  //   stopMotor1();
  // }

  // if (stacksensorB == 0)
  // {
  //   clockwise2();
  //   if (val2 == 1)
  //   {
  //     stacksensorB = 1;
  //     Serial.print("******************************************************************BBBB");
  //   }
  // }
  // if (stacksensorB == 1 && !isPostedB)
  // {
  //   postsensorB();
  //   isPostedB = true;
  //   stopMotor2();
  // }

  if (currentMillis - lastGetTime >= 300000) // ทุก5 นาที
  {
    Getclient(); // เรียกใช้ฟังก์ชัน httpsGet()
    Getmedtotal();
    lastGetTime = currentMillis; // ปรับปรุงเวลาของการรับข้อมูลล่าสุด
  }
  if (currentMillis - lastGetTime >= 900000)
  {
    PostTempandHumi();
    lastGetTime = currentMillis; // ปรับปรุงเวลาของการรับข้อมูลล่าสุด
  }

  Display.loop(); // Keep GUI work
}