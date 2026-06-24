#include "../include/ProxyServer.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib") //Ağ kütüphanesi derleyici ile bağlamak için
using namespace std;

//Kurucu Metot tanımlama
ProxyServer::ProxyServer()
{
    //Sürüm bilgilerini tutabilmek için
    WSAData wsaData;


    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        cerr << "WSAStartup() failed";
    }
    else
    {
        cout << "WSAStartup() succeeded";
    }
}

//Yıkıcı metot tanımlama
ProxyServer::~ProxyServer()
{
    WSACleanup();
}

