#include "ViaGprs.h"
#include "ESP32Ping.h"

#define SignalTime 20*60*1000
void logComunicador(const char*);

void ViaGprs::printCredentials(){
  Serial.println("Conectado a:");
  Serial.printf(" - APN: %s\n",apn);  
  Serial.printf(" - Usuario: %s\n",user);  
  Serial.printf(" - Contraseña: %s\n",pass);   
}



void ViaGprs::PrintConexionParameters(){
   // GPRS connection parameters are usually set after network registration
  Serial.println("Parametros de conexion:");
  Serial.println("Configuracion de modem:");
  Serial.println("CCID:"+ modem->getSimCCID());    
  Serial.println("IMEI: " + modem->getIMEI());  
  Serial.println("IMSI: " + modem->getIMSI());  
  Serial.println("Modem info: "+modem->getModemInfo());  
  Serial.println("Modem name: "+modem->getModemName());  
  Serial.println("Local IP: "+modem->getLocalIP());        
}


void ViaGprs::Init(){
    //Inicializo el puerto serie para los comandos AT  
    Serial.println("Inicializando conexion gprs");     
    SerialAT=new HardwareSerial(1);
    SerialAT->begin(115200, SERIAL_8N1, PIN_RX, PIN_TX, false); //Baud, Datos, rx, tx, no se
    modem = new TinyGsm(*SerialAT);
    gsm = new TinyGsmClient(*modem);    
    modem->restart();
}


ViaGprs::ViaGprs(){
    iam=Gprs;    
    Init();
    strncpy(imei,modem->getIMEI().c_str(),30);   
}


void ViaGprs::SetParams(char* apnG,char* userG,char* passG){         
   strncpy(apn,apnG,30);
   strncpy(user,userG,30);
   strncpy(pass,passG,30);    
   apn[strlen(apnG)]='\0';
   user[strlen(userG)]='\0';
   pass[strlen(passG)]='\0';   
}





bool ViaGprs::Connect(){
   Serial.println("Intentando conectar gprs con las credenciales:");   
   Serial.printf(" - APN: %s\n",apn);  
   Serial.printf(" - Usuario: %s\n",user);  
   Serial.printf(" - Contraseña: %s\n",pass);  
  
   if (!modem->isNetworkConnected()){
      Serial.println("Red gprs no conectado, esperando por señal");      
      if (!modem->waitForNetwork()) {
         logComunicador("Fallo al obtener red gprs");
         Serial.printf("Señal gprs:%d\n",getSignal());
      }

      if (!tryCon){                  
         tryCon=3;
         Restart();
      }

      

      if (modem->isNetworkConnected()) { Serial.println("Obtuve red gprs"); }

      Serial.println("Conectando gprs");
      
   }   
   if(IsConnected() || modem->gprsConnect(apn, user,pass)){ 
      Serial.println("Conectado por gprs"); 
      tryCon=3;
   }

   else tryCon--;

   if (!tryCon){
      tryCon=3;
      Restart();
   }

   return IsConnected(); 
}



bool ViaGprs::IsConnected(){   
   return modem->isGprsConnected();
}


bool ViaGprs::LinkLayerConnection(){  
   return !modem->getModemInfo().isEmpty();         
}


void ViaGprs::Disconnect(){
   modem->gprsDisconnect();
}


Client* ViaGprs::getClient(){
   return gsm;
}


int ViaGprs::getSignal(){
   return modem->getSignalQuality();
}


bool ViaGprs::IsVia(CurrentVia v){
   return iam==v;
}

CurrentVia ViaGprs::getViaName(){
   return iam;
}


IPAddress ViaGprs::getIP(){
   return modem->localIP();//modem->TinyGsmIpFromString(modem->getLocalIP);
}


char* ViaGprs::getEventConnect(){
   return recoverGprs;
}

char* ViaGprs::getEventDisconnect(){
   return lostGprs;
}


unsigned long ViaGprs::getSignalTime(){
   return SignalTime;
}


//Libera recursos y los vuelve a crear
bool ViaGprs::Restart(){ 
   logComunicador("Reiniciando modulo gprs");
   delete SerialAT;
   delete modem;
   delete gsm;
   Init();
   return LinkLayerConnection();
}


bool ViaGprs::CheckHosts(const char* h1,const char* h2,uint32_t p){         
   if(!gsm->connect(h1,p))return false;
   gsm->stop();
   if(!gsm->connect(h2,p)) return false;    
   gsm->stop();       
   return true;
}


char* ViaGprs::GetIMEI(){         
   return imei;  
}