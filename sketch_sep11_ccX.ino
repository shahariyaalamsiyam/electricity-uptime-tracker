#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>



// ================= WIFI SETTINGS =================

const char* ssid     = "XOME";
const char* password = "exe.siyam";


// ================= GOOGLE SCRIPT URL =================

const String serverName = "https://script.google.com/macros/s/AKfycbwIxF9bjhdbdPp9z37D-LfEbDm0zQtptrgN4rOU_fgQVpAZTs7Nn_XeKz-yzEVSy8O9xA/exec";


// ================= TIMING CONFIGURATION =================

// Send heartbeat update every 10 seconds so the Google Sheet always has the latest OFF time & Running time
const unsigned long SEND_INTERVAL = 10000; 

unsigned long startTime = 0;
unsigned long lastSend  = 0;


// ================= HELPER: FORMAT RUNTIME =================
// Formats milliseconds into clean string: "1h_15m_30s", "2m_10s", or "10s"
String formatRuntime(unsigned long ms)
{
  unsigned long totalSeconds = ms / 1000;
  unsigned long hours        = totalSeconds / 3600;
  unsigned long minutes      = (totalSeconds % 3600) / 60;
  unsigned long seconds      = totalSeconds % 60;

  String result = "";
  if (hours > 0)
  {
    result += String(hours) + "h_";
  }
  if (minutes > 0 || hours > 0)
  {
    result += String(minutes) + "m_";
  }
  result += String(seconds) + "s";
  return result;
}


// ================= SEND DATA TO GOOGLE SHEET =================

void sendToSheet(String status, String runtime)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String url = serverName;
    url += "?status=" + status;
    url += "&runtime=" + runtime;

    Serial.print("Sending [Status: ");
    Serial.print(status);
    Serial.print(" | Runtime: ");
    Serial.print(runtime);
    Serial.println("]...");

    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();

    if (httpCode > 0)
    {
      Serial.print("Sheet Response: ");
      Serial.println(httpCode);
    }
    else
    {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi not connected. Cannot send data.");
  }
}


// ================= SETUP =================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("   ESP32 RUNTIME TRACKER START   ");
  Serial.println("=================================");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("WiFi Connected Successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Record session start time
    startTime = millis();
    lastSend  = millis();

    // Log the initial "ON" status to Google Sheet
    sendToSheet("ON", "0s");
  }
  else
  {
    Serial.println();
    Serial.println("WiFi Connection Failed!");
  }
}


// ================= MAIN LOOP =================

void loop()
{
  // Auto-reconnect if WiFi disconnects
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected! Attempting reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
    return;
  }

  unsigned long currentTime = millis();

  // Send heartbeat update every 10 seconds
  if (currentTime - lastSend >= SEND_INTERVAL)
  {
    lastSend = currentTime;

    String runtime = formatRuntime(currentTime - startTime);

    sendToSheet("RUNNING", runtime);
  }
}