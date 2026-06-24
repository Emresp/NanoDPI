//Dosyanın iki kere okunmasını engellemek için
#pragma once
#ifndef NANODPI_PROXYSERVER_H
#define NANODPI_PROXYSERVER_H
//Soket progralama için gerekli olan kütüphane
#include <winsock2.h>
#include <thread>
#include <wininet.h>

class ProxyServer
{
    public:
    //Kurucu ve yıkıcı metotlar
    ProxyServer();
    ~ProxyServer();

    //Socketi tek tuşla başlatmak için start metodu
    void start();

    private:
    //SOCKET yapısında serversocket adında socket bilgilerini tutucak değişken
    SOCKET serverSocket;

    //Yakalanan istekler üzerinden işlem yapabilmek için
    void handleClient(SOCKET clientSocket);

    //Windows proxy ayarlarını yönetebilmek için
    void setSystemProxy(bool enable);




};

#endif //NANODPI_PROXYSERVER_H