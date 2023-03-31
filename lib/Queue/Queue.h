#ifndef __QUEUE__H
#define __QUEUE__H


#include <Arduino.h>
#include "../../include/Constants.h"


#define MAX_ELEMENTS 50

#if defined(ESP8266)
#include <RingBufCPP.h>
#endif




//Interfaz para la clase Queue
class Queue{
    public:
         Queue();
         bool Push(Event);
         Event Pop();
         bool IsEmpty();
         UBaseType_t Length();

    private:        
        #if defined(ESP32)
        QueueHandle_t q32;
        #elif defined(ESP8266)
        RingBufCPP<Event, MAX_ELEMENTS> q8266;
        #endif

};


#endif
