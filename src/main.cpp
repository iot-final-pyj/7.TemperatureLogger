#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <DHTesp.h>

const char* ssid = "IoT518";
const char* password = "iot123456";

DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;
char                dht_buffer[10];

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();              // Read humidity (percent)
        temperature = dht.getTemperature();             // Read temperature as Fahrenheit
    }
}

WiFiClient client;

void setup(void){
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    dht.setup(15, DHTesp::DHT22);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to "); Serial.println(ssid);
    Serial.print("IP address: "); Serial.println(WiFi.localIP());

    MDNS.begin("miniwifi");
}

void loop(void){
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    char urlbuf[200];
    readDHT22();
    Serial.printf("Temperature: %.2f\n", temperature);
    sprintf(urlbuf, "http://192.168.82.105:8000/?temperature=%.2f", temperature);
    if (http.begin(client, urlbuf)) {  // HTTP

        Serial.print("[HTTP] GET...\n");  // start connection and send HTTP header
        int httpCode = http.GET();

        if (httpCode > 0) {                // httpCode will be negative on error
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = http.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
        Serial.printf("[HTTP} Unable to connect\n");
    }

    delay(3000);
}
