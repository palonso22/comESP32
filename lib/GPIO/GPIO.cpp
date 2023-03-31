#include "GPIO.h"
#include "ProcessCID.h"
#include "../../include/ContactID.h"

#if defined(ESP8266)
#include <RingBuf.h>
#endif

volatile unsigned int lastTrigger1 = 0;
volatile unsigned int lastTrigger2 = 0;
volatile unsigned int lastTrigger3 = 0;
byte gpioOnPrev1=1, gpioOnActual1=0;
byte gpioOnPrev2=1, gpioOnActual2=0;
byte gpioOnPrev3=1, gpioOnActual3=0;
byte gpioVal1;
byte gpioVal2;
byte gpioVal3;

#if defined(ESP8266)
RingBuf *queueGpio1 = RingBuf_new(sizeof(byte), 10);
RingBuf *queueGpio2 = RingBuf_new(sizeof(byte), 10);
RingBuf *queueGpio3 = RingBuf_new(sizeof(byte), 10);
#elif defined(ESP32)
QueueHandle_t queueGpio1 = xQueueCreate( 10, sizeof( byte ) );
QueueHandle_t queueGpio2 = xQueueCreate( 10, sizeof( byte ) );
QueueHandle_t queueGpio3 = xQueueCreate( 10, sizeof( byte ) );
#endif

void gpio_setup(){
    Serial.println("Se inicializa los GPIO");
    pinMode(Input1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Input1), gpioReadInput1, CHANGE);
    #ifndef useRev2
    pinMode(Input2, INPUT_PULLUP);
    pinMode(Input3, INPUT_PULLUP);
    #endif
    #if defined(ESP32) || defined(useRev2)
    pinMode(Output1,OUTPUT); //Pines de salida comandados por comandos D1X y D0X
    pinMode(Output2,OUTPUT);
    #endif
    #if defined(ESP32)
    pinMode(ResetGprs,OUTPUT);
    pinMode(ResetEthernet,OUTPUT);
    #endif
    #ifndef useRev2
    attachInterrupt(digitalPinToInterrupt(Input2), gpioReadInput2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Input3), gpioReadInput3, CHANGE);
    #endif
    pinMode(LED_BUILTIN, OUTPUT);
}

void gpio_cambiar_salida(byte salida, byte estado){
    #if defined(useRev2) 
    if(salida == Output1){
        if(estado == HIGH){
            digitalWrite(Output1,HIGH);
            pcid_callback(califEv,outputCID,PARTITION_OFF,IO1);
            Serial.println("Salida 1 ON");
        }
        else if(estado == LOW){
            digitalWrite(Output1,LOW);
            pcid_callback(califRest,outputCID,PARTITION_OFF,IO1);
            Serial.println("Salida 1 OFF");
        }
    }
    if(salida == Output2){
        if(estado == HIGH){
            digitalWrite(Output2,HIGH);
            pcid_callback(califEv,outputCID,PARTITION_OFF,IO2);
            Serial.println("Salida 2 ON");
        }
        else if(estado == LOW){
            digitalWrite(Output2,LOW);
            pcid_callback(califRest,outputCID,PARTITION_OFF,IO2);
            Serial.println("Salida 2 OFF");
        }
    }
    #endif
}

#if defined(ESP32)
void IRAM_ATTR gpioReadInput1(){
#elif defined(ESP8266)
void ICACHE_RAM_ATTR gpioReadInput1(){
#endif
    if(!(millis() - lastTrigger1 < 50)){
        gpioOnActual1 = digitalRead(Input1);
        if(gpioOnPrev1 != gpioOnActual1){
            #if defined(ESP8266)
            queueGpio1->add(queueGpio1, &gpioOnActual1);
            #elif defined(ESP32)
            xQueueSend(queueGpio1, &gpioOnActual1, ( TickType_t ) 0 );
            #endif
            gpioOnPrev1 = gpioOnActual1;
            lastTrigger1 = millis();
        }
    }
}
#ifndef useRev2
#if defined(ESP32)
void IRAM_ATTR gpioReadInput2(){
#elif defined(ESP8266)
void ICACHE_RAM_ATTR gpioReadInput2(){
#endif
    if(!(millis() - lastTrigger2 < 50)){
        gpioOnActual2 = digitalRead(Input2);
        if(gpioOnPrev2 != gpioOnActual2){
            #if defined(ESP8266)
            queueGpio2->add(queueGpio2, &gpioOnActual2);
            #elif defined(ESP32)
            xQueueSend(queueGpio2, &gpioOnActual2, ( TickType_t ) 0 );
            #endif
            gpioOnPrev2 = gpioOnActual2;
            lastTrigger2 = millis();
        }
    }
}
#if defined(ESP32)
void IRAM_ATTR gpioReadInput3(){
#elif defined(ESP8266)
void ICACHE_RAM_ATTR gpioReadInput3(){
#endif
    if(!(millis() - lastTrigger3 < 50)){
        gpioOnActual3 = digitalRead(Input3);
        if(gpioOnPrev3 != gpioOnActual3){
            #if defined(ESP8266)
            queueGpio3->add(queueGpio3, &gpioOnActual3);
            #elif defined(ESP32)
            xQueueSend(queueGpio3, &gpioOnActual3, ( TickType_t ) 0 );
            #endif
            gpioOnPrev3 = gpioOnActual3;
            lastTrigger3 = millis();
        }
    }
}
#endif
void gpio_loop(){
    #if defined(ESP8266)
    while(!queueGpio1->isEmpty(queueGpio1)){
        queueGpio1->pull(queueGpio1, &gpioVal1);
    #elif defined(ESP32)
    if(xQueueReceive(queueGpio1, &gpioVal1, ( TickType_t ) 0) == pdPASS){
    #endif
        if(!gpioVal1){
            Serial.println("Entrada 1 ON");
            pcid_callback(califEv,input1CID,PARTITION_OFF,ZONE_OFF);
        }
        else{
            Serial.println("Entrada 1 OFF");
            pcid_callback(califRest,input1CID,PARTITION_OFF,ZONE_OFF);
        }
    }
    #ifndef useRev2

    #if defined(ESP8266)
    while(!queueGpio2->isEmpty(queueGpio2)){
        queueGpio1->pull(queueGpio2, &gpioVal2);
    #elif defined(ESP32)
    if(xQueueReceive(queueGpio2, &gpioVal2, ( TickType_t ) 0) == pdPASS){
    #endif
        if(!gpioVal2){
            Serial.println("Entrada 2 ON");
            pcid_callback(califEv,input2CID,PARTITION_OFF,ZONE_OFF);
        }
        else{
            Serial.println("Entrada 2 OFF");
            pcid_callback(califRest,input2CID,PARTITION_OFF,ZONE_OFF);
        }
    }
    #if defined(ESP8266)
    while(!queueGpio3->isEmpty(queueGpio3)){
        queueGpio1->pull(queueGpio3, &gpioVal3);
    #elif defined(ESP32)
    if(xQueueReceive(queueGpio3, &gpioVal3, ( TickType_t ) 0) == pdPASS){
    #endif
        if(!gpioVal3){
            Serial.println("Entrada 3 ON");
            pcid_callback(califEv,input3CID,PARTITION_OFF,ZONE_OFF);
        }
        else{
            Serial.println("Entrada 3 OFF");
            pcid_callback(califRest,input3CID,PARTITION_OFF,ZONE_OFF);
        }
    }
    #endif
}