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
int longNotification = 0;
int shortNotification = 1;

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

// ประกาศตัวแปร global สำหรับเก็บค่า client_name และ timeStr
char client_name[50];
char timeStr[9];

void clientConfig(const char* client) {
    // ทำสิ่งที่ต้องการกับชื่อ client ที่ได้รับ เช่น ส่งผ่าน Serial
    Serial.print("******************Selected client: ");
    Serial.println(client);
}

// ฟังก์ชันสำหรับค้นหาและแสดงข้อมูล CT1, CT2, CT3, CT4 ของ client ที่เลือก
void displayClientCT(const char* selectedClientName) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String getserver = "https://senior-app.azurewebsites.net/api/client";
        http.begin(getserver);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            String payload = http.getString();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            JsonArray rows = doc["result"]["rows"];
            // Loop through each client to find the selected client
            for (int i = 0; i < rows.size(); i++) {
                JsonObject row = rows[i];
                const char* clientName = row["client_name"];
                if (strcmp(clientName, selectedClientName) == 0) {
                    // Found the selected client, now get CT1, CT2, CT3, CT4 values
                    const char* CT1 = row["CT1"];
                    const char* CT2 = row["CT2"];
                    const char* CT3 = row["CT3"];
                    const char* CT4 = row["CT4"];

                    const char* T1 = row["T1"];
                    const char* T2 = row["T2"];
                    const char* T3 = row["T3"];
                    const char* T4 = row["T4"];

                     // Check if T1 is null
                    if (T1 == NULL) {
                        Serial.println("check1");
                        lv_obj_add_state(ui_Checkbox1A, LV_STATE_CHECKED);
                        lv_obj_add_state(ui_switch1_select1, LV_STATE_CHECKED);  
                    } else {
                        // Display T1 value
                        Serial.print("T1: ");
                        Serial.println(T1);
                    }

                    // Check if T2 is null
                    if (T2 == NULL) {
                        Serial.println("check2");
                        lv_obj_add_state(ui_Checkbox2A, LV_STATE_CHECKED);
                        lv_obj_add_state(ui_switch2_select1, LV_STATE_CHECKED);  
                    } else {
                        // Display T2 value
                        Serial.print("T2: ");
                        Serial.println(T2);
                    }

                    // Check if T3 is null
                    if (T3 == NULL) {
                        Serial.println("check3");
                        lv_obj_add_state(ui_Checkbox3A, LV_STATE_CHECKED); 
                        lv_obj_add_state(ui_switch3_select1, LV_STATE_CHECKED); 
                    } else {
                        // Display T3 value
                        Serial.print("T3: ");
                        Serial.println(T3);
                    }

                    // Check if T4 is null
                    if (T4 == NULL) {
                        Serial.println("check4");
                        lv_obj_add_state(ui_Checkbox4A, LV_STATE_CHECKED);
                        lv_obj_add_state(ui_switch4_select1, LV_STATE_CHECKED);  
                    } else {
                        // Display T4 value
                        Serial.print("T4: ");
                        Serial.println(T4);
                    }

                    // Display CT1, CT2, CT3, CT4 values
                    Serial.print("CT1: ");
                    Serial.println(CT1);
                    Serial.print("CT2: ");
                    Serial.println(CT2);
                    Serial.print("CT3: ");
                    Serial.println(CT3);
                    Serial.print("CT4: ");
                    Serial.println(CT4);

                   
                    // Display CT1, CT2, CT3, CT4 values on LVGL labels
                    lv_label_set_text(ui_time__in1A, CT1);
                    lv_label_set_text(ui_time__in2A, CT2);
                    lv_label_set_text(ui_time__in3A, CT3);
                    lv_label_set_text(ui_time__in4A, CT4);
                    lv_label_set_text(ui_A_Label, clientName);
                    lv_label_set_text(ui_clientalarm, clientName);


                    // Exit loop once found the selected client
                    break;

                }
            }
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

// Function to extract hour and minute from time_t structure
void extractHourMinute(time_t time, int& hour, int& minute) {
    struct tm *timeinfo;
    timeinfo = localtime(&time);
    hour = timeinfo->tm_hour;
    minute = timeinfo->tm_min;
}

// Function to parse time string (HH:MM) to total minutes
// Function to parse time in HH:MM format to minutes
int parseTimeToMinutes(const char* timeStr) {
    int hours, minutes;
    sscanf(timeStr, "%d:%d", &hours, &minutes);
    return hours * 60 + minutes;
}

// Function to display client CT in HH:MM format
void displayClientCT1(const char* selectedClientName, NTPClient& timeClient) {
    // ตรวจสอบสถานะ WiFi ก่อนที่จะดึงข้อมูล
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected");
        return;
    }

    // เริ่ม HTTP ร้องขอ
    HTTPClient http;
    String getserver = "https://senior-app.azurewebsites.net/api/client";
    http.begin(getserver);

    // ทำการ GET ข้อมูล
    int httpResponseCode = http.GET();
    if (httpResponseCode <= 0) {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return;
    }

    // ดึงข้อมูล JSON และตรวจสอบ
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        http.end();
        return;
    }

    // หาข้อมูลของ client ที่เลือก
    JsonArray rows = doc["result"]["rows"];
    for (int i = 0; i < rows.size(); i++) {
        JsonObject row = rows[i];
        const char* clientName = row["client_name"];

        // ถ้าพบ client ที่เลือก
        if (strcmp(clientName, selectedClientName) == 0) {
            const char* CT1 = row["CT1"];
            const char* CT2 = row["CT2"];
            const char* CT3 = row["CT3"];
            const char* CT4 = row["CT4"];

            // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามนั้น
            int currentTotalMinutes = timeClient.getHours() * 60 + timeClient.getMinutes();
              
            // แปลง CT1, CT2, CT3, CT4 เป็นนาที
            int ct1TotalMinutes = parseTimeToMinutes(CT1);
            int ct2TotalMinutes = parseTimeToMinutes(CT2);
            int ct3TotalMinutes = parseTimeToMinutes(CT3);
            int ct4TotalMinutes = parseTimeToMinutes(CT4);

            // แปลงเวลาจากนาทีกลับเป็น HH:MM
            char ct1Formatted[6], ct2Formatted[6], ct3Formatted[6], ct4Formatted[6];
            snprintf(ct1Formatted, sizeof(ct1Formatted), "%02d:%02d", ct1TotalMinutes / 60, ct1TotalMinutes % 60);
            snprintf(ct2Formatted, sizeof(ct2Formatted), "%02d:%02d", ct2TotalMinutes / 60, ct2TotalMinutes % 60);
            snprintf(ct3Formatted, sizeof(ct3Formatted), "%02d:%02d", ct3TotalMinutes / 60, ct3TotalMinutes % 60);
            snprintf(ct4Formatted, sizeof(ct4Formatted), "%02d:%02d", ct4TotalMinutes / 60, ct4TotalMinutes % 60);

            // แสดงผล CT1, CT2, CT3, CT4
            Serial.println("CT1: " + String(ct1Formatted));
            Serial.println("CT2: " + String(ct2Formatted));
            Serial.println("CT3: " + String(ct3Formatted));
            Serial.println("CT4: " + String(ct4Formatted));

             // เปรียบเทียบเวลาปัจจุบันกับค่า CT และแสดงผลตามเงื่อนไข
           if (currentTotalMinutes > ct1TotalMinutes && currentTotalMinutes < ct2TotalMinutes) {
            Serial.println("*********CT2: " + String(ct2Formatted));
            lv_label_set_text(ui_time__alarm, ct2Formatted);
            
} else if (currentTotalMinutes >= ct2TotalMinutes && currentTotalMinutes < ct3TotalMinutes) {
            Serial.println("**********CT3: " + String(ct3Formatted));
            lv_label_set_text(ui_time__alarm, ct3Formatted);
} else if (currentTotalMinutes >= ct3TotalMinutes && currentTotalMinutes < ct4TotalMinutes) {
            Serial.println("*********CT4: " + String(ct4Formatted));
            lv_label_set_text(ui_time__alarm, ct4Formatted);
} else {
            Serial.println("***********CT1: " + String(ct1Formatted));
            lv_label_set_text(ui_time__alarm, ct1Formatted);
}



            // อัพเดท LVGL labels
            lv_label_set_text(ui_time__in1A, ct1Formatted);
            lv_label_set_text(ui_time__in2A, ct2Formatted);
            lv_label_set_text(ui_time__in3A, ct3Formatted);
            lv_label_set_text(ui_time__in4A, ct4Formatted);
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
void Sentclienttime() {
    // Your Domain name with URL path or IP address with path
    String serverName = "https://senior-app.azurewebsites.net/api/client/eating";
  
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
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

        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }

        // Free resources
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

static void client_click_handle(lv_event_t * e) {
    // ดึงชื่อ client ที่เลือก
    lv_dropdown_get_selected_str(ui_P_name, client_name, sizeof(client_name));
    
    // เรียกใช้งานฟังก์ชัน clientConfig() เพื่อประมวลผล
    clientConfig(client_name);
    
    // แสดงข้อมูล CT1, CT2, CT3, CT4 ของ client ที่เลือก
    //displayClientCT(client_name);
    displayClientCT1(client_name, timeClient);
    
}

static void accept_click_handle(lv_event_t * e) {
    // Get current time
    int hourNow = timeClient.getHours();
    int minuteNow = timeClient.getMinutes();

    // Format the time string
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hourNow, minuteNow);

    // Send time over Serial
    Serial.print("****************Current time (HH:MM): ");
    Serial.println(timeStr);
    
    // ส่งข้อมูล client_name และ timeStr ไปยังฟังก์ชัน Sentclienttime()
    Sentclienttime();
}


// ฟังก์ชันสำหรับดึงข้อมูลจากเซิร์ฟเวอร์และแสดงข้อมูล client ใน dropdown
void httpsGet2() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String getserver = "https://senior-app.azurewebsites.net/api/client";
        http.begin(getserver);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();

            Serial.println(payload);
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            JsonArray rows = doc["result"]["rows"];
            
            // Loop through each client and add their name to the dropdown
            for (int i = 0; i < rows.size(); i++) {
                JsonObject row = rows[i];
                const char *client_name = row["client_name"];
                
                // Add client name to the dropdown
                lv_dropdown_add_option(ui_P_name, client_name, i);
            }
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
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

void SentTempandHumi()
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

    // Send data to server
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
      lv_label_set_text(ui_A_total_lebel, md_set_A);
      lv_label_set_text(ui_A_alarm, md_set_A);

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
  lv_obj_add_event_cb(ui_P_refresh, client_click_handle, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Button1, accept_click_handle, LV_EVENT_CLICKED, NULL);
  
  
}

void loop()
{
  updateWiFiIcon();
  timenow();
  timeClient.update();



  unsigned long currentMillis = millis(); // เวลาปัจจุบัน

  if (currentMillis - lastGetTime >= 10000)
  {             
     // เรียกใช้งานฟังก์ชันสร้างเสียงเตือน

               // ทุก 1 ชั่วโมง (3600000 มิลลิวินาที)
    httpsGet2();                  // เรียกใช้ฟังก์ชัน httpsGet()
    //Getmedtotal();
    Sentclienttime();
    lastGetTime = currentMillis; // ปรับปรุงเวลาของการรับข้อมูลล่าสุด
  }
  // Display.loop();

  
  

  Display.loop(); // Keep GUI work
}