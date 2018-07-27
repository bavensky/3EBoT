#include "WiFiModule.h"
#include "LCDModule.h"

#define WIFI_CONFIG_FILE "/wifi.json"

extern LCDModule* lcdModule;

void WiFiModule::configLoop() {
  if (digitalRead(0) == HIGH) {
    while(digitalRead(0) == HIGH) {
      delay(10); 
    } 
    File f = SPIFFS.open("/enabled", "a+");
    delay(100);
    ESP.restart();
  }
}

void WiFiModule::config(CMMC_System *os, AsyncWebServer *server)
{
  strcpy(this->path, "/api/wifi/sta");
  static WiFiModule *that = this;
  this->_serverPtr = server;
  this->_managerPtr = new CMMC_ConfigManager(WIFI_CONFIG_FILE);
  this->_managerPtr->init();
  this->_managerPtr->load_config([](JsonObject *root, const char *content) {
    if (root == NULL)
    {
      Serial.print("wifi.json failed. >");
      Serial.println(content);
      return;
    }
    Serial.println("[user] wifi config json loaded..");
    const char *sta_config[2];
    sta_config[0] = (*root)["sta_ssid"];
    sta_config[1] = (*root)["sta_password"];
    if ((sta_config[0] == NULL) || (sta_config[1] == NULL))
    {
      Serial.println("NULL..");
      SPIFFS.remove("/enabled");
      return;
    };
    strcpy(that->sta_ssid, sta_config[0]);
    strcpy(that->sta_pwd, sta_config[1]);
  });
  if (lcdModule) {
    lcdModule->displayConfigWiFi(); 
  }
  this->configWebServer();
}

void WiFiModule::configWebServer()
{
  static WiFiModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest *request) {
    String output = that->saveConfig(request, this->_managerPtr);
    request->send(200, "application/json", output);
  });
}

void WiFiModule::isLongPressed()
{
  uint32_t prev = millis();
  while (digitalRead(15) == HIGH)
  {
    delay(50);
    if ((millis() - prev) > 5L * 1000L)
    {
      Serial.println("LONG PRESSED.");
      while (digitalRead(15) == HIGH)
      {
        delay(10);
      }
    }
  }
}
void WiFiModule::setup()
{
  _init_sta();
}

void WiFiModule::loop() {}

void WiFiModule::_init_sta()
{
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(20);
  WiFi.begin(sta_ssid, sta_pwd);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("Connecting to %s:%s\r\n", sta_ssid, sta_pwd); 
    lcdModule->displayConnectingWiFi(sta_ssid);
    isLongPressed();
    delay(300);
  }
  lcdModule->displayWiFiConnected();
  Serial.println("WiFi Connected.");
}