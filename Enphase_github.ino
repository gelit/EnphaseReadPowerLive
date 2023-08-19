/*
Adapt Parameters 
- P1(WiFi SSID & password)
- P2 (Token)

My Serial Output :
Hello Enphase Hobbyist !
My_IP1=192.168.1.128
mDNS request : envoy.local=192.168.1.127  normal value
End of setup
mDNS request : envoy.local=192.168.1.127  normal value
Req    Solar_W=4377  Grid_W=-2498
*/

const byte FORMAT = 1; 

byte Pause = 1;                               // https request every min
int Solar_W, Cons_W, Grid_W;

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP_SSLClient.h>

ESP_SSLClient ssl_client; 
WiFiClient basic_client;

WiFiUDP Udp;
  
int NowHour, NowHourM, NowMinute;
unsigned long T1,T2;  // Wifi
unsigned long T5;     // Pause

IPAddress ip(0,0,0,0);

#include <mDNSResolver.h>            
#define NAME_TO_RESOLVE "envoy.local"
using namespace mDNSResolver;
Resolver resolver(Udp);

bool Resolve() { // j'ai perdu un jour début août avant de comprendre que envoy.local avait changé d'adrIP !!!???
  bool ok = 0;
  IPAddress ipL(0,0,0,0);
  Serial.print("mDNS request : ");
  resolver.setLocalIP(WiFi.localIP());
  ipL = resolver.search(NAME_TO_RESOLVE);
  Serial.print(NAME_TO_RESOLVE);
  if(ipL != INADDR_NONE) { ok=1; Serial.print("="); Serial.println(ipL); ip=ipL; }
  else                   { Serial.println(" unresolved"); }
  return ok;
}

void setup() { //=======================================================SETUP
  
  Serial.begin(115200); Serial.println(); Serial.println("Hello Enphase Hobbyist !");

  ssl_client.setInsecure();             // ignore server ssl certificate verification
  ssl_client.setClient(&basic_client);  // assign the basic client

  if (WifiON()) {
    delay(1000);
    Serial.print("My_IP1="); Serial.println(WiFi.localIP());
    delay(1000);
    Resolve(); 
    delay(1000);
    WifiOFF();
  }
  else {Serial.println("No WiFi setup");}

  T5 = millis() + (10000);
  Serial.println("End of setup");
}

void loop() {
  
  if (millis() > T5) { //-------------------------------------------- each Pause en min
    if (WifiON()) {
      T5 = millis() + (10000);
      if (Resolve()) { Serial.print("Req  "); delay(2000); Enphase(); delay(1000); }  // 18 aout : add mDNS before Enphase (after 1 DOWN day) !!!
      WifiOFF();
    }
    else {Serial.println("No WiFi loop");}
  }
}

bool WifiON() {
  const char* ssid = "xyz";                    // P1
  const char* pass = "123";                    // P1

  bool Status=0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);  // appel bloquant --> no timeout
  T1=millis()+10000;       
  
  do {
    T2=millis();
    delay(500);
  }  while ((WiFi.status() != WL_CONNECTED) && (T2<T1));
  
  if (WiFi.status() == WL_CONNECTED) {Status=1;}
  if (T2>=T1)                        {Serial.println("No Wifi");}

  return Status;
  delay(2000);
}

void WifiOFF() {
  WiFi.disconnect();  // don't use .end
}

void Enphase() {
  String S1,S2,S3;
  int Data=0; 
  int N=1;
  char Buf[20];
  S1=0;
    
  if (ssl_client.connect(ip, 443)) {
    
    ssl_client.println("GET /ivp/meters/readings HTTP/1.1");    // you can change this URL
    ssl_client.print  ("Host: "); ssl_client.println(ip);
    ssl_client.println("Accept: application/json");
    ssl_client.println("Authorization: Bearer eyJra ...cxkVA");  // P2 = Token
    ssl_client.println("Connection: close");
    ssl_client.println("");

    unsigned long ms = millis();
    while (!ssl_client.available() && millis() - ms < 3000) {delay(0);}
    while (ssl_client.available()) { if ( FORMAT == 0 ) { Serial.print((char)ssl_client.read()); }
                                     else               { char ch = ssl_client.read(); S1 += ch; } 
    }

    ssl_client.stop();
  }
  else { Serial.println("  No SSL connexion"); }

  S2=S1;
  Solar_W=0; Grid_W=0;

  for (int i=0; i < S1.length(); i++) {  // Parse
    S3=0;
    if (S2.startsWith("\"activePower")) {
    //Serial.print("N="); Serial.println(N);
      i += 15;  int j=15;
      do {//Serial.print(S2.charAt(j));
            S3 += S2.charAt(j); i++; j++;}
      while (S2.charAt(j) != '.');
        
      S3 = S3.substring(1);
      int L=S3.length();
      S3.toCharArray(Buf,L+1);
      Data = atoi(Buf);

      if (N==1) { Solar_W=Data; Serial.print("  Solar_W="); Serial.print(Solar_W);  }   // N=2-4 : phase R S T
      if (N==5) { Grid_W=Data;  Serial.print("  Grid_W=");  Serial.println(Grid_W); }    // N=6-8 : phase R S T
        
        S2 = S2.substring(j);
        N++;
      }
    S2 = S2.substring(1);
  }

  if (Grid_W==0) { Serial.println("  No DATA");}  // Problem with format
}
