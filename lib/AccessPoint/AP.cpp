#include "AP.h" 
#include "../../include/Constants.h"
#define APonTime 1000*600 //tiempo acción del punto de acceso

#if defined(ESP32)
#include <SPIFFS.h>
#include <WebServer.h>
#elif defined(ESP8266)
#include "LittleFS.h"
#include <ESP8266WebServer.h>
#endif

void ChangeState(GlobalState);
void logComunicador(const char*);

//parametros de conexion harcodeados para debug
/*char wifiSSID[MAX_SIZE_P]="";
char wifiPASS[MAX_SIZE_P]="";
char gprsAPN[MAX_SIZE_P]="datos.personal.com.";
char gprsPASS[MAX_SIZE_P]="datos";
char gprsUSER[MAX_SIZE_P]="datos";
char mqttHost1[MAX_SIZE_P]="host1.centinet.com.ar";
char mqttHost2[MAX_SIZE_P]="host1.centinet.com.ar";
char mqttPort[MAX_SIZE_P]="18831";
char mqttPass[MAX_SIZE_P]="8j3NDEbD";
char business[MAX_SIZE_P]="CEN";
char account[MAX_SIZE_P]="0007";*/

char wifiSSID[MAX_SIZE_P]="";
char wifiPASS[MAX_SIZE_P]="";
char gprsAPN[MAX_SIZE_P]="";
char gprsPASS[MAX_SIZE_P]="";
char gprsUSER[MAX_SIZE_P]="";
char mqttHost1[MAX_SIZE_P]="";
char mqttHost2[MAX_SIZE_P]="";
char mqttPort[MAX_SIZE_P]="";
char mqttPass[MAX_SIZE_P]="";
char business[MAX_SIZE_P]="";
char account[MAX_SIZE_P]="";
const char* filePath = "/parametros.txt";
extern bool useWifi;
extern bool useGprs;

#if defined(ESP32)
WebServer server(80);
#elif defined(ESP8266)
ESP8266WebServer server(80);
#endif

//signaturas
void setupAP(bool&,bool&);
void launchWeb(String,bool&,bool&);
void createWebServer(String,bool&,bool&);
bool readParameters();

//Leer parametros de las vias
bool checkParams(){
    if(!readParameters())return false;
    bool paramsOk=true;       
    //chequear que los parametros son no vacios     
    paramsOk&=strlen(mqttHost1)>0 && strlen(mqttHost2)>0 && strlen(mqttPort)>0 && strlen(business) && strlen(account)>0;            
    if(useWifi)
      paramsOk&=strlen(wifiSSID)>0 && strlen(wifiPASS)>0;      
    if(useGprs)        
        paramsOk&=strlen(gprsAPN)>0 && strlen(gprsUSER)>0 && strlen(gprsPASS)>0;        
    
    return paramsOk;    
}

bool existConfigurationFile(){
    #if defined(ESP32)
    return SPIFFS.exists(filePath);
    #elif defined(ESP8266)
    return LittleFS.exist(filePath);
    #endif
}

bool deleteConfigurationFile(){
    if (!existConfigurationFile())return true;
    #if defined(ESP32)
  	return SPIFFS.remove(filePath);
    #elif defined(ESP8266)
    return LittleFS.remove(filePath);
    #endif    

}

//Levanta el access point y retorna true si los datos ingresados son validos
bool InitAP(){
    //Inicia Modo AP           
    ChangeState(APMode);
    logComunicador("Iniciando AP");
    bool checkValid;
    bool finish=false;
    setupAP(checkValid,finish);// Setea punto de acceso
    unsigned long startTime=millis();        
    while (!finish&& (millis() - startTime < APonTime))//levanta el punto de acceso en caso de que no se pueda conectar y este en la vantana de tiempo
    {        
        server.handleClient();//mantiene levantado el punto de acceso        
    }
    //en caso de que paso la ventana de tiempo y no se conecto tirar el web server e intentar arrancar con los parametros seteados
    Serial.println("Server Web detenido");
    server.stop();
    Serial.println("AP detenido");    
    WiFi.softAPdisconnect(true);
    return checkValid;
}





//Setea access point
void setupAP(bool& checkValid,bool& finish){
    unsigned char n;
    unsigned char i;
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    delay(100);
    n = WiFi.scanNetworks();//se escanean todas las redes wifi disponibles y devuelve la cantidad de redes escaneadas
    if (n == 0)
        Serial.println("no networks found");
    else{
        Serial.print(n);
        Serial.println(" networks found");
        for (i = 0; i < n; ++i){            
            Serial.printf("%d:%s,(%d)\n",i,WiFi.SSID(i).c_str(),WiFi.RSSI(i));            
            delay(10);
        }
    }
    Serial.println("");
    String st = "<ol>";
    for (i = 0; i < n; ++i){
        // Print SSID and RSSI for each network found
        st += "<li>";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i); 
        st += ")";
        Serial.println("");        
        st += "</li>";
    }
    st += "</ol>";
    delay(100);
    WiFi.softAP("AccesPointComunicador", "");
    Serial.println("Lanzando AP");    
    launchWeb(st,checkValid,finish);
}

void launchWeb(String st,bool& checkValid,bool& finish){
    Serial.println("Lanzando web");
    if (WiFi.status() == WL_CONNECTED) Serial.println("AP levantado");  
    createWebServer(st,checkValid,finish);
    server.begin();//levanta el servidor
    Serial.println("WebServer iniciado");
}
void loadParameter(String key,String value){  
    Serial.println("Cargando parametro "+key+","+value);  
    if (key.equals(kwifiID)){
        value.toCharArray(wifiSSID,MAX_SIZE_P);
    }
    else if (key.equals(kwifiPASS)){
        value.toCharArray(wifiPASS,MAX_SIZE_P);
    }
    else if (key.equals(kUseWifi)){
        useWifi=value.equals("1");
    }
    else if (key.equals(kgprsAPN)){
        value.toCharArray(gprsAPN,MAX_SIZE_P);
    }
    else if (key.equals(kgprsUSER)){
        value.toCharArray(gprsUSER,MAX_SIZE_P);
    }
    else if (key.equals(kgprsPASS)){
        value.toCharArray(gprsPASS,MAX_SIZE_P);
    }
    else if (key.equals(kServer1)){
        value.toCharArray(mqttHost1,MAX_SIZE_P);
    }
    else if (key.equals(kServer2)){
        value.toCharArray(mqttHost2,MAX_SIZE_P);
    }

    else if (key.equals(kServerPass)){
        value.toCharArray(mqttPass,MAX_SIZE_P);
    }

    else if (key.equals(kServerPort)){
        value.toCharArray(mqttPort,MAX_SIZE_P);
    }

    else if (key.equals(kbusiness)){
        value.toCharArray(business,MAX_SIZE_P);
    }        
    else if (key.equals(kaccount)){
        value.toCharArray(account,MAX_SIZE_P);
    }
}



//Tomar argumento del web service, validarlo y copiarlo
bool copyValueWS(String key){
    if (key.equals(kUseWifi)){        
        useWifi=!server.hasArg(key);        
        return true;
    }
    //Limpiar espacios
    String value=server.arg(key);
    value.trim();    
    loadParameter(key,value);    
    return value.length()>0;
}


//Crear contenido del webserver
void createWebServer(String st,bool& checkValid,bool& finish){        
    server.on("/", [st]() {
        String content="";        
        String ip=String(WiFi.softAPIP()[0])+"."+String(WiFi.softAPIP()[1])+"."+String(WiFi.softAPIP()[2])+"."+String(WiFi.softAPIP()[3]);        
        content = "<!DOCTYPE HTML>\r\n<html>";        
        content += "<h1>"+ String(SoftwareName) + " "+ String(SoftwareVersion) + "</h1>";
        content += "<h2>Por favor, introduzca los parametros de inicializacion para una correcta comunicacion con monitoreo </h2>";        
        content += ip;
        content += "<p>";
        content += st;
        content += "</p><form method='post' action='setting'>";
        content += "<label><p>Parametros para comunicacion con monitoreo: </p></label>";
        content += "<label>Servidor de monitoreo 1: </label><input name='"+String(kServer1)+"' length=32><br>";
        content += "<label>Servidor de monitoreo 2: </label><input name='"+String(kServer2)+"' length=32><br>";
        content += "<label>Puerto:</label><input name='"+String(kServerPort)+"' length=64><br>";
        content += "<label>Pass del Servidor: </label><input name='"+String(kServerPass)+"' length=32><br>";        
        content +=  "<label>Empresa: </label><input name='"+String(kbusiness)+"' length=64><br>";
        content +=  "<label>Cuenta: </label><input name='"+String(kaccount)+"' length=64><br>";        
        content += "<p>Parametros de wifi:</p>";
        content += "<label>SSID: </label><input name='"+String(kwifiID)+"' length=32><br><label>Password: </label><input name='"+String(kwifiPASS)+"' length=64><br>";        
        content += "<input type='checkbox' name="+String(kUseWifi)+">";
        content += "<label>Usar sin wifi</label><br>";        
        if (useGprs){
            content += "<p>Modulo gprs detectado, ingrese parametros:</p>";
            content += "<label>APN: </label><input name='"+String(kgprsAPN)+"' length=50><br><label>User: </label><input name='"+String(kgprsUSER)+"' length=20><br><label>Pass: </label><input name='"+String(kgprsPASS)+"' length=20><br>";
        }                    
        
        content +="<input value='Aceptar' type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);        
    });    
    server.on("/setting", [&checkValid,&finish]() mutable {
        if(!deleteConfigurationFile())ESP.restart();
        checkValid=copyValueWS(kServer1);                
        checkValid&=copyValueWS(kServer2);        
        checkValid&=copyValueWS(kServerPort);                
        checkValid&=copyValueWS(kServerPass);
        checkValid&=copyValueWS(kbusiness);
        checkValid&=copyValueWS(kaccount);     
        checkValid&=copyValueWS(kUseWifi);  
        if(checkValid)writeParams(None);   
        //Chequear wifi solo si debe ser usado        
        if (useWifi){            
            checkValid&=copyValueWS(kwifiID);
            checkValid&=copyValueWS(kwifiPASS); 
            if(checkValid) writeParams(Wifi);           
        }

        if (useGprs){
            checkValid&=copyValueWS(kgprsAPN);
            checkValid&=copyValueWS(kgprsUSER);
            checkValid&=copyValueWS(kgprsPASS);
            if(checkValid) writeParams(Gprs);           
        }                        
        String content = "<!DOCTYPE HTML>\r\n<html>";                
        content += "<p>";
        if (checkValid){
            content += "Parametros establecidos.Reiniciando...";        
        }
        else{
            content += "Alguno de los campos no fue seteado correctamente... Reiniciando";           
            deleteConfigurationFile(); 
        }
        
        content += "</p>";        
        content += "</html>";            
        server.send(200, "text/html", content);        
        delay(10000);
        Serial.println("Stop Server Web");
        server.stop();
        Serial.println("Stop Acces Point");
        WiFi.softAPdisconnect (true);                
        server.send(200, "text/html", content); 
        logComunicador("AP detenido,reiniciando");
        ESP.restart();  
        finish=true;              
    });     
}


File openConfigurationFile(const char*);
bool readParameters(){	
    File file =SPIFFS.open(filePath,"r"); 
    if(!file)return false;
    String key,value;
	Serial.println("Leyendo parámetros");    
	//Cada linea consta de un par key=valor
	while (file.available()){                         
        key=file.readStringUntil('=');
        Serial.println("Key leida:"+key);
        value=file.readStringUntil('\n');
        Serial.println("Valor leido:"+value);        
        loadParameter(key,value);
    }    
    file.close();    
}



bool InitFS(){
    #if defined(ESP32)
    return SPIFFS.begin(true);
    #elif defined(ESP8266)
    return LittleFS.begin();
    #endif
    
}

File openConfigurationFile(const char* mode){    
    #if defined(ESP32)        
  	return SPIFFS.open(filePath,mode); //Abro el archivo en modo mode
    #elif defined(ESP8266)
    return LittleFS.open("/parametros.txt",mode); //Abro el archivo para solo lectura 
    #endif        
}



bool writeParamsKV(const char*,const char*);
bool writeParams(CurrentVia v){
    Serial.println("Escribiendo parametros");      
    bool check=true;    
    switch(v){
        case Wifi:            
            Serial.println("De wifi");            
            check&=writeParamsKV(kwifiID,wifiSSID);
            check&=writeParamsKV(kwifiPASS,wifiPASS);            
        break;

        case Gprs:
            Serial.println("De gprs");        
            check&=writeParamsKV(kgprsAPN,gprsAPN);
            check&=writeParamsKV(kgprsUSER,gprsUSER);
            check&=writeParamsKV(kgprsPASS,gprsPASS);
        break;

        case None:
            check&=writeParamsKV(kServer1,mqttHost1);
            check&=writeParamsKV(kServer2,mqttHost2);
            check&=writeParamsKV(kServerPort,mqttPort);
            check&=writeParamsKV(kServerPass,mqttPass);
            check&=writeParamsKV(kbusiness,business);
            check&=writeParamsKV(kaccount,account);            
            check&=writeParamsKV(kUseWifi,useWifi?"1":"0");
        break;     
    }
    return check;
}


bool writeParamsKV(const char* key, const char* value){        
    File file=SPIFFS.open(filePath,"a");  
    if(!file)return false;               
    Serial.printf("Escribiendo %s=%s\n",key,value);
    size_t ctos = file.printf("%s=%s\n",key,value);        
    file.close();
    return ctos == strlen(key)+strlen(value)+2;       
}