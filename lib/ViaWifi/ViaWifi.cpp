#include "ViaWifi.h"
#include <IPAddress.h>
#include "ESP32Ping.h"

#define SignalTime 30*60*1000;

void ViaWifi::PrintConexionParameters(){
    Serial.println("Parametros de conexion:");    
    Serial.println("DNS:"+String(WiFi.dnsIP()));            
    Serial.println("Gateway:"+String(WiFi.gatewayIP()));                    
    Serial.println("Máscara de subred:"+String(WiFi.subnetMask()));     
    Serial.println("IP Asignada:"+String(WiFi.localIP()));
}

void ViaWifi::SetParams(char* ssid, char* pass){
    strncpy(_ssid,ssid,20);
    strncpy(_pass,pass,20);    
}

ViaWifi::ViaWifi(){
    iam=Wifi;        
};



bool ViaWifi::CheckHosts(const char* h1,const char* h2,uint32_t p){
    Serial.printf("Chequeando hosts: %s,%s\n",h1,h2);
    Serial.printf("Puerto: %s\n",String(p));
    WiFiClient c;  
    if(!c.connect(h1,p)){
        Serial.println("Error chequeando host 1");
        return false;    
    }
    c.stop();
    if(!c.connect(h2,p)){
        Serial.println("Error chequeando host 2");
        return false;    
    } 
    c.stop();    
    return true;
}





bool ViaWifi::Connect(){    
    Serial.println("Intentando conectar wifi");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(); //Elimino cualquier configuración previa que haya quedado      
    Serial.println("Estableciendo conexion wifi con la siguientes credenciales");
    Serial.printf("SSID:%s\n",_ssid);
    Serial.printf("Password:%s\n",_pass);    
    WiFi.begin(_ssid,_pass);
    bool con=false;
    for(int i=0;i<50&&!con;i++){                
        con|=WiFi.status()==WL_CONNECTED;
        delay(1000);
    }
        
    if (con)PrintConexionParameters();
    return IsConnected();        
}

bool ViaWifi::LinkLayerConnection(){
    return WiFi.status() == WL_CONNECTED ;
}


void ViaWifi::Disconnect(){
      WiFi.disconnect();        
}


//Chequear coneccion a internet mediante una conexion udp
bool ViaWifi::IsConnected(){ 
    bool b=client.connect(PingUrl,PingPort);
    if(b)client.stop();
    return b;         
}

bool ViaWifi::Reconnect(){
    Disconnect();
    return Connect();
}


CurrentVia ViaWifi::getViaName(){
   return iam;
}



bool ViaWifi::IsVia(CurrentVia v){
   return iam==v;
}

IPAddress ViaWifi::getIP(){
    return WiFi.localIP();
}


int ViaWifi::getSignal(){
    return WiFi.RSSI();
}


 Client* ViaWifi::getClient(){
     return &clientMQTT;
 }


 
char* ViaWifi::getEventConnect(){
   return recoverWifi;
}

char* ViaWifi::getEventDisconnect(){
   return lostWifi;
}


unsigned long ViaWifi::getSignalTime(){
   return SignalTime;
}

bool ViaWifi::Restart(){
    return Connect();
}