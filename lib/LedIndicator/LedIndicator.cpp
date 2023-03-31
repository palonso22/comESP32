
#include "LedIndicator.h"
#include "../../include/Constants.h"

extern GlobalState state;
extern SemaphoreHandle_t xState;
extern SemaphoreHandle_t xFlags;
extern bool comunicadorStatus[3];

void blink(long interval){
    /*
      interval: cantidad en millisegundos que el proceso
      queda en delay
    */    
    digitalWrite(LED_BUILTIN, HIGHLED);    	    
    delay(interval);    	
    digitalWrite(LED_BUILTIN, LOWLED);           
    delay(interval);
    			        
}


/*
Estados a reportar:
 - inicializacion -> blink de un segundo
 - <modo> AP -> blink de 500 milisegundos
 - salida de AP y probando vias-> blink de 3 segundos
 - estado correcto -> encendido permanente
 - desconexion de red wifi -> 2 blinks de 3 segundos cada 5 segundos
 - desconexion modulo gprs -> 4 blinks de 3 segundos cada 5 segundos
 - desconexion de ambas vias -> 6 blinks de 3 segundos cada 5 segundos
*/
void taskLedIndicator(void *p){    
    Serial.println("Inicia led indicator");	        
    digitalWrite(LED_BUILTIN, HIGHLED);    				            
    GlobalState current;
    byte ctosBlinks=0;      
    while(true){            
            // Indicar estado de las vias, primero se lee el estado actual 
            if(xSemaphoreTake(xState,portMAX_DELAY ) == pdTRUE){
                current=state;
                xSemaphoreGive(xState);
            } 

            switch(current){
                case initialize:                    
                    blink(500);
                    break;
                case APMode:                
                    blink(100);
                    break;
                case checkingVias:
                    blink(1500);
                    break;
                case pooling:    
                    //en modo loop se mira la conexion de las 
                    //vias + la conexion mqtt                
                    ctosBlinks = 0;
                    //Se calcula la cantidad de blinks a partir del arreglo
                    //con el estado de conexion de las vias
                    if(xSemaphoreTake(xFlags,portMAX_DELAY)==pdTRUE){                        
                        for(int i=0;i<3;i++) ctosBlinks+=pow(2,i)*!comunicadorStatus[i];                                                                                                                                                                             
                        xSemaphoreGive(xFlags);
                    }                    
                    //Se duplica la cantidad de blinks calculados                    
                    for(int i =0;i<ctosBlinks;i++) blink(250);                                                                   
                    if(ctosBlinks==0)digitalWrite(LED_BUILTIN, HIGHLED);    	    
                    delay(750);
                    break;
            }
                       
    }
    vTaskDelete(TaskSetupLed);
}



