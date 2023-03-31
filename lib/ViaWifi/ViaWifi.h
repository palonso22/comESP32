#include "ViaInterface.h"
#include <assert.h>
#include <string.h>

#if defined(ESP32)
#include <WiFi.h>
#include <ESP32Ping.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#endif

#define recoverWifi "R36000000"
#define lostWifi "E36000000"


class ViaWifi: public Via{
    public:
         ViaWifi();         
         bool Connect();
         void Disconnect();
         bool IsConnected(); 
         bool Reconnect();
         bool LinkLayerConnection();
         CurrentVia getViaName();
         int getSignal();
         bool IsVia(CurrentVia);        
         IPAddress getIP();
         Client* getClient();
         char* getEventConnect();
         char* getEventDisconnect();         
         void PrintConexionParameters();
         unsigned long getSignalTime();         
         bool Restart();
         void SetParams(char*,char*);
         bool CheckHosts(const char*,const char*,uint32_t);
    private:
         char _ssid[20];
         char _pass[20];    
         const char* PingUrl = "www.google.com.ar";  
         u_int PingPort=80;
         WiFiClient clientMQTT;
         WiFiClient client; 
         CurrentVia iam;      
        
};


