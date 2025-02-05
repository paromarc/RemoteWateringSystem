#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LCD_I2C.h>

//set I2C ID (0x27 for PCF8574 I2C adapter. 16 columns by 2 rows display)
LCD_I2C lcd(0x27, 16, 2);

//Wireless Network Configuration
const char *ssid = "YourNetworkSSID";  
const char *password = "YourWifiPassword";
//config http server to listen to port 80
WebServer server(80);

// root default response
void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");  
}

// /on handle. Switch relay to active and trigger the water pump on
void on() {
  //http response
  server.send(200, "text/plain", "Water ON !");
  //turn pump on
  digitalWrite(32, HIGH);
}

// /off handle. Switch relay to inactive and trigger the water pump off
void off() {
  //http response
  server.send(200, "text/plain", "Water OFF !");
  //turn pump off
  digitalWrite(32, LOW);
}

// /pulse handle. Switch relay to active for 400ms (this is probably too fast for the relay to respond but setting it like this anyway :-) and then switch it off)
void pulse() {
  //http response
  server.send(200, "text/plain", "Water On for 400ms !");
  //turn pump on 
  digitalWrite(32, HIGH);
  //wait 400ms
  delay(400);
  //turn pump off
  digitalWrite(32, LOW);
}

//manage unknown http handle
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

//initial Arduino setup
void setup() {
  //LCD
  lcd.begin();
  lcd.backlight();

  //set serial port (115200 baud rate) debug purpose
  Serial.begin(115200);

  //wireless configuration
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //log wireless connection state in terminal for debug purpose 
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //position led text first row (0) second column (1)
  lcd.setCursor(1, 0);
  //text IP Address:
  lcd.print("IP Address:");
  //position led text second row (1) second column (1)
  lcd.setCursor(1, 1);
  //text device IP Address
  lcd.print(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  //HTTP handles setup
  server.on("/", handleRoot);
  server.on("/on", on);
  server.on("/off", off);
  server.on("/pulse",pulse);
  server.onNotFound(handleNotFound);
  //start HTTP server
  server.begin();
  //debug
  Serial.println("HTTP server started");  

  //relay. Pin 32 config as output
  pinMode(32,OUTPUT); 
}

// put your main code here, to run repeatedly: 
void loop() {
  //HTTP listener
  server.handleClient();  
  //short delay before re-running loop
  delay(1);
}
