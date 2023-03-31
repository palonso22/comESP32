#ifndef  __VIA_H_
#define __VIA_H_
#include "../../include/Constants.h"

class Client;

class Via{
    public:
        virtual bool Connect()=0;//=0 significa que Via es una clase abstracta
        virtual void Disconnect()=0;
        virtual bool IsConnected()=0;
        virtual bool LinkLayerConnection()=0;
        virtual Client* getClient()=0;
        virtual int getSignal()=0;
        virtual bool IsVia(CurrentVia)=0;
        virtual CurrentVia getViaName()=0;
        virtual IPAddress getIP()=0;
        virtual char* getEventConnect()=0;
        virtual char* getEventDisconnect()=0;
        virtual void PrintConexionParameters()=0;        
        virtual unsigned long getSignalTime()=0;
        virtual bool Restart()=0;
        
};


#endif