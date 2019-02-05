#include <WiFiManager.h>          // Version 0.14.0 from Library Manager
#include <ESP8266WiFi.h>          // Version 1.0.0 from Library Manager
#include <WiFiClient.h>           //   - from ESP8266WiFi
#include <ESP8266WebServer.h>     // Version 1.0.0 from Library Manager
#include <ESP8266mDNS.h>          // Version 1.1 from https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS

#define USE_MQTT 1
#define DEBUG 1

#ifdef USE_MQTT
#include <PubSubClient.h>
char mqtt_server[] = "10.1.1.1";
int mqtt_port = 21883;
char mqtt_clientID[] = "Esp8266Relay_1549234326";
char mqtt_username[] = "4966ddf2-85ae-4ea4-bf1a-583ec3ed7932";
char mqtt_password[] = "r:d30d843f328bdb7133468dfc9b32f555";
char mqtt_publish_topic[] = "qiot/things/admin/Esp8266Relay/Line1";
char mqtt_subscribe_topic[] = "qiot/things/admin/Esp8266Relay/ButtonL1";
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int buttonState = 0;
#endif

void handleConfigWifi();

ESP8266WebServer server(80);
#define LIGHT1 2
#define LIGHT2 0
#ifndef DEBUG
#define LIGHTR1 3
#define LIGHTR2 1
#endif
char _webMain[] = "\
<!DOCTYPE HTML>\
<html>\
  <body>\
    <table>\
      <tr>\
        <td>Lights 1</td>\
        <td>\
          <form action='/lights1'>\
            <input style='height:200px;width:200px' type='submit' name='Status' value='Activate'/>\
          </form>\
        </td>\
        <td>\
          <form action='/lights1'>\
            <input style='height:200px;width:200px' type='submit' name='Status' value='Deactivate'/>\
          </form>\
        </td>\
      </tr>\
      <tr>\
        <td>Lights 2</td>\
        <td>\
          <form action='/lights2'>\
            <input style='height:200px;width:200px' type='submit' name='Status' value='Activate'/>\
          </form>\
        </td>\
        <td>\
          <form action='/lights2'>\
            <input style='height:200px;width:200px' type='submit' name='Status' value='Deactivate'/>\
          </form>\
        </td>\
      </tr>\
      <tr>\
        <td>\
          <form action='/ConfigWifi'>\
            <input style='height:200px;width:200px' type='submit' name='Configure Wifi' value='ConfigWifi'/>\
          </form>\
        </td>\
        <td>\
          <form action='/RebootESP8266'>\
            <input style='height:200px;width:200px' type='submit' name='Reboot' value='Reboot'/>\
          </form>\
        </td>\
      </tr>\
    </table>\
  </body>\
</html>\
";

void handleRoot() {
  server.send(200, "text/html", _webMain);
#ifdef DEBUG
  Serial.println("sent root webpage");
#endif
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void startWebServer() {
  server.on("/", handleRoot);

  server.on("/lights1", HTTP_GET, []() {
#ifndef DEBUG
    char ctmp[20] = "";
    server.send(200, "text/html", strcat(_webMain, itoa(digitalRead(LIGHTR1),ctmp,10)));
#else
    server.send(200, "text/html", _webMain);
#endif
    // Relay board with low level trigger
    if (server.arg("Status") == String("Activate")) {
      digitalWrite(LIGHT1, 0);
    } else if (server.arg("Status") == String("Deactivate")) {
      digitalWrite(LIGHT1, 1);
    }
  });

  server.on("/lights2", HTTP_GET, []() {
#ifndef DEBUG
    char ctmp[20] = "";
    server.send(200, "text/html", strcat(_webMain, itoa(digitalRead(LIGHTR2),ctmp,10)));
#else
    server.send(200, "text/html", _webMain);
#endif
    // Relay board with low level trigger
    if (server.arg("Status") == String("Activate")) {
      digitalWrite(LIGHT2, 0);
    } else if (server.arg("Status") == String("Deactivate")) {
      digitalWrite(LIGHT2, 1);
    }
  });

  server.on("/ConfigWifi", handleConfigWifi);
  
  server.on("/RebootESP8266", handleReboot);

  server.onNotFound(handleNotFound);

  server.begin();
#ifdef DEBUG
  Serial.println("HTTP server started");
#endif
}

void handleConfigWifi() {
  //WiFiManager
  WiFiManager wifiManager;
  static int done = 0;

  server.send(200, "text/html", "<!DOCTYPE HTML><html><body>Please, connect to " + wifiManager.getConfigPortalSSID() + " and point to <A href=192.168.4.1>192.168.4.1</A> to configure the Wifi Settings.</body></html>");
  server.stop();
  delay(5000);
  WiFi.disconnect();
  delay(5000);

  //reset settings
  //wifiManager.resetSettings();
  done = 0;

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);

  //it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
  //WiFi.mode(WIFI_STA);

  if (!wifiManager.startConfigPortal("AutoConnectAP", "password")) {
#ifdef DEBUG    
    Serial.println("failed to connect, we should reset as see if it connects");
#endif
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  if (done == 0) {
    //if you get here you have connected to the WiFi
#ifdef DEBUG
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
    done = 1;
  }

  startWebServer();

}

void handleReboot() {
  server.send(200, "text/html", _webMain);
  ESP.restart();
}

void setup(void) {
  //WiFiManager
  WiFiManager wifiManager;

  pinMode(LIGHT1, OUTPUT);
  digitalWrite(LIGHT1, 1);
  pinMode(LIGHT2, OUTPUT);
  digitalWrite(LIGHT2, 1);
#ifndef DEBUG
  pinMode(LIGHTR1, INPUT);
  pinMode(LIGHTR2, INPUT);
#else
  //Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  Serial.begin(115200);
#endif

#ifdef USE_MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
#endif

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
#ifdef DEBUG
    Serial.println("failed to connect, we should reset as see if it connects");
#endif
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  
#ifdef DEBUG
  //if you get here you have connected to the WiFi

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  if (MDNS.begin("esp8266")) {
#ifdef DEBUG
    Serial.println("MDNS responder started");
#endif
  }

  startWebServer();
}

#ifdef USE_MQTT
void callback(char* topic, byte* payload, unsigned int length) {
#ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on 
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
#endif
    // Attempt to connect
    if (client.connect(mqtt_clientID,mqtt_username,mqtt_password)) {
#ifdef DEBUG
      Serial.println("connected");
#endif
      //Subscribe
      client.subscribe(mqtt_subscribe_topic);
    } else {
#ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
#endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
#endif

void loop(void) {
#ifdef USE_MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(!digitalRead(LIGHT1) != buttonState){
    String msg_a = "{\"value\":"+ String(buttonState) +"}";
    msg_a.toCharArray(msg, 50);
#ifdef DEBUG
    Serial.println(msg);
#endif
    //Publish
    client.publish(mqtt_publish_topic, msg); 
    buttonState = !digitalRead(LIGHT1);
  }
#endif
  server.handleClient();
}
