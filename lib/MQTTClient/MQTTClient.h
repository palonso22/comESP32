#ifndef __MQTT_H__
#define __MQTT_H__


#include <PubSubClient.h>
#include "../../include/Constants.h"

#define SIZE_TOPIC 30 //tamaño del tópico
#define SIZE_PAYLOAD_EXTEND 60
#define SIZE_CID_PAYLOAD 11 //tamaño del payload en CID ejemplo E10000000

#define Retain 1
#define QOS1 1
#define StOFFLINE "TOFFLINE" //mensaje LWT
#define StONLINE "TONLINE" //mensaje cuando conecta a MQTTClient
#define KeepAliveGPRS 10*60 // keepalive de MQTTClient en segundos
#define KeepAliveWIFI 30*60 // keepalive de MQTTClient en segundos
#define ReconnectMQTTTime 5000 //tiempo que intento reconectar a MQTTClient


#define WiFiSignalTime 60000*20 //Cada 30 minutos se publica la intensidad de la señal wifi,gprs
#define GPRSSignalTime 60000*30 //Cada 30 minutos se publica la intensidad de la señal wifi,gprs

#define MAX_TIME 20*1000
#define SIZE_HOST 40



// Parámetros seteados desde el access point
/*extern char
* mqttHost1;
extern char* mqttHost2;
extern char* mqttPort;
extern char* mqttBrokerPass;
extern char* business;
extern char* account;*/




class MQTTClient : public PubSubClient {
public:
    MQTTClient();    
    void setupMQTT(String mqttServer1,String mqttServer2, String mqttPortString, String mqttBrokPass);
    bool Connect(unsigned);
    bool IsConnected();
    bool Reconnect();    
    void handle();    
    #ifdef useEth
    void mqttEthernetReconnect();
    #endif            
    void Publish(Event);   
    void Disconnect();    

    //Deterina si el último mensaje enviado por EP se envío correctamente
    // si pasa un tiempo preestablecido lo reenvía    
    bool LastMsgSended();
    void SetClient(Client*,CurrentVia);
    Client* GetClient();    
    
    //Determina si ah pasado una cantidad de tiempo MAX_TIME desde que se publico 
    //el ultimo mensaje por EP
    
private:
    Client* client;
    void PublishWIFISignal(char*);    
    void PublishGPRSSignal(char*);
    bool PublishLog(const char*);

    #if defined(ESP8266)
    void mefStatusNoReconnect(); //Estado de led que indica la falta de conexion vía mqtt
    #endif
    void setupTopics();
    void buildTopic(char*,char*,char*,const char*);    
    static const char* GiveMeHost(); 
    char ClientID[20];

    bool logsOn = 0;

    //CONTADORES PARA RECONECTAR CON EL BROKER EN CASO DE PERDERSE LA CONEXIÓN//
    unsigned long mqttPreviousTime = 0;
    unsigned long mqttCurrentTime = 0;
    
    static bool index;
    uint16_t port;

    //Usados por setUpTopics para poder colocar la empresa y subscriptor en los tópicos
    char topicST[20];//Topico XXX/NNNN/ST
    static char topicC[20];//Topico XXX/NNNN/C
    char topicES[20];//Topico XXX/NNNN/ES
    char topicEP[20];//Topico XXX/NNNN/EP
    char topicWIFI[20];//Topico XXX/NNNN/wifi
    char topicGPRS[20];//Topico XXX/NNNN/gprs
    char topicLOG[20];//Topico XXX/NNNN/Log
    char topicVersion[20];//Topico XXX/NNNN/Log
    char topicInfo[20];//Topico XXX/NNNN/Log
    char topicStateVias[20];//Topico XXX/NNNN/Log    
    char topicCurrentVia[20];//Topico XXX/NNNN/Log
    char topicCurrentHostMQTT[20];//Topico XXX/NNNN/Log
    
    bool publishSucceedPub;
    //Estos atributos son usadas por una funcion estática, en consecuencia deben ser declarados estáticos
    //La función estática es la callback
    static bool flagNewPayload;
    static String arrivedPayload;
    static String arrivedTopic;
    static String LOGpublish;

    #ifdef useEth
    byte noReconect=0;
    #endif
    
    byte serverActivo = 0;
    byte reconFallida = 0;
    
    static void onMessage(char*,uint8_t*,unsigned int);
    static char lastEvent[20];
    static unsigned long time;   
    static bool msgSended; 
    static bool IsPrimaryTopic(const char*);
    static void CheckMsgSended(const char*);
    void PublishEP(char*);
    void PublishES(char*);
    void PublishLog(char*);
    void PublishVersion(char*);
    void PublishInfo(char*);
    void PublishCtVia(char*);
    void PublishCtHost(char*);
    void PublishStVias(char*);
    bool trySend;

    static void execCommand(const char*);

};

#endif 