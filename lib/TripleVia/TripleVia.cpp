
#include "TripleVia.h"
#include "AP.h"
#include "dscAdapter.h"
#include "ProcessCID.h"
#include "MQTTClient.h"
#include "Queue.h"
#include "ViaWifi.h"
#include "ViaGprs.h"
#include <tuple>


#define TRY_CON_VIA 3
#define TRY_CON_MQTT 3
#define STACK_DEPH 4096
#define TIME_CON_MQTT 20*1000
#define Core0 0
#define Core1 1



const char* getStringVia(CurrentVia);
std::pair<SemaphoreHandle_t*,bool*> getComponents(CurrentVia);


//parametros de conexion
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
bool useWifi;
bool useGprs=false;
bool flagLog=false;

//Estado global del comunicador
GlobalState state;

//Vias
ViaGprs* gprs;
ViaWifi* wifi;



CurrentVia currentVia = None;

//cliente mqtt;
MQTTClient* mqttClient;;

//panel
dscAdapter dsc(dscClockPin, dscReadPin, dscWritePin);





///cola de eventos
Queue* queue;

//TAREAS
TaskHandle_t TaskHandleLedIndicator;
TaskHandle_t TaskSetupLed;
TaskHandle_t TaskScheduler;
TaskHandle_t TaskWifi;
TaskHandle_t TaskMqtt;
TaskHandle_t TaskGprs;
TaskHandle_t TaskReadDSC;
TaskHandle_t TaskHandleInterfaces;

//Mutexs
SemaphoreHandle_t xVia;
SemaphoreHandle_t xWifiOn;
SemaphoreHandle_t xEthOn;
SemaphoreHandle_t xGprsOn;
SemaphoreHandle_t xMqttOn;
SemaphoreHandle_t xDSC;
SemaphoreHandle_t xState;
SemaphoreHandle_t xFlags;


//Funcion para cambiar el estado del comunicador
void ChangeState(GlobalState s){
    if(xSemaphoreTake(xState,portMAX_DELAY)==pdTRUE){
        state=s;
        xSemaphoreGive(xState);
    }
}

//Crear eventos de log
void logComunicador(const char* msg){
    Serial.println(msg);
    if(flagLog){
        EventLog e;
        int ctos=strlen(msg);
        strncpy(e.msg,msg,ctos);
        e.msg[ctos]='\0';        
        queue->Push(e);
    }	
}


// orden de prioridad: eth,wifi,gprs
bool wifiON = false;
bool ethON = false;
bool gprsON = false;
bool mqttON = false;
bool initON = false;
//Este arreglo sirve para llevar el estado de conexion de cada via
//La primer posicion corresponde a la via wifi (arranca en true)
//La segunda posicion corresponde a la via gprs (arranca en true)
//La tercer posicion corresponde a la comunicacion mqtt (arranca en false)
bool comunicadorStatus[]={true,true,false};

void ChangeConStatus(CurrentVia v,bool status){
    /*
    Almacena el estado de conexion a internet de la via v 
    */
    if (xSemaphoreTake(xFlags,portMAX_DELAY)==pdTRUE){
        switch (v)
        {            
            case Wifi://conexion
                comunicadorStatus[0]=status;                
                break;
            case Gprs:                
                comunicadorStatus[1]=status;                
                break;
            /*case Ethernet:                
                comunicadorStatus[2]=status;                
                break;        */
            default://mqtt
                comunicadorStatus[2]=status;                
                break;
        }
        xSemaphoreGive(xFlags);
    }    
}


void printParameters(bool useWifi,bool useGprs){
    Serial.println("Parametros leidos:");
    Serial.printf("Host 1:%s\n",mqttHost1);
    Serial.printf("Host 2:%s\n",mqttHost2);
    Serial.printf("Pass broker:%s\n",mqttPass);
    Serial.printf("Puerto:%s\n",mqttPort);
    if (useWifi){
        Serial.printf("wifi ssid:%s\n",wifiSSID);
        Serial.printf("Pass wifi:%s\n",wifiPASS);        
    }

    if (useGprs){
        Serial.printf("gprs apn:%s\n",gprsAPN);
        Serial.printf("gprs user:%s\n",gprsUSER);        
        Serial.printf("gprs pass:%s\n",gprsPASS);        
    }    
}


//convertir de CurrentVia a Type
Type fromViaToType(CurrentVia v){
    switch(v){
        case Gprs:
          return gprsSignal;
          break;
        case Wifi:
          return wifiSignal;
          break;                                            
    }
}





void sendSignal(Via* v){    
    if (v!=nullptr){        
        Event e;
        Type t = fromViaToType(v->getViaName());     
        e.SetType(t);
        sprintf(e.msg,"%d",v->getSignal());
        queue->Push(e);
    }    
}

//publica señal de la via cada cierto periodo
void sendSignalPeriod(Via* v){
    static unsigned long last = 0; 
    unsigned long now = millis();
    if (now-last> v->getSignalTime()){
        last = now;         
        sendSignal(v);
    }
}


void sender();    
//Control de mqtt, solo llamar cuando la via esta activa
 bool ControlMQTT(){          
    boolean on=mqttClient->IsConnected();                                
    ChangeConStatus(None,on);                            
    // mqtt desconectado me conecto
    if (!on){          
        logComunicador("Desconectado mqtt,intentando conectar");                
        //El keepAlive depende de la via en la que se encuentre
        //comunicando        
        if(xSemaphoreTake( xVia, portMAX_DELAY) == pdTRUE){               
            on=mqttClient->Connect(currentVia==Wifi?KeepAliveWIFI:KeepAliveGPRS);
            xSemaphoreGive( xVia );                
        }        
        
        //El delay es para evitar dos conexiones consecutivas
        delay(5000);        
    }
    else{        
        mqttClient->loop();
        sender();    
    }              
    return on;
}           

/*Devuelve el correspondiente mutex y booleano para la via*/
std::pair<SemaphoreHandle_t*,bool*> getComponents(CurrentVia v){    
    switch(v){
        case Wifi:
            return std::make_pair(&xWifiOn,&wifiON);
            break;
        case Gprs:
            return std::make_pair(&xGprsOn,&gprsON);
            break;
        case Ethernet:
            //return std::make_pair(&xWifiOn,&wifiON);
            break;        
    }        
}




/*Algoritmo de control de via*/
void taskVia(void* p){        
    Via* via = (Via*)p;
    bool on = false;   
    bool lastValue = false,isVia;    
    CurrentVia id = via->getViaName();    
    std::pair<SemaphoreHandle_t*,bool*> comps = getComponents(id); 
    SemaphoreHandle_t* mutex = comps.first;
    bool* state = comps.second;
    EventP e;
    const char* viaName= getStringVia(id);
    char buffer[SIZE_MSG];  
    snprintf(buffer,SIZE_MSG,"Iniciando via %s",viaName);
    logComunicador(buffer);
    byte tryConVia=TRY_CON_VIA;
    byte tryConMQTT=TRY_CON_MQTT;
    while(true){     
        lastValue=on;        
        on=via->IsConnected();
        ChangeConStatus(via->getViaName(),on);
        sendSignalPeriod(via);            
        //pasa de desconectado a conectado
        if (!lastValue&&on){    
            via->PrintConexionParameters();                                
            strncpy(e.msg,via->getEventConnect(),SIZE_MSG);
            queue->Push(e);
            snprintf(buffer,SIZE_MSG,"Conecto via %s",viaName);
            logComunicador(buffer);             
            tryConVia=TRY_CON_VIA;       
        }

        //pasa de conectado a desconectado
        if (lastValue&&!on){
            strncpy(e.msg,via->getEventDisconnect(),SIZE_MSG);
            snprintf(buffer,SIZE_MSG,"Desconecto via %s",viaName);
            logComunicador(buffer);
            queue->Push(e);            
        }       

        // tomar el mutex solo cuando haya un cambio de estado
        if (on != lastValue && xSemaphoreTake( *mutex, portMAX_DELAY) == pdTRUE ){                
                snprintf(buffer,SIZE_MSG,"Hay un cambio de estado en %s",viaName);
                logComunicador(buffer);
                *state = on;
                xSemaphoreGive(*mutex);
                sendSignal(via);           
        }
        // via con salida a internet
        if(on){              
            isVia=false;   
            if(xSemaphoreTake( xVia, portMAX_DELAY) == pdTRUE){    
                isVia=via->IsVia(currentVia);
                xSemaphoreGive( xVia );                
            } 
            if(isVia){                                          
                if(!ControlMQTT()){                           
                        //si mqtt intenta conectarse mas de 3 veces y no puede
                        // prueba a desconectar y volver a conectar                           
                        tryConMQTT--;
                        if(!tryConMQTT){
                            snprintf(buffer,SIZE_MSG,"Desconectando via %s",viaName);
                            logComunicador(buffer);
                            via->Disconnect();                                 
                            tryConMQTT=TRY_CON_MQTT;                            
                        }
                }                
            }                
                                                          
        }     
        else{//Desconectado            
            snprintf(buffer,SIZE_MSG,"Desconectado via %s",viaName);
            logComunicador(buffer);
            via->Connect();           
            tryConVia--;            
            if(!tryConVia){
                snprintf(buffer,SIZE_MSG,"Reiniciando via %s",viaName);
                via->Restart();   
                tryConVia=TRY_CON_VIA;
            }                                                                         
        }      
    }        
}




/*
Tarea encargada de leer eventos del panel y encolarlos
*/
void taskReadDSC(void *p){
    logComunicador("Inicializa task reader");
    while(true){
        //LECTURA PANEL
		if(xSemaphoreTake( xDSC,portMAX_DELAY) == pdTRUE){            
			dsc.ReaderDSC();                        
			xSemaphoreGive( xDSC );
		}        
		gpio_loop();
        pcid_loop();
        yield();        
    }
    vTaskDelete(TaskReadDSC);
}

const char* getStringVia(CurrentVia v){       
    switch(v){
        case Wifi:
            return "Wifi";
            break;
        case Gprs:
            return "Gprs";
            break;
        case Ethernet:
            return "Ethernet";
            break;
        case None:
            return "None";
            break;
    }    
    return "error";
}


/*
Setea la actual via de comunicacion, none quiere decir que no hay salida a internet por ninguna via
*/
void setCurrentVia(CurrentVia v,Client* c){
    char buffer[SIZE_MSG];
    if(xSemaphoreTake( xVia, portMAX_DELAY ) == pdTRUE){
        if(currentVia!=v){                                    
            snprintf(buffer,SIZE_MSG,"Cambiando de via %s a via %s",getStringVia(currentVia),getStringVia(v));
            logComunicador(buffer);
            currentVia=v;            
            if(v!=None)mqttClient->SetClient(c,v);        
        }                          
        xSemaphoreGive( xVia );
    }

}


/*
Control de seleccion de vias
*/
void taskScheduler(void*p){
    logComunicador("Inicializa scheduler");    
    bool state[3];    
    while(true){  
            for(int i=0;i<3;i++)state[i]=false;                          
            if(xSemaphoreTake( xEthOn, portMAX_DELAY ) == pdTRUE){
                state[0] = ethON;
                xSemaphoreGive( xEthOn );
            }            
            if(xSemaphoreTake( xWifiOn, portMAX_DELAY) == pdTRUE){
                state[1]=wifiON;
                xSemaphoreGive( xWifiOn );
            }

            if(xSemaphoreTake( xGprsOn, portMAX_DELAY ) == pdTRUE){
                state[2] = gprsON;
                xSemaphoreGive( xGprsOn );
            }             

            if (state[0]){// eth tiene salida a internet
               //setCurrentVia(Ethernet);
            }
            else if (state[1]){//wifi tiene salida a internet                                
                setCurrentVia(Wifi,wifi->getClient());
            }
            else if (state[2]){//gprs tiene salida a internet y wifi no
                setCurrentVia(Gprs,gprs->getClient());                
            }
            else{//Ninguna via disponible
                setCurrentVia(None,nullptr);
            }        
    }
}









void setupTripleVia(){
    Serial.println("Inicia setup");    
    //crear semaforos
    xVia = xSemaphoreCreateMutex();    
    xMqttOn = xSemaphoreCreateMutex();
    xWifiOn = xSemaphoreCreateMutex();
    xEthOn = xSemaphoreCreateMutex();
    xGprsOn = xSemaphoreCreateMutex();
    xDSC = xSemaphoreCreateMutex();
    xMqttOn=xSemaphoreCreateMutex();    
    xState=xSemaphoreCreateMutex();    
    xFlags=xSemaphoreCreateMutex();    
    //Indicar inicializacion
    ChangeState(initialize);    
    //Inicializo GPIO
    gpio_setup();     
    //Control de led
    xTaskCreatePinnedToCore(taskLedIndicator,"Task_setupLed",STACK_DEPH,nullptr,1,&TaskSetupLed,Core1);         

    //Montar FS
    if(!InitFS()){
        logComunicador("Error montando FS");
        Serial.println("Error montando FS");
        ESP.restart();
    }
    Serial.println("FS montado con exito");
    
     //Creo la cola
    queue=new Queue(); 

     //Inicializa pcid y dsc
    pcid_setup(dsc);
    dsc.setupDSC();

    //Controla lectura del panel   
    xTaskCreatePinnedToCore(taskReadDSC,"Task_ReadDSC",STACK_DEPH,NULL,1,&TaskReadDSC,Core1);         
    
    //Crear via gprs
    gprs=new ViaGprs();    

    //Crear via wifi
    wifi = new ViaWifi();
    
    //detectar si hay un modulo gprs conectado
    useGprs=gprs->LinkLayerConnection();  
    logComunicador(useGprs?"Gprs detectado":"Gprs no detectado");
    if(!checkParams()){
        logComunicador("Error en lectura de parametros,iniciando AP");
        InitAP();
    }

    printParameters(useWifi,useGprs);

    //Inicia chequeo de vias
    ChangeState(checkingVias);

    auto checkConn=[](Via* v){  
        CurrentVia id =  v->getViaName();
        const char* viaName=getStringVia(id);
        Serial.printf("Probando via %s\n",viaName);
        bool con=v->Connect()&&v->IsConnected();
        if(con)Serial.printf("Chequeo de via %s exitoso\n",viaName);
        else Serial.printf("Fallo chequeo de via %s\n",viaName);                     
        return con;
    };


    bool initSuccessfull=true;
    //Probar conexion wifi
    if (useWifi){
        wifi->SetParams(wifiSSID,wifiPASS);        
        initSuccessfull&=checkConn(wifi)&&wifi->CheckHosts(mqttHost1,mqttHost2,atoi(mqttPort));
    }
    else {
        logComunicador("Descartando via wifi");
        delete wifi;
        wifi=nullptr;
    }
    

    //Probar conexion gprs
    if (useGprs){
        gprs->SetParams(gprsAPN,gprsUSER,gprsPASS);
        if(initSuccessfull){
            initSuccessfull&=checkConn(gprs);
            if(!useWifi)initSuccessfull&=gprs->CheckHosts(mqttHost1,mqttHost2,atoi(mqttPort));
        }        
    }
    else{
        Serial.println("Descartando via gprs");
        delete gprs;
        gprs=nullptr;
    }
    
            
    //Si falla al inicializar,levantar AP
    if (!initSuccessfull){
        Serial.println("Inicializacion incorrecta");
        InitAP();
    }

    //Creo el cliente mqtt
    mqttClient=new MQTTClient();     

    if(useWifi){
        //Administracion de wifi
        xTaskCreatePinnedToCore(taskVia,"Task_Wifi",STACK_DEPH,wifi,1,&TaskWifi,Core1);                                     
    }

    if(useGprs){
        //Administracion de gprs
        xTaskCreatePinnedToCore(taskVia,"Task_Gprs",STACK_DEPH,gprs,1,&TaskGprs,Core1);      
    }   
           
            
   
    //Administracion de vias
    //xTaskCreatePinnedToCore(,"Task_Scheduler",STACK_DEPH,NULL,1,&TaskScheduler,Core1);      
    
            
    logComunicador("Comunicador inicializado");
    ChangeState(pooling);
}


// si hay un mensaje pendiente sin enviar intenta reenviarlo
// sino mira en la cola si hay pendientes   
void sender(){
    if (mqttClient->LastMsgSended() && !queue->IsEmpty()){        
        Event e = queue->Pop();        
        mqttClient->Publish(e); 
    }            
}



	