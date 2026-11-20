// RIT - IOT : IR Car Parking (servo gate + LCD)  (NodeMCU ESP8266)
// Offline: join hotspot "RIT-IOT" (pass 12345678) -> app IP 192.168.4.1 | Online: put your WiFi below
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
const char* STA_SSID="YOUR_WIFI"; const char* STA_PASS="YOUR_PASS";
const int RELAY[4]={D5,D6,D7,D0}; // R1 Light, R2 Fan, R3 TV/Relay, R4 Pump (active LOW)
#define SEN D3
#define SRV D8
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
LiquidCrystal_I2C lcd(0x27,16,2);
void say(String a,String b=""){ lcd.clear(); lcd.setCursor(0,0); lcd.print(a); lcd.setCursor(0,1); lcd.print(b); }
Servo gate; int lastIR=1;
void showP(){ if(p>=3) say("PARKING FULL","Slots 3/3"); else say("Parking Status","Free: "+String(3-p)+"/3"); }
void openGate(){ for(int a=0;a<=90;a++){gate.write(a);delay(25);sv.handleClient();} delay(3000); for(int a=90;a>=0;a--){gate.write(a);delay(25);sv.handleClient();} }
void setup(){
  for(int i=0;i<4;i++){ if(RELAY[i]>=0){ pinMode(RELAY[i],OUTPUT); setR(i,0);} }
  pinMode(LED,OUTPUT); pinMode(BUZ,OUTPUT);
  WiFi.mode(WIFI_AP_STA); WiFi.softAP("RIT-IOT","12345678"); WiFi.begin(STA_SSID,STA_PASS);
  sv.on("/r",[](){cors(); setR(sv.arg("n").toInt()-1,sv.arg("s").toInt()); sv.send(200,"application/json","{}");});
  sv.on("/mode",[](){cors(); sv.send(200,"application/json","{}");});
  sv.on("/cmd",[](){cors(); String k=sv.arg("k"); int x=sv.arg("v").toInt();
    if(k=="auto")autoM=x; if(k=="reset")c=0; if(k=="exit"&&p>0)p--; if(k=="door")door=x; sv.send(200,"application/json","{}");});
  sv.on("/data",[](){cors(); String j="{\"r\":["+String(rs[0])+","+rs[1]+","+rs[2]+","+rs[3]+"],\"v\":"+String(v)+",\"d\":"+d+",\"c\":"+c+",\"p\":"+p+",\"dist\":"+(int)dist+",\"t\":"+(int)t+",\"h\":"+(int)h+",\"auto\":"+(autoM?1:0)+",\"u\":\""+lastUID+"\",\"log\":\""+logS+"\"}"; sv.send(200,"application/json",j);});
  lcd.init(); lcd.backlight(); say("RIT IOT","Ready");
  gate.attach(SRV); gate.write(0); pinMode(SEN,INPUT); showP();
  udp.begin(4210);
  sv.begin();
}
unsigned long lt=0;
void loop(){
  sv.handleClient(); discover();
  if(millis()-lt>200){ lt=millis();
    int x=digitalRead(SEN);
    if(x==0 && lastIR==1){
      if(p<3){ beep(3); openGate(); p++; }     // car enters: LED+buzzer x3, gate opens slowly, closes after 3 s
      else { for(int i=0;i<6;i++){ digitalWrite(LED,!digitalRead(LED)); delay(150);} digitalWrite(LED,LOW); } // FULL: gate stays shut, red LED blinks
      showP();
    }
    lastIR=x;
    static int lp=-1; if(lp!=p){ lp=p; showP(); }   // also updates when app presses "Car left" 
  }
}
