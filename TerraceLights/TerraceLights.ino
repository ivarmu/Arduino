#include <WiFiManager.h>          // Version 0.14.0 from Library Manager
#include <ESP8266WiFi.h>          // Version 1.0.0 from Library Manager
#include <WiFiClient.h>           //   - from ESP8266WiFi
#include <ESP8266WebServer.h>     // Version 1.0.0 from Library Manager
#include <ESP8266mDNS.h>          // Version 1.1 from https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS

ESP8266WebServer server(80);
#define LIGHT1 2
#define LIGHT2 0
#define LIGHTR1 3
#define LIGHTR2 1
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
      </tr>\
    </table>\
  </body>\
</html>\
";

void handleRoot() {
  server.send(200, "text/html", _webMain);
  //Serial.println("sent root webpage");
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
    char ctmp[20] = "";
    server.send(200, "text/html", strcat(_webMain, itoa(digitalRead(LIGHTR1),ctmp,10)));
    // Relay board with low level trigger
    if (server.arg("Status") == String("Activate")) {
      digitalWrite(LIGHT1, 0);
    } else if (server.arg("Status") == String("Deactivate")) {
      digitalWrite(LIGHT1, 1);
    }
  });

  server.on("/lights2", HTTP_GET, []() {
    char ctmp[20] = "";
    server.send(200, "text/html", strcat(_webMain, itoa(digitalRead(LIGHTR2),ctmp,10)));
    // Relay board with low level trigger
    if (server.arg("Status") == String("Activate")) {
      digitalWrite(LIGHT2, 0);
    } else if (server.arg("Status") == String("Deactivate")) {
      digitalWrite(LIGHT2, 1);
    }
  });

  server.on("/ConfigWifi", handleConfigWifi);

  server.onNotFound(handleNotFound);

  server.begin();
  //Serial.println("HTTP server started");
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
    //Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  if (done == 0) {
    //if you get here you have connected to the WiFi
    /*
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    */
    done = 1;
  }

  startWebServer();

}

void setup(void) {
  //WiFiManager
  WiFiManager wifiManager;

  pinMode(LIGHT1, OUTPUT);
  digitalWrite(LIGHT1, 1);
  pinMode(LIGHT2, OUTPUT);
  digitalWrite(LIGHT2, 1);
  pinMode(LIGHTR1, INPUT);
  pinMode(LIGHTR2, INPUT);
  //Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    //Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  /*
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  */
  startWebServer();
}

void loop(void) {

  server.handleClient();

}
