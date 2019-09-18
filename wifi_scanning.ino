#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#define turn_on_wifi() WiFi.forceSleepWake()
#define turn_off_wifi() WiFi.forceSleepBegin();  // 15 mA else 70 mA 
#define _wdt_enable(t) ESP.wdtEnable(t)
#define LED_BUILTIN2 2
#else
#include <WiFi.h>
#include <PowerManagement.h>
//#include "wiring_watchdog.h"
#define turn_on_wifi()
#define turn_off_wifi()
#define _wdt_enable(t) wdt_enable(t)
#define LED_BUILTIN2 5
#endif

#define SLEEP_TIME 10
#define USE_DEEP_SLEEP

#define max_SSID 10
#define SSID_size 15
#define MAC_size 6
#define BSSID_size MAC_size
typedef struct { char SSID[SSID_size]; byte BSSID[BSSID_size]; byte encryptionType; short RSSI; } SSID_t;

char ssid[] = "R906";     // your network SSID (name)
//char ssid[] = "mobile-h";     // your network SSID (name)
//char pass[] = "03310331";  // your network password
//char ssid[] = "WL2";     // your network SSID (name)
char pass[] = "r906r906";  // your network password
//char pass[] = "03310331";  // your network password
//int status  = WL_IDLE_STATUS;    // the Wifi radio's status

const uint16_t port = 60;
//const char * host = "192.168.75.94"; // ip or dns
//const char * host = "10.20.15.150"; // ip or dns
//const char * host = "192.168.1.101"; // ip or dns
const char * host = "192.168.0.122"; // ip or dns
//const char * host = "xds.ym.edu.tw"; // ip or dns
//const char * host = "120.126.84.239"; // ip or dns
//const char * host = "xds2.ym.edu.tw"; // ip or dns
//const uint16_t port = 80;
//const char * host = "www.ntu.edu.tw"; // ip or dns

/*
#define init_LED() 
#define turn_on_LED()  
#define turn_off_LED()  
*/
#define init_LED() pinMode(LED_BUILTIN2, OUTPUT)
#define turn_on_LED()  digitalWrite(LED_BUILTIN2, LOW)
#define turn_off_LED()  digitalWrite(LED_BUILTIN2, HIGH)

void setup() {
  turn_off_wifi();
  #ifdef ESP8266 
  Serial.begin(74880);
//  Serial.begin(115200);
  delay(10);
  Serial.println();
  #else
  Serial.begin(38400);
  #endif

//    while (!Serial) {}; // wait for serial port to connect. Needed for native USB port only

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);     // don't continue:
    }
  
  #ifdef ESP8266
  WiFi.mode(WIFI_STA);  // ESP8266
  WiFi.disconnect();
//  delay(100);
  #endif  

  init_LED();
  
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC:");
//  printMAC(mac);
  Serial.print(MAC2str(mac));
  Serial.println();

  Serial.println("Setup");
  
}

void warning(byte i) {

  for (; (i); i--) {
    turn_on_LED();
    delay(500);
    turn_off_LED();
    if (i!=1) delay(500);
    }
}

void loop() {
  SSID_t SSIDs[max_SSID];
  word SSID_n=0;
  
  turn_on_LED();
  turn_on_wifi(); 
  
  scan_networks(SSIDs, &SSID_n);
  connect_wifi();   
  boolean success=connect_server(SSIDs, SSID_n);

  turn_off_wifi();
  turn_off_LED();

  if (!success) {
    delay(500);
    warning(2);    
    }
    
  #ifdef USE_DEEP_SLEEP
  deep_sleep(SLEEP_TIME);  // 20 uA for ESP8266
  #else
//  _wdt_enable(2000);
  Serial.print("wait "); Serial.print(SLEEP_TIME); Serial.println(" s");
  delay(SLEEP_TIME*1000);  // 15 mA for ESP8266
  #endif
}

#ifdef USE_DEEP_SLEEP
void deep_sleep(word t) {
char buffer[20];
  #ifdef ESP8266
  sprintf(buffer, "deep sleep %d s", SLEEP_TIME);
  Serial.println(buffer);  
//  delay(t*1000);    // 15.3 mA
  ESP.deepSleep(t*1000000); // is us, 20 uA
  #else
  if (!PowerManagement.safeLock())  {
    sprintf(buffer, "deep sleep %d s", t);
    Serial.println(buffer);  
    PowerManagement.deepsleep(t*1000);  // in ms  // 88 uA with green LED
    }
  else   {
    sprintf(buffer, "delay %d s", t);
    Serial.println(buffer);  
    delay(t*1000);   // 13.1 mA
    }
  #endif
}
#endif

void connect_wifi() {
  if (WiFi.status()!=WL_CONNECTED) {
    Serial.print("Connecting to WiFi: ");
    Serial.print(ssid);    
    #ifdef ESP8266 
//    delay(100);  // necessary for 8266 for normal UART operation
    WiFi.begin(ssid, pass);
    byte i=0;
    while ((WiFi.status() != WL_CONNECTED) && (i<30)) {
      Serial.print(".");
      delay(1000);
      i++;
      }
/*    
    ESP8266WiFiMulti WiFiMulti;
    WiFiMulti.addAP(ssid, pass);
//    Serial.print("Wait for WiFi... ");
    int i=0;
    while ((WiFiMulti.run() != WL_CONNECTED) && (i<20)) {
      Serial.print(".");
      delay(1000);
      }    
*/      
    #else  
    WiFi.begin(ssid, pass);
    byte i=0;
    while ((WiFi.status() != WL_CONNECTED) && (i<20)) {
      Serial.print(".");
      delay(1000);
      i++;
      }
    #endif     
    if (WiFi.status()==WL_CONNECTED) {
      Serial.println(" OK");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
//      Serial.println("delay 500 ms");
//      delay(500);  // necessary!
      }
    else 
      Serial.println(" failed");
    }
}

boolean connect_server(SSID_t *s, word n) {
boolean success=false;  
  if (WiFi.status()==WL_CONNECTED) {
    Serial.print("Connecting to server: ");
    Serial.print(host);
    WiFiClient client;
    boolean rv;
    rv=client.connect(host, port);
    if (rv==false) {
      Serial.print(" Retry");      
      rv=client.connect(host, port);
      }
    if (rv) {      // Use WiFiClient class to create TCP connections
      Serial.println(" OK");
//      const char * tx_data = "GET / HTTP/1.1"; // "Send this data to server";    
//      String tx_data=String("GET /")+" HTTP/1.1\r\n"+"Host: " + host + "\r\n"+"Connection: close\r\n\r\n";
//      String tx_data=String("GET / HTTP/1.1\r\nConnection: close\r\n\r\n");
//      char * tx_data="GET / HTTP/1.1\r\nConnection: close\r\n\r\n";
      Serial.print("TX: ");
      String tx_data;
/*      
      tx_data=String("GET / HTTP/1.1\r\nHost: ") + host + "\r\nConnection: close\r\n\r\n";
      Serial.print(tx_data);
      client.print(tx_data);      // This will send the request to the server
*/
      byte mac[MAC_size];
      tx_data=MAC2str(WiFi.macAddress(mac))+"\r\n";
//      String str(chArray)      
      tx_data=tx_data+String(n)+"\r\n";
//      tx_data=String(n)+"\r\n";
      for (word i=0; i<n; i++) {
        tx_data=tx_data+ String(s[i].SSID)+","+ MAC2str(s[i].BSSID)+","+String(s[i].RSSI)+"\r\n";
//        tx_data=tx_data+String(i+1)+" "+ String(s[i].SSID)+" "+String(s[i].RSSI)+"\r\n";
        }
      Serial.print(tx_data);
      client.print(tx_data);      // This will send the request to the server
        
//      delay(10);
      
      Serial.print("RX: ");      
      while (client.available()) {
        String line = client.readStringUntil('\r');    //read back one line from serve
        Serial.print(line);
        }
      Serial.println("");  
      Serial.println("Closing connection");
      client.stop();
      success=true;
      delay(1);   // necessary for a successful disconnection
      }
    else {
      Serial.println(" failed");
      }
    WiFi.disconnect();
    }  
    
  else {
    Serial.println("WiFi not connected");
    }
  return success;
}

void scan_networks(SSID_t *ssid, word *n) {
  *n=0;
  Serial.print("Scanning networks: ");
  
  int numSsid = WiFi.scanNetworks();   // scan for nearby networks:
  
  // print the list of networks seen:
  Serial.println(numSsid);
  
  if (numSsid < 0) 
    return;

  if (numSsid>=65535)
    numSsid=65535;

  for (word i = 0; i < numSsid; i++) {
//    Serial.print(i);    Serial.print(" ");    Serial.println(*n);    
    SSID_t d;
    word kk;
    String ss=WiFi.SSID(i);
//    ss.toCharArray(d.SSID, SSID_size-1);
//    d.SSID[SSID_size-2]=0;  // null terminated
    ss.toCharArray(d.SSID, SSID_size);
    d.SSID[SSID_size-1]=0;  // null terminated
    byte *p=WiFi.BSSID(i);
    for (byte j=0; j<BSSID_size; j++) d.BSSID[j]=p[j];
    d.RSSI=WiFi.RSSI(i);
    d.encryptionType=WiFi.encryptionType(i);
//    Serial.println(d.SSID);

    for (kk=0; kk<*n; kk++) {
      if (ssid[kk].RSSI<d.RSSI) {
        for (int l=*n-1; l>=(int)kk; l--) {
//        for (int l=*n-1; l>=kk; l--) {  // crash
          if (l<max_SSID-1) 
           ssid[l+1]=ssid[l];  // moving behind
//              memcpy(&SSIDs[l+1], &SSIDs[l], sizeof(SSID_t));
          }
        goto _1; //break;
        }
      }

    _1:
//    k=SSID_n;      
    if (kk<max_SSID)
      ssid[kk]=d;
//    memcpy(&SSIDs[k], &d, sizeof(d));
    if (*n<max_SSID) 
      (*n)++;       
    }
    
  for (word i = 0; i < *n; i++) {
    Serial.print(i+1);
    Serial.print(") ");
    Serial.print(ssid[i].SSID);
    Serial.print("\tMAC:"); 
//    printMAC(SSIDs[i].BSSID);
    Serial.print(MAC2str(ssid[i].BSSID));
    Serial.print("\tSignal:"); 
    Serial.print(ssid[i].RSSI);
    Serial.print(" dBm");
    Serial.print("\tEncryption:");
    printEncryptionType(ssid[i].encryptionType);
    }
}

void printEncryptionType(byte thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
  }
}

String MAC2str(byte *mac) {
/*  
  String s=String(mac[0], HEX)+":"+
    String(mac[1], HEX)+":"+
    String(mac[2], HEX)+":"+
    String(mac[3], HEX)+":"+
    String(mac[4], HEX)+":"+
    String(mac[5], HEX);
  return s;
*/  
  char buffer[18];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buffer);
}
