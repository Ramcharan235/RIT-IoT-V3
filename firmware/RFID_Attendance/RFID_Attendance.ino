// RIT - IOT : RFID Attendance System  (NodeMCU ESP8266)
// Offline: join hotspot "RIT-IOT" (pass 12345678) -> app IP 192.168.4.1 | Online: put your WiFi below
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <MFRC522.h>
const char* STA_SSID="YOUR_WIFI"; const char* STA_PASS="YOUR_PASS";
#define SS_PIN D8
#define RST_PIN D3
const int RELAY[4]={-1,-1,-1,D0}; // SPI uses D5-D7, so only R4 (door lock relay) on D0
#define LED D4   // external LED + 220R
#define BUZ 3    // RX pin (GPIO3) buzzer

ESP8266WebServer sv(80);
WiFiUDP udp;
void discover(){ if(udp.parsePacket()<=0)return; char b[20]; int n=udp.read(b,19); b[n>0?n:0]=0; if(strncmp(b,"RIT_DISCOVER",12)!=0)return;
  String s="{\"device\":\"nodemcu\",\"name\":\"RIT-IOT NodeMCU\"}"; udp.beginPacket(udp.remoteIP(),udp.remotePort()); udp.write((const uint8_t*)s.c_str(),s.length()); udp.endPacket(); } // app finds the board instantly
int rs[4]={0,0,0,0}; bool autoM=true; float v=0,t=0,h=0,dist=100; int d=0,c=0,p=0,door=0; String lastUID="",logS="";
void setR(int i,int s){ if(RELAY[i]<0)return; rs[i]=s; digitalWrite(RELAY[i],s?LOW:HIGH); } // relay modules are active LOW
void beep(int n,int ms=120){ for(int i=0;i<n;i++){ digitalWrite(BUZ,HIGH); digitalWrite(LED,HIGH); delay(ms); digitalWrite(BUZ,LOW); digitalWrite(LED,LOW); delay(ms);} }
void cors(){ sv.sendHeader("Access-Control-Allow-Origin","*"); sv.sendHeader("Access-Control-Allow-Private-Network","true"); }
MFRC522 rfid(SS_PIN,RST_PIN);
String readUID(){ String s=""; for(byte i=0;i<rfid.uid.size;i++){ if(rfid.uid.uidByte[i]<16)s+="0"; s+=String(rfid.uid.uidByte[i],HEX);} s.toUpperCase(); return s; }
String UIDS[5]={"","","","",""};   // filled by the app: Save & send to board
void setup(){
  for(int i=0;i<4;i++){ if(RELAY[i]>=0){ pinMode(RELAY[i],OUTPUT); setR(i,0);} }
  pinMode(LED,OUTPUT); pinMode(BUZ,OUTPUT);
  WiFi.mode(WIFI_AP_STA); WiFi.softAP("RIT-IOT","12345678"); WiFi.begin(STA_SSID,STA_PASS);
  sv.on("/r",[](){cors(); setR(sv.arg("n").toInt()-1,sv.arg("s").toInt()); sv.send(200,"application/json","{}");});
  sv.on("/mode",[](){cors(); sv.send(200,"application/json","{}");});
  sv.on("/cmd",[](){cors(); String k=sv.arg("k"); int x=sv.arg("v").toInt();
    if(k=="auto")autoM=x; if(k=="reset")c=0; if(k=="exit"&&p>0)p--; if(k=="door")door=x; sv.send(200,"application/json","{}");});
  sv.on("/data",[](){cors(); String j="{\"r\":["+String(rs[0])+","+rs[1]+","+rs[2]+","+rs[3]+"],\"v\":"+String(v)+",\"d\":"+d+",\"c\":"+c+",\"p\":"+p+",\"dist\":"+(int)dist+",\"t\":"+(int)t+",\"h\":"+(int)h+",\"auto\":"+(autoM?1:0)+",\"u\":\""+lastUID+"\",\"log\":\""+logS+"\"}"; sv.send(200,"application/json",j);});
  sv.on("/card",[](){cors(); int i=sv.arg("i").toInt(); if(i>=0&&i<5) UIDS[i]=sv.arg("u"); sv.send(200,"application/json","{}");});
  SPI.begin(); rfid.PCD_Init();
  udp.begin(4210);
  sv.begin();
}
unsigned long lt=0;
void loop(){
  sv.handleClient(); discover();
  if(millis()-lt>200){ lt=millis();
    if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
      lastUID=readUID(); c++;                           // c = scan counter, the app reads it
      bool known=false; for(int i=0;i<5;i++) if(UIDS[i]!="" && UIDS[i]==lastUID) known=true;
      if(known) beep(1); else beep(3);                  // 1 beep = registered card, 3 beeps = unknown card
      rfid.PICC_HaltA();
    }
  }
}
