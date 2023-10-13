#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include "HTTPTemplates.h"
#include <WiFi.h>
#include <PubSubClient.h>

#include "Config.h"
#include "SerialCom.h"
#include "Types.h"
#include <WebSerial.h>

particleSensorState_t state;

uint8_t mqttRetryCounter = 0;

DNSServer dns;
AsyncWebServer server(80);

AsyncWiFiManager wifiManager(&server, &dns);
WiFiClient wifiClient;
PubSubClient mqttClient;

AsyncWiFiManagerParameter custom_mqtt_server("server", "mqtt server", Config::mqtt_server, sizeof(Config::mqtt_server));
AsyncWiFiManagerParameter custom_mqtt_user("user", "MQTT username", Config::username, sizeof(Config::username));
AsyncWiFiManagerParameter custom_mqtt_pass("pass", "MQTT password", Config::password, sizeof(Config::password));

uint32_t lastMqttConnectionAttempt = 0;
const uint16_t mqttConnectionInterval = 60000; // 1 minute = 60 seconds = 60000 milliseconds

uint32_t statusPublishPreviousMillis = 0;
const uint16_t statusPublishInterval = 30000; // 30 seconds = 30000 milliseconds

char identifier[24];
#define FIRMWARE_PREFIX "esp32-aqm-particle-sensor"
#define AVAILABILITY_ONLINE "online"
#define AVAILABILITY_OFFLINE "offline"
char MQTT_TOPIC_AVAILABILITY[128];
char MQTT_TOPIC_STATE[128];
char MQTT_TOPIC_COMMAND[128];

char MQTT_TOPIC_AUTOCONF_WIFI_SENSOR[128];
char MQTT_TOPIC_AUTOCONF_PM25_SENSOR[128];

bool shouldSaveConfig = false;

void saveConfigCallback() {
    shouldSaveConfig = true;
}

String getUptime() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  return String(String(days) + "d " + String(hours % 24) + "h " + String(minutes % 60) + "m " + String(seconds % 60) + "s");
}

void handle_index(AsyncWebServerRequest *request)
{
  String page;

  page += String(AG_HTTP_HEAD_START);
  page += String("<title>" + String(identifier) + "</title>");
  page += String("<meta http-equiv=\"refresh\" content=\"5\">");
  page += String(AG_HTTP_STYLE);
  page += String(AG_HTTP_HEAD_END);
  page += String("<h3>" + String(identifier) + "</h3><hr><dl>");
  page += String("<dt>Uptime</dt><dd>" + getUptime() + "</dd>");
  page += String("<dt>Last Report to MQTT:</dt><dd>" + String((millis() - (statusPublishPreviousMillis)) / 1000) + " seconds ago</dd>");
  page += String("<dt>Connected to MQTT:</dt><dd>" + String(isMqttConnected() ? "Connected" : "Not Connected") + "</dd>");
  page += String("<dt>MQTT Server:</dt><dd>" + String(Config::mqtt_server) + "</dd>");
  page += String("<dt>Current PM2</dt><dd>" + String(state.avgPM25) + " μg/m³</dd>");
  page += String("</dl><hr>");
  page += String("<br/><br/><form method='POST' action='/reboot'><button type='submit'>Reboot</button></form><br><form method='POST' action='/reset'><button type='submit'>Reset Configuration</button></form>");
  page += String(AG_HTTP_END);

  request->send(200, "text/html", page);
}


#define LED_BUILTIN 4

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
  if (d == "PUBLISH"){
    publishState();
  }
  if (d=="CONFIG"){
    publishAutoConfig();
  }
  if (d == "RECONNECT") {
    WebSerial.println("Reconnecting to mqtt...");
    mqttReconnect();
  }
  if (d == "CONNECTED") {
    WebSerial.println(isMqttConnected() ? "Connected" : "Not connected");
  }
  if (d == "CREDS") {
    WebSerial.println(Config::mqtt_server);
    WebSerial.println(Config::username);
    WebSerial.println(Config::password);
  }
  if (d == "LAST") {
    WebSerial.println(statusPublishPreviousMillis);
  }
}

void setup() {
    Serial.begin(115200);
    SerialCom::setup();

    Serial.println("\n");
    Serial.println("Hello from esp32-aqm-particle-sensor");
    Serial.printf("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
    /*Serial.printf("Core Version: %s\n", ESP.getCoreVersion().c_str());
    Serial.printf("Boot Version: %u\n", ESP.getBootVersion());
    Serial.printf("Boot Mode: %u\n", ESP.getBootMode());
    Serial.printf("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Reset reason: %s\n", ESP.getResetReason().c_str());*/

    delay(3000);

    snprintf(identifier, sizeof(identifier), "AQM-%S", ESP.getChipModel());
    snprintf(MQTT_TOPIC_AVAILABILITY, 127, "%s/%s/status", FIRMWARE_PREFIX, identifier);
    snprintf(MQTT_TOPIC_STATE, 127, "%s/%s/state", FIRMWARE_PREFIX, identifier);
    snprintf(MQTT_TOPIC_COMMAND, 127, "%s/%s/command", FIRMWARE_PREFIX, identifier);

    snprintf(MQTT_TOPIC_AUTOCONF_PM25_SENSOR, 127, "homeassistant/sensor/%s/%s_pm25/config", FIRMWARE_PREFIX, identifier);
    snprintf(MQTT_TOPIC_AUTOCONF_WIFI_SENSOR, 127, "homeassistant/sensor/%s/%s_wifi/config", FIRMWARE_PREFIX, identifier);

    WiFi.hostname(identifier);

    Config::load();

    setupWifi();
    setupOTA();
    mqttClient.setServer(Config::mqtt_server, 1883);
    mqttClient.setKeepAlive(10);
    mqttClient.setBufferSize(2048);
    mqttClient.setCallback(mqttCallback);

    Serial.printf("Hostname: %s\n", identifier);
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

    Serial.println("-- Current GPIO Configuration --");
    Serial.printf("PIN_UART_RX: %d\n", SerialCom::PIN_UART_RX);

    mqttReconnect();
    server.on("/", HTTP_GET, handle_index);
    server.on("/reboot", HTTP_POST, handle_index);
    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Resetting config and restarting!");
        resetWifiSettingsAndReboot();
    });
    
    WebSerial.begin(&server);
    WebSerial.msgCallback(recvMsg);

    server.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
}

void setupOTA() {
    ArduinoOTA.onStart([]() { WebSerial.println("Start"); });
    ArduinoOTA.onEnd([]() { WebSerial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        WebSerial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        WebSerial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            WebSerial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            WebSerial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            WebSerial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            WebSerial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            WebSerial.println("End Failed");
        }
    });

    ArduinoOTA.setHostname(identifier);

    // This is less of a security measure and more a accidential flash prevention
    ArduinoOTA.setPassword(identifier);
    ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    SerialCom::handleUart(state);
    mqttClient.loop();

    const uint32_t currentMillis = millis();
    if (currentMillis - statusPublishPreviousMillis >= statusPublishInterval) {
        statusPublishPreviousMillis = currentMillis;

        if (state.valid) {
            WebSerial.println("Publish state");
            publishState();
        }
    }

    if (!mqttClient.connected() && currentMillis - lastMqttConnectionAttempt >= mqttConnectionInterval) {
        lastMqttConnectionAttempt = currentMillis;
        WebSerial.println("Reconnect mqtt");
        mqttReconnect();
    }
    //server.handleClient();
}

void setupWifi() {
    wifiManager.setDebugOutput(false);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);

    WiFi.hostname(identifier);
    wifiManager.autoConnect(identifier);
    mqttClient.setClient(wifiClient);

    strcpy(Config::mqtt_server, custom_mqtt_server.getValue());
    strcpy(Config::username, custom_mqtt_user.getValue());
    strcpy(Config::password, custom_mqtt_pass.getValue());

    if (shouldSaveConfig) {
        Config::save();
    } else {
        // For some reason, the read values get overwritten in this function
        // To combat this, we just reload the config
        // This is most likely a logic error which could be fixed otherwise
        Config::load();
    }
}

void resetWifiSettingsAndReboot() {
    wifiManager.resetSettings();
    delay(3000);
    ESP.restart();
}

void mqttReconnect() {
    for (uint8_t attempt = 0; attempt < 3; ++attempt) {
        if (mqttClient.connect(identifier, Config::username, Config::password, MQTT_TOPIC_AVAILABILITY, 1, true, AVAILABILITY_OFFLINE)) {
            mqttClient.publish(MQTT_TOPIC_AVAILABILITY, AVAILABILITY_ONLINE, true);
            publishAutoConfig();

            // Make sure to subscribe after polling the status so that we never execute commands with the default data
            mqttClient.subscribe(MQTT_TOPIC_COMMAND);
            break;
        }
        delay(5000);
    }
}

bool isMqttConnected() {
    return mqttClient.connected();
}

void publishState() {
    DynamicJsonDocument wifiJson(192);
    DynamicJsonDocument stateJson(604);
    char payload[256];

    wifiJson["ssid"] = WiFi.SSID();
    wifiJson["ip"] = WiFi.localIP().toString();
    wifiJson["rssi"] = WiFi.RSSI();

    stateJson["pm25"] = state.avgPM25;

    stateJson["wifi"] = wifiJson.as<JsonObject>();

    serializeJson(stateJson, payload);
    mqttClient.publish(&MQTT_TOPIC_STATE[0], &payload[0], true);
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) { }

void publishAutoConfig() {
    char mqttPayload[2048];
    DynamicJsonDocument device(256);
    DynamicJsonDocument autoconfPayload(1024);
    StaticJsonDocument<64> identifiersDoc;
    JsonArray identifiers = identifiersDoc.to<JsonArray>();

    identifiers.add(identifier);

    device["identifiers"] = identifiers;
    device["manufacturer"] = "Ikea";
    device["model"] = "aqm";
    device["name"] = identifier;
    device["sw_version"] = "2021.08.0";

    autoconfPayload["device"] = device.as<JsonObject>();
    autoconfPayload["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
    autoconfPayload["state_topic"] = MQTT_TOPIC_STATE;
    autoconfPayload["name"] = identifier + String(" WiFi");
    autoconfPayload["value_template"] = "{{value_json.wifi.rssi}}";
    autoconfPayload["unique_id"] = identifier + String("_wifi");
    autoconfPayload["unit_of_measurement"] = "dBm";
    autoconfPayload["json_attributes_topic"] = MQTT_TOPIC_STATE;
    autoconfPayload["json_attributes_template"] = "{\"ssid\": \"{{value_json.wifi.ssid}}\", \"ip\": \"{{value_json.wifi.ip}}\"}";
    autoconfPayload["icon"] = "mdi:wifi";

    serializeJson(autoconfPayload, mqttPayload);
    mqttClient.publish(&MQTT_TOPIC_AUTOCONF_WIFI_SENSOR[0], &mqttPayload[0], true);

    autoconfPayload.clear();

    autoconfPayload["device"] = device.as<JsonObject>();
    autoconfPayload["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
    autoconfPayload["state_topic"] = MQTT_TOPIC_STATE;
    autoconfPayload["name"] = identifier + String(" PM 2.5");
    autoconfPayload["unit_of_measurement"] = "μg/m³";
    autoconfPayload["value_template"] = "{{value_json.pm25}}";
    autoconfPayload["unique_id"] = identifier + String("_pm25");
    autoconfPayload["icon"] = "mdi:air-filter";

    serializeJson(autoconfPayload, mqttPayload);
    mqttClient.publish(&MQTT_TOPIC_AUTOCONF_PM25_SENSOR[0], &mqttPayload[0], true);

    autoconfPayload.clear();
}
