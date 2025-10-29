#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#define LCD_BACK_LIGHT_PIN 21

// --- Touch Screen Pins (Cheap Yellow Display defaults) ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// --- Display Constants ---
#define SCREEN_W 320
#define SCREEN_H 240

#define WIFI_SSID ""
#define WIFI_PASS ""

// --- Configuration Structure ---
struct WebsiteConfig {
  const char* url;
  const char* displayName;
  unsigned long checkFrequency; // in seconds
};

// --- Website Configuration ---
#define MAX_WEBSITES 6

WebsiteConfig websites[] = {
  {"https://driftngin.com", "Driftngin", 30},
  {"https://www.google.com", "Google", 60},
  {"https://www.github.com", "GitHub", 45},
  {"https://www.cloudflare.com", "Cloudflare", 30},
  {"https://www.amazon.com", "Amazon", 120},
  {"https://www.wikipedia.org", "Wikipedia", 90}
};

int numWebsites = 6;

// --- Website Status Structure ---
struct WebsiteStatus {
  bool isUp;
  int lastResponseCode;
  unsigned long lastResponseTime;
  unsigned long lastCheckTime;
  int totalChecks;
  int successfulChecks;
};

// --- Globals ---
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

WebsiteStatus status[MAX_WEBSITES];
unsigned long uptimeStart = 0;
unsigned long lastDisplayUpdate = 0;
int scrollPos = 0;
int maxVisibleSites = 5; // How many sites can fit on screen

// Draw the monitoring dashboard
void drawDashboard() {
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Uptime Monitor", 10, 5);
  
  // Uptime
  unsigned long uptime = (millis() - uptimeStart) / 1000;
  unsigned long hours = uptime / 3600;
  unsigned long minutes = (uptime % 3600) / 60;
  String uptimeStr = String(hours) + "h " + String(minutes) + "m";
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(uptimeStr, 220, 5);
  
  // Separator line
  tft.drawLine(0, 25, SCREEN_W, 25, TFT_DARKGREY);
  
  // Website list
  int yPos = 35;
  int lineHeight = 40;
  
  for (int i = scrollPos; i < numWebsites && i < scrollPos + maxVisibleSites; i++) {
    // Status circle
    if (status[i].totalChecks > 0) {
      if (status[i].isUp) {
        tft.fillCircle(15, yPos + 10, 8, TFT_GREEN);
      } else {
        tft.fillCircle(15, yPos + 10, 8, TFT_RED);
      }
    } else {
      tft.fillCircle(15, yPos + 10, 8, TFT_DARKGREY);
    }
    
    // Website name
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(websites[i].displayName, 35, yPos);
    
    // Status details
    tft.setTextFont(1);
    if (status[i].totalChecks > 0) {
      // Response time and code
      String details = String(status[i].lastResponseTime) + "ms | " + 
                      String(status[i].lastResponseCode);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString(details, 35, yPos + 15);
      
      // Success rate
      float successRate = (status[i].successfulChecks * 100.0 / status[i].totalChecks);
      String rateStr = String(successRate, 0) + "%";
      tft.setTextColor(successRate >= 95 ? TFT_GREEN : (successRate >= 80 ? TFT_YELLOW : TFT_RED), TFT_BLACK);
      tft.drawString(rateStr, 150, yPos + 15);
      
      // Next check countdown
      unsigned long timeSinceCheck = (millis() - status[i].lastCheckTime) / 1000;
      unsigned long nextCheck = websites[i].checkFrequency - timeSinceCheck;
      if (nextCheck > websites[i].checkFrequency) nextCheck = 0; // Handle overflow
      
      String nextStr = "Next: " + String(nextCheck) + "s";
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.drawString(nextStr, 220, yPos + 15);
    } else {
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.drawString("Waiting for check...", 35, yPos + 15);
    }
    
    yPos += lineHeight;
  }
  
  // Scroll indicator if needed
  if (numWebsites > maxVisibleSites) {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextFont(1);
    String scrollInfo = "Sites " + String(scrollPos + 1) + "-" + 
                       String(min(scrollPos + maxVisibleSites, numWebsites)) + 
                       " of " + String(numWebsites);
    tft.drawString(scrollInfo, 10, 225);
  }
}

// Check a specific website
void checkWebsite(int index) {
  HTTPClient http;
  Serial.printf("Checking %s...\n", websites[index].displayName);
  
  http.begin(websites[index].url);
  http.setTimeout(10000);  // 10 second timeout
  
  unsigned long startTime = millis();
  int httpCode = http.GET();
  unsigned long responseTime = millis() - startTime;
  
  status[index].lastResponseCode = httpCode;
  status[index].lastResponseTime = responseTime;
  status[index].totalChecks++;
  status[index].lastCheckTime = millis();
  
  if (httpCode > 0) {
    Serial.printf("%s - HTTP Response: %d (%lu ms)\n", 
                  websites[index].displayName, httpCode, responseTime);
    
    // Consider 2xx and 3xx as success
    if (httpCode >= 200 && httpCode < 400) {
      status[index].isUp = true;
      status[index].successfulChecks++;
      Serial.printf("%s is UP!\n", websites[index].displayName);
    } else {
      status[index].isUp = false;
      Serial.printf("%s returned error code!\n", websites[index].displayName);
    }
  } else {
    status[index].isUp = false;
    Serial.printf("%s - Request failed: %s\n", 
                  websites[index].displayName, 
                  http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);
  
  // Display init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Backlight setup
  ledcSetup(0, 5000, 12);
  ledcAttachPin(LCD_BACK_LIGHT_PIN, 0);
  ledcWrite(0, 2048); // 50% brightness
  
  // Show connecting message
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(4);
  tft.drawString("Connecting...", 10, 100);
  
  // Wi-Fi connect
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  // Touch init
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
  
  // Initialize monitoring
  uptimeStart = millis();
  
  // Initialize all website statuses
  for (int i = 0; i < numWebsites; i++) {
    status[i].isUp = false;
    status[i].lastResponseCode = 0;
    status[i].lastResponseTime = 0;
    status[i].lastCheckTime = millis() - (websites[i].checkFrequency * 1000); // Force immediate check
    status[i].totalChecks = 0;
    status[i].successfulChecks = 0;
  }
  
  // Draw initial dashboard
  drawDashboard();
}

void loop() {
  // Check each website based on its frequency
  for (int i = 0; i < numWebsites; i++) {
    unsigned long timeSinceCheck = millis() - status[i].lastCheckTime;
    unsigned long checkInterval = websites[i].checkFrequency * 1000;
    
    if (timeSinceCheck >= checkInterval) {
      checkWebsite(i);
      drawDashboard(); // Redraw after each check
    }
  }
  
  // Update display every 5 seconds
  if (millis() - lastDisplayUpdate >= 5000) {
    lastDisplayUpdate = millis();
    drawDashboard();
  }
  
  // Touch handling for scrolling
  if (ts.tirqTouched() && ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.printf("Touch raw: x=%d y=%d z=%d\n", p.x, p.y, p.z);
    
    // Top swipe zone → scroll up
    if (p.y < 1500 && scrollPos > 0) {
      scrollPos--;
      drawDashboard();
    } 
    // Bottom zone → scroll down
    else if (p.y > 3000 && scrollPos < numWebsites - maxVisibleSites) {
      scrollPos++;
      drawDashboard();
    }
    // Middle zone → force refresh all checks
    else if (p.y >= 1500 && p.y <= 3000) {
      Serial.println("Force checking all websites...");
      for (int i = 0; i < numWebsites; i++) {
        status[i].lastCheckTime = 0; // Force immediate check on next loop
      }
    }
    
    delay(300); // Debounce
  }
}