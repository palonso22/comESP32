#include <Arduino.h>
#include <cmath>
#include "GPIO.h"
using std::pow; 


extern bool wifiON;
extern bool ethON;
extern bool gprsON;
extern bool mqttON;
extern bool initON;

//MUTEX
extern SemaphoreHandle_t xWifiOn;
extern SemaphoreHandle_t xEthOn;
extern SemaphoreHandle_t xGprsOn;
extern SemaphoreHandle_t xInitOn;
extern SemaphoreHandle_t xMqttOn;
extern SemaphoreHandle_t xDSC;

extern TaskHandle_t TaskSetupLed;

void taskLedIndicator(void *);