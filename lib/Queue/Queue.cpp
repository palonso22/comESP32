#include "Queue.h"




Queue::Queue(){    
    #if defined(ESP32)    
    Serial.println("Creando cola para esp32");
    q32 =xQueueCreate(200,sizeof(Event));
    if(q32==nullptr)Serial.println("Error al crear cola para esp32");
    #endif
    #if defined(ESP8266)
    Serial.println("Creando cola para esp8266"):
    q8266  = RingBuf_new(sizeof(Event), MAX_ELEMENTS);
    if(q8266==nullptr)Serial.println("Error al crear cola para esp8266");
    #endif
};





bool Queue::Push(Event e){
    #if defined(ESP32)   
    return xQueueSend(q32,&e, ( TickType_t ) 0 );
    #elif defined(ESP8266)
    Serial.printf("Pusheando evento %s para esp8266\n",e.msg);
    q8266.add(e);
    #endif    
}


unsigned Queue::Length(){
    int ctos=0;
    #if defined(ESP32)
    ctos=uxQueueMessagesWaiting( q32 );
    #elif defined(ESP8266)
    ctos=q8266.numElements();
    #endif
    return ctos;

}



Event  Queue::Pop(){    
    #if defined(ESP32)    
    Event e;    
    xQueueReceive(q32,&e,0);
    #elif defined(ESP8266)    
    q8266.pull(e);
    Serial.printf("Popeando evento %s para esp8266\n",e.msg);
    #endif
    return e;
}


bool Queue::IsEmpty(){
    return Length()==0;
}