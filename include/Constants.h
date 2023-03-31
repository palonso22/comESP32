#ifndef __CONSTANTS__H
#define __CONSTANTS__H


#include <Arduino.h>
//no aumentar este numero porq se va todo a la mierda
#define SIZE_MSG 36
#define SIZE_TOPIC 20
const static char* SoftwareName="Comunicador";
const static  char* SoftwareVersion="1.3.0";

enum Type{
    primary,
    secondary,
    logueo,
    wifiSignal,
    gprsSignal    
};

//evento  base
typedef struct Event {
    Type t;
    char msg[SIZE_MSG];
    void SetType(Type type){
        t=type;
    }
} Event;

//tipo de evento primario
typedef struct EventP:Event{
    EventP(){
        SetType(primary);
    }
};

//tipo de evento secundario
typedef struct EventS:Event{ 
    EventS(){
        SetType(secondary);
    };
};

//tipo de evento de log
typedef struct EventLog:Event{ 
    EventLog(){
        SetType(logueo);
    };
};


//tipo de evento wifi signal
typedef struct EventWifi:Event{ 
    EventWifi(){
        SetType(wifiSignal);
    };
};

//tipo de evento wifi signal
typedef struct EventGprs:Event{ 
    EventGprs(){
        SetType(gprsSignal);
    };
};



enum CurrentVia{
    Wifi,
    Ethernet,
    Gprs,
    None
};


/*
Estados del comunicador
*/

enum GlobalState{
    //inicializacion
    initialize,
    //modeo AP
    APMode,
    //modo chequeo de vias
    checkingVias,
    //modo loop
    pooling    
};



#endif
