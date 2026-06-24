//Dosyanın iki kere okunmasını engellemek için
#pragma once
#ifndef NANODPI_PROXYSERVER_H
#define NANODPI_PROXYSERVER_H
//Soket progralama için gerekli olan kütüphane
#include <winsock2.h>

class ProxyServer
{
    public:
    //Kurucu ve yıkıcı metotlar
    ProxyServer();
    ~ProxyServer();
};

#endif //NANODPI_PROXYSERVER_H