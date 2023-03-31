#include "MQTTClient.h"
#include "GPIO.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "ProcessCID.h"
#include "../../include/Comandos.h"
#include "../../include/ContactID.h"
#include "AP.h"
#include "ViaWifi.h"
#include "ViaGprs.h"
#include "Queue.h"

void logComunicador(const char*);
const char* getStringVia(CurrentVia);

// subtopicos
const char* subTopicST = "ST";
const char* subTopicEP = "EP";
const char* subTopicES = "ES";
const char* subTopicC = "C";
const char* subTopicLOG = "LOG";
const char* subTopicWIFI =  "WIFI";
const char* subTopicGPRS = "GPRS";
const char* subTopicVersion = "VERSION";
const char* subTopicInfo = "INFO";
const char* subTopicStateVias = "STATEVIAS";
const char* subTopicCurrentVia = "CURRENTVIA";
const char* subTopicCurrentHost = "CURRENTHOST";
const char* subTopicCurrentHosts = "HOSTS";

extern char wifiSSID[MAX_SIZE_P];
extern char wifiPASS[MAX_SIZE_P];
extern char gprsAPN[MAX_SIZE_P];
extern char gprsPASS[MAX_SIZE_P];
extern char gprsUSER[MAX_SIZE_P];
extern char mqttHost1[MAX_SIZE_P];
extern char mqttHost2[MAX_SIZE_P];
extern char mqttPort[MAX_SIZE_P];
extern char mqttPass[MAX_SIZE_P];
extern char business[MAX_SIZE_P];
extern char account[MAX_SIZE_P];






extern bool flagLog;
extern ViaWifi* wifi;
extern ViaGprs* gprs;
extern bool useWifi;
extern bool useGprs;
extern bool wifiON;
extern bool gprsON;
extern SemaphoreHandle_t xVia;
extern SemaphoreHandle_t xWifiOn;
extern SemaphoreHandle_t xGprsOn;
extern CurrentVia currentVia;
extern Queue* queue;

bool MQTTClient::flagNewPayload=0;
String MQTTClient::arrivedPayload;
String MQTTClient::arrivedTopic;
String MQTTClient::LOGpublish;

char MQTTClient::topicC[20];
bool MQTTClient::index;
static String eventDSCrx;
extern String eventCID;




char MQTTClient::lastEvent[20];
bool MQTTClient::msgSended;
unsigned long MQTTClient::time;




MQTTClient::MQTTClient(){    
    Serial.println("Creando mqtt client");
    msgSended=true;
    lastEvent[0]='\0';
    port=atoi(mqttPort);
     //Tiempo en el cual le va a mandar un pinrequest al broker para que el mismo sepa si sigue conectado este cliente
    setCallback(onMessage);
    Serial.printf("mqtthost1:%s\n",mqttHost1);
    Serial.printf("mqtthost2:%s\n",mqttHost2);       
    setupTopics();    
    index=random(2);
}


bool MQTTClient::IsConnected(){
    return connected();        
}


const char* MQTTClient::GiveMeHost(){    
    index=!index;        
    return index?mqttHost1:mqttHost2;
}
/*
CONECTA CON EL BROKER
*/
bool MQTTClient::Connect(unsigned keepAlive) {               
    logComunicador("Intentando conexion MQTT");
    char buffer[SIZE_MSG];    
    snprintf(buffer,SIZE_MSG,"Por via:%s",getStringVia(currentVia));    
    logComunicador(buffer);    
    snprintf(buffer,SIZE_MSG,"Con host:%s",GiveMeHost());    
    logComunicador(buffer);
    snprintf(buffer,SIZE_MSG,"Y puerto :%u",port);        
    logComunicador(buffer);
    setServer(GiveMeHost(),port);      
    snprintf(buffer,SIZE_MSG,"Seteando keepAlive:%d",keepAlive);    
    logComunicador(buffer);
    setKeepAlive(keepAlive);                  
    snprintf(buffer,SIZE_MSG,"Cliente:%s Pass:%s",ClientID,mqttPass);                
    logComunicador(buffer);
    if (connect(ClientID,ClientID,mqttPass,topicST,QOS1,Retain,StOFFLINE)) {        
        logComunicador("Conexion MQTT exitosa");
        //publicar online con retain
        publish(topicST,StONLINE,true);
        subscribe(topicC,QOS1);        
        subscribe(topicEP,QOS1);       
    }
    else {
        logComunicador("Fallo de conexion MQTT");
    }    
    return IsConnected();
}
/*
SETEO DE TÓPICOS
*/
void MQTTClient::setupTopics(){    

    // ID para el broker
    ClientID[0]='\0';
    strncat(ClientID,business,strlen(business));
    strncat(ClientID,account,strlen(account));
    // Tópicos de pub y sub
    //construir topicos
    buildTopic(topicST,business,account,subTopicST);// XXX/NNNN/ST
    buildTopic(topicEP,business,account,subTopicEP);// XXX/NNNN/SP
    buildTopic(topicES,business,account,subTopicES);// XXX/NNNN/ES    
    buildTopic(topicC,business,account,subTopicC);// XXX/NNNN/LOG
    buildTopic(topicLOG,business,account,subTopicLOG);// XXX/NNNN/
    buildTopic(topicWIFI,business,account,subTopicWIFI);// XXX/NNNN/WiFi
    buildTopic(topicGPRS,business,account,subTopicGPRS);// XXX/NNNN/gprs
    buildTopic(topicVersion,business,account,subTopicVersion);// XXX/NNNN/gprs
    buildTopic(topicInfo,business,account,subTopicInfo);// XXX/NNNN/gprs
    buildTopic(topicStateVias,business,account,subTopicStateVias);// XXX/NNNN/gprs
    buildTopic(topicCurrentVia,business,account,subTopicCurrentVia);// XXX/NNNN/gprs    
    buildTopic(topicCurrentHostMQTT,business,account,subTopicCurrentHost);// XXX/NNNN/gprs        
}


void MQTTClient::buildTopic(char*topic,char* business,char* account,const char* subTopic){    
    topic[0]='\0';
    strncat(topic,business,strlen(business));    
    strncat(topic,"/",1);    
    strncat(topic,account,strlen(account));    
    strncat(topic,"/",1);    
    strncat(topic,subTopic,strlen(subTopic));   
}


 
void sendSignal(Via*);
const char* getStringVia(CurrentVia);
void MQTTClient::execCommand(const char* comm){   
    EventLog e; 
    if(!strcmp(comm,Version)){                
        strncpy(e.msg,SoftwareName,SIZE_MSG);        
        queue->Push(e);        
        strncpy(e.msg,SoftwareVersion,SIZE_MSG);        
        queue->Push(e);
    }

    else if(!strcmp(comm,logActive)){
        flagLog=true;        
    }

    else if(!strcmp(comm,logDesactive)){
        flagLog=false;        
    }    
    else if(!strcmp(comm,signalGprs)){
        sendSignal(gprs);
    }
    else if(!strcmp(comm,signalWifi)){
        sendSignal(wifi);
    }
    else if(!strcmp(comm,StVias)){         
        if(useWifi){
            if(xSemaphoreTake( xWifiOn, portMAX_DELAY) == pdTRUE){
                snprintf(e.msg,SIZE_MSG,"Wifi:%d",wifiON);
                queue->Push(e);
                xSemaphoreGive( xWifiOn );                
                
            }            
        }

        if(useGprs){
            if(xSemaphoreTake( xGprsOn, portMAX_DELAY) == pdTRUE){
                snprintf(e.msg,SIZE_MSG,"Gprs:%d",gprsON);
                queue->Push(e);
                xSemaphoreGive( xGprsOn);                                
            }            
        }                   
    }
    else if(!strcmp(comm,CtVia)){          
          if(xSemaphoreTake( xVia, portMAX_DELAY) == pdTRUE){                           
            strncpy(e.msg,getStringVia(currentVia),SIZE_MSG);
            xSemaphoreGive( xVia );                
            queue->Push(e);
        }                
    }

    else if(!strcmp(comm,CtHost)){                        
        strncpy(e.msg,index?mqttHost1:mqttHost2,SIZE_MSG);          
        queue->Push(e);
    }

    else if(!strcmp(comm,Info)){                        
        snprintf(e.msg,SIZE_MSG,"Host1:%s",mqttHost1);
        queue->Push(e);
        snprintf(e.msg,SIZE_MSG,"Host2:%s",mqttHost2);        
        queue->Push(e);
        snprintf(e.msg,SIZE_MSG,"Puerto:%s",mqttPort);
        queue->Push(e);
        snprintf(e.msg,SIZE_MSG,"Business:%s",business);
        queue->Push(e);
        snprintf(e.msg,SIZE_MSG,"Account:%s",account);
        queue->Push(e);

        if (useWifi){            
            snprintf(e.msg,SIZE_MSG,"WifiSSID:%s",wifiSSID);
            queue->Push(e);
            snprintf(e.msg,SIZE_MSG,"WifiPASS:%s",wifiPASS);
            queue->Push(e);
        }

        if (useGprs){            
            snprintf(e.msg,SIZE_MSG,"GprsAPN:%s",gprsAPN);
            queue->Push(e);
            snprintf(e.msg,SIZE_MSG,"GprsUSER:%s",gprsUSER);
            queue->Push(e);
            snprintf(e.msg,SIZE_MSG,"GprsPASS:%s",gprsPASS);            
            queue->Push(e);
            snprintf(e.msg,SIZE_MSG,"GprsIMEI:%s",gprs->GetIMEI());            
            queue->Push(e);
        }    
        
    }    

    else{
        pcid_recibComand(comm);    
        pcid_transmitComand();    
    }
  
    
}



/*
FUNCIÓN CALLBACK QUE RECIBE COMANDOS, CUANDO ES CONVOCADO
*/
void MQTTClient::onMessage(char* topic, byte* bytePayload, unsigned int length){
     if(length > 9) {
        Serial.printf("Advertencia:el tamaño del payload es %d\n",length);
        return;
    }           
    char* payload = (char*) bytePayload;
    payload[length]='\0'; 
    //si el topico es EP, chequear si coincide con el último mensaje
    //enviado       
    if (IsPrimaryTopic(topic)){
        CheckMsgSended(payload);
    }
    Serial.printf("Nuevo mensaje mqtt->Topico:%s Mensaje:%s\n",topic,payload);                       
    char buffer[SIZE_MSG];
    snprintf(buffer,SIZE_MSG,"Llega por topico %s",topic);
    logComunicador(buffer);
    snprintf(buffer,SIZE_MSG,"Mensaje %s",payload);
    logComunicador(buffer);
    arrivedTopic = topic;    
    flagNewPayload=1;// Cada vez que llega un mensaje llamo a transmitCommand() para ejecutar el comando sobre el panel        
    if(!strcmp(topic,topicC)){                
        Serial.printf("Ejecutando comando %s\n",payload);      
        execCommand(payload);
    } 
}

bool MQTTClient::IsPrimaryTopic(const char* topic){
    char* a=strchr(topic,'/');
    a=strchr(a+1,'/');
    return strncmp(a+1,subTopicEP,2)==0;            
}


void MQTTClient::CheckMsgSended(const char* m){    
    msgSended=strncmp(m,lastEvent,SIZE_MSG)==0;
    if (msgSended){
        char buffer[SIZE_MSG];
        snprintf(buffer,SIZE_MSG,"Ack:%s",m);
        logComunicador(buffer);
    }
}



void MQTTClient::Publish(Event e){
    // chequear que tipo de evento es    
    switch (e.t)
    {
    case primary:        
        PublishEP(e.msg);
        break;

    case secondary:        
        PublishES(e.msg);
        break;

    case logueo:        
        PublishLog(e.msg);
        break;
    
    case wifiSignal:
        PublishWIFISignal(e.msg);
        break;
    case gprsSignal:
        PublishGPRSSignal(e.msg);    
        break;            
    default:
        break;
    }

}


void MQTTClient::PublishEP(char* e){
    char buffer[SIZE_MSG];  
    int ctos = strlen(e); 
    snprintf(buffer,SIZE_MSG,"Publicando %s",e);
    logComunicador(buffer);
    strncpy(lastEvent,e,ctos); 
    lastEvent[ctos]='\0';
    time=millis(); 
    msgSended=false; 
    trySend=true;
    publish(topicEP,lastEvent);  
    
}

void MQTTClient::PublishES(char* e){
    Serial.printf("Publicando evento secundario:%s\n",e);
    publish(topicES,e);
}

void MQTTClient::PublishLog(char* e){    
    publish(topicLOG,e);
}


/*
Publica el nivel de intensidad wifi cada 1/2 hora
*/
void MQTTClient::PublishWIFISignal(char* msg){        
    publish(topicWIFI,msg);         
}


/*
Publica el nivel de intensidad gprs cada 20 minutos
*/
void MQTTClient::PublishGPRSSignal(char* msg){        
    publish(topicGPRS,msg);         
}


void MQTTClient::PublishVersion(char* msg){        
    publish(topicVersion,msg);         
}

void MQTTClient::PublishInfo(char* msg){        
    publish(topicInfo,msg);         
}

void MQTTClient::PublishStVias(char* msg){        
    publish(topicStateVias,msg);         
}


void MQTTClient::PublishCtVia(char* msg){        
    publish(topicCurrentVia,msg);         
}

void MQTTClient::PublishCtHost(char* msg){        
    publish(topicCurrentHostMQTT,msg);             
}




void MQTTClient::Disconnect(){
    disconnect();
}

bool MQTTClient::LastMsgSended(){
    //si el mensaje no se envío en MAX_TIME segundos volver a publicar
    if (!msgSended && millis()-time>MAX_TIME){       
        //Se intenta enviarlo una vez,sino se reinicia conexion        
        if (trySend){
            logComunicador("Mensaje no enviado,desconecto");
            trySend=false;
            Disconnect();            
            time=millis();
        }
        else{
            PublishEP(lastEvent);                   
        }        
    }         
    return msgSended;
}


Client* MQTTClient::GetClient(){
    return client;
}

void MQTTClient::SetClient(Client* c,CurrentVia v){
    if(!c)logComunicador("Cliente nulo");
    char buffer[SIZE_MSG];
    snprintf(buffer,SIZE_MSG,"Cambiando a cliente:%S",getStringVia(v));
    Serial.println();
    client=c;
    setClient(*client);
}






#if defined(ESP8266)
/*
Indicación de pérdida de comunicación MQTTClient mediante LED para la ESP8266
*/
void MQTTClient::mefStatusNoReconnect(){
    static byte cont = 0;
    static byte ledOn = 0; //Conmienza con el LED apagado
    while(cont < 16){
        if(ledOn){
            digitalWrite(LED_BUILTIN, HIGHLED);
            ledOn = 0;
                delay(100);
        }
        else{
            digitalWrite(LED_BUILTIN, LOWLED);
            ledOn = 1;
                delay(100);
            }
        cont ++;
    }
    cont = 0;
    digitalWrite(LED_BUILTIN, HIGHLED);
}
#endif