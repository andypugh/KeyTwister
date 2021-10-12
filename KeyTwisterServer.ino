
// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <lx16a-servo.h>
#include <EEPROM.h>
#include <DNSServer.h>

LX16ABus servoBus;
LX16AServo servo(&servoBus, 1);
char buffer[200];
#define __P(f_, ...) snprintf (buffer, 200, (f_), ##__VA_ARGS__) ; content += buffer ;

// Replace with your network credentials
const char* ssid     = "KeyTwister";
const char* password = "123456789";
const byte DNS_PORT = 53;

int pos[] = {0,45,90,120, 2000};

// Set web server port number to 80
WebServer server(80);
DNSServer dnsServer;

void handleShow(){
    // initialise the content string
    String content = "<!DOCTYPE html>";
    __P("<html>");
    __P("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
    __P("<link rel=\"icon\" href=\"data:,\">");
    __P("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    __P(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    __P("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    __P(".button2 {background-color: #555555;}</style></head>");
    
    // Web Page Heading
    __P("<body><h1>Key Twister</h1>");

    __P("Current angle %d\n", (uint)servo.pos_read() / 100);

    __P("<form action=\"/pos0\">");
    __P("<input type=\"number\" id=\"pos0\" name=\"pos\" min=\"0\" max=\"240\" style=\"width:10em;height:5em;font-size:24\" value=\"%i\">\n", pos[0]);
    __P("<input type=\"submit\" style=\"width:10em;height:5em;font-size:24\" value=\"POS0\">");
    __P("</form>");

    __P("<form action=\"/pos1\">");
    __P("<input type=\"number\" id=\"pos1\" name=\"pos\" min=\"0\" max=\"240\" style=\"width:10em;height:5em;font-size:24\" value=\"%i\">\n", pos[1]);
    __P("<input type=\"submit\" style=\"width:10em;height:5em;font-size:24\" value=\"POS1\">");
    __P("</form>");

    __P("<form action=\"/pos2\">");
    __P("<input type=\"number\" id=\"pos2\" name=\"pos\" min=\"0\" max=\"240\" style=\"width:10em;height:5em;font-size:12\" value=\"%i\">\n", pos[2]);
    __P("<input type=\"submit\" style=\"width:10em;height:5em;font-size:24\" value=\"POS2\">");
    __P("</form>");

    __P("<form action=\"/pos3\">");
    __P("<input type=\"number\" id=\"pos3\" name=\"pos\" min=\"0\" max=\"240\" style=\"width:6em;height:2.5em;font-size:12\" value=\"%i\">\n", pos[3]);
    __P("<input type=\"number\" id=\"time\" name=\"time\" min=\"0\" max=\"10000\" style=\"width:6em;height:2.5em;font-size:12\" value=\"%i\">\n", pos[4]);
    __P("<input type=\"submit\" style=\"width:10em;height:5em;font-size:24\" value=\"POS3\">");
    __P("</form>");
    server.send(200, "text/html", content);
}

void handleNotFound(){
  handleShow();
}

void setPos(int p){
  int angle = server.arg("pos").toInt();
  int tim = server.arg("time").toInt();
  if (tim == 0) tim = pos[4];
  Serial.printf("Position %i, Setting angle to %i\n", p, angle);
  if (EEPROM.read(p) != angle) EEPROM.write(p, angle);
  pos[p] = angle;
  if (EEPROM.read(4) != tim / 40) EEPROM.write(4, tim / 40);
  EEPROM.commit();
  pos[4] = tim;
  int current_angle = servo.pos_read() / 100;
  Serial.printf("Current angle %i\n", servo.pos_read());
  servo.move_time(angle * 100, abs(angle - current_angle) * 10);
  if (p == 3){ // twist and release
    delay(tim);
    servo.move_time(pos[2] * 100, abs(pos[3] - pos[2]) * 10);
  }
  handleShow();
}
void setPos0(){
  setPos(0);
}
void setPos1(){
  setPos(1);
}
void setPos2(){
  setPos(2);
}
void setPos3(){
  setPos(3);
}

void setup() {
  Serial.begin(115200);
  
  servoBus.beginOnePinMode(&Serial2,12);
  servoBus.debug(1);
  servoBus.retry=1;
  
  //servo.calibrate(0, 0, 24000);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
    
  // Remove the password parameter, if you want the AP (Access Point) to be open
  // WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.on("/", handleShow);
  server.on("/pos0", setPos0);
  server.on("/pos1", setPos1);
  server.on("/pos2", setPos2);
  server.on("/pos3", setPos3);
  //server.onNotFound(handleNotFound);
  server.onNotFound([]() {server.send(404, "text/plain", "FileNotFound");});
  server.begin();

  //dnsServer.start(DNS_PORT, "*", IP);
  
  EEPROM.begin(10);
  if (EEPROM.read(5) == 111){;
    pos[0] = EEPROM.read(0);
    pos[1] = EEPROM.read(1);
    pos[2] = EEPROM.read(2);
    pos[3] = EEPROM.read(3);
    pos[4] = EEPROM.read(4) * 40;
  } else {
    EEPROM.write(5, 111);
  }
}

void loop(){
  //dnsServer.processNextRequest();
  server.handleClient();
}
