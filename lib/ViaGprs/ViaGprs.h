#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <TinyGsmCommon.h>
#include <ViaInterface.h>



// Set serial for debug console (to the Serial Monitor, default speed 115200)

#define PIN_RX 16
#define PIN_TX 17

#define RAT 1 //0: CATM1 | 1: GPRS | 2: NBIoT

//#define GPRS_TIME_RECONNECT 10000
#define recoverGprs "R35500000"
#define lostGprs "E35500000"



class ViaGprs: public Via{
    public:
         ViaGprs();         
         bool Connect();
         void Disconnect();
         bool IsConnected(); 
         bool LinkLayerConnection();
         Client* getClient();
         int getSignal();
         bool IsVia(CurrentVia);    
         CurrentVia getViaName();         
         IPAddress getIP();
         char* getEventConnect();
         char* getEventDisconnect();         
         void PrintConexionParameters();
         unsigned long getSignalTime();
         bool Restart();
         void SetParams(char*,char*,char*);
         bool CheckHosts(const char*,const char*,uint32_t);
         char* GetIMEI();
    private:
        HardwareSerial* SerialAT;
        TinyGsmClient* gsm;        
        TinyGsm* modem;
        char apn[30]="";
        char user[30]="";
        char pass[30]=""; 
        char imei[30]="";       
        void printCredentials();
        CurrentVia iam;    
        void Init();    
        byte tryCon=3;        
};
