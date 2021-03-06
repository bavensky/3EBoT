#include "WiFiModule.h"
#include "LCDModule.h"

#define WIFI_CONFIG_FILE "/wifi.json"

extern LCDModule* lcdModule;


void WiFiModule::isLongPressed() {
  if (digitalRead(15) == HIGH) {
    lcdModule->displayLogo();
    Serial.println("15 PRESSED.");
    while(digitalRead(15) == HIGH) {
      delay(10); 
    } 
    SPIFFS.remove("/enabled");
    digitalWrite(0, HIGH);
    delay(1000);
    ESP.restart();
  } 
}
void WiFiModule::configLoop() {
  isLongPressed();
}

void WiFiModule::configSetup() {
  lcdModule->displayConfigWiFi(); 
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

void WiFiModule::setup()
{
  _init_sta();
}

void WiFiModule::loop() {

}

void WiFiModule::_init_sta()
{
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(20);
  WiFi.begin(sta_ssid, sta_pwd);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("Connecting to %s:%s\r\n", sta_ssid, sta_pwd); 
    lcdModule->displayConnectingWiFi(sta_ssid, counter++);
    isLongPressed();
    delay(500);
  }
  lcdModule->displayWiFiConnected();
  Serial.println("WiFi Connected.");
}