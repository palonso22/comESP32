#ifndef __AP_H__
#define __AP_H__
#include "../../include/Constants.h"

#define kServer1 "mqttHost1"
#define kServer2 "mqttHost2"
#define kServerPort "mqttPort"
#define kServerPass "mqttPass"
#define kwifiID "wifiSSID"
#define kwifiPASS "wifiPASS"
#define kUseWifi "useWifi"
#define kbusiness "business"
#define kaccount "account"
#define kgprsAPN "APN"
#define kgprsUSER "UserAPN"
#define kgprsPASS "PassAPN"
#define kminKA "keepAliveUDP"
#define MAX_SIZE_P 30


//
bool checkParams();
//inicia AccessPoint, las entradas determinan que parametros se deben poder setear en el mismo
bool InitAP();
//toma una via v y escribe los parametros de la misma
bool writeParams(CurrentVia);
//chequea que exista un el archivo de parametros
bool deleteConfigurationFile();
bool existConfigurationFile();
//Inicia el sistema de archivos
bool InitFS();
#endif 