#include "../include/ProxyServer.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib") //Ağ kütüphanesi derleyici ile bağlamak için
using namespace std;

//Kurucu Metot tanımlama
ProxyServer::ProxyServer()
{
    //Sürüm bilgilerini tutabilmek için
    WSAData wsaData;

    //Windowsta ağ işlemleri yapabilmek için WSAStartup fonksiyonunu çağırmamız gerek
    //MAKEWORD iki sayıyı işletim sisteminin anlayacağı dile çevirip 2.2 sürümünü çağırır
    //&wsaData ile sürüm bilgilerini koyacağı adresi seçtik
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    //kontrol
    if (result != 0)
    {
        cerr << "WSAStartup() failed ";
    }
    else
    {
        cout << "WSAStartup() succeeded ";
    }
}

//Yıkıcı metot tanımlama
ProxyServer::~ProxyServer()
{
    WSACleanup();
}


void ProxyServer::start()
{
    //İlk parametre ipv4 olduğunu söyler ikinci parametre ise TCP akış türünde soket açılcağını söyler
    //0 demek ise ipv4 ve TCP akışı olan bir socket için işletim sistemi tarafından protocol seçrer
    serverSocket = socket(AF_INET, SOCK_STREAM,0);

    //kontrol
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "socket() failed ";
    }
    else
    {
        cout << "socket() succeeded ";
    }

    //sockaddr_in yapısında serverAddress değişkeni oluşturduk bu değişken ıp ve portu birlikte tutar
    sockaddr_in serverAddress;

    //yukarıda ıpv4 ile oluşturduğumuz socketi burada da eşlebilmesi için ipv4 olarak seçiyoruz
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //inet_addr ip adresini bilgisayarın anlayabileceği şekle getirir
    serverAddress.sin_port = htons(8080);// Little Endian yani pc tersten yazdığı için sayıları biz internetin anlayabileceği düz şekle htons ile getirdik

    //oluşturduğumuz serverAddress değişkenini  kendi oluşturduğumuz sockete mbağlıyoruz bind ediyoruz
    //(sockaddr*)&serverAddress casting işlemi var
    SOCKET bind_control=bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));

    //bind işlemi kontrol
    if (bind_control == SOCKET_ERROR)
    {
        cerr << "bind() failed ";
    }
    else
    {
        cout << "bind() succeeded ";
    }

    //oluşturduğumuz soketi dinleyebilmek izleybilmek için gerekli  fonksiyon
    //ilk parametre hangi soketi dinlemesi gerektiği
    //ikinci parametre ise maksimum isteği al anlamına gelir
    SOCKET listen_control=listen(serverSocket,SOMAXCONN);

    //listen işlemi kontrolü
    if (listen_control == SOCKET_ERROR)
    {
        cerr << "listen() failed ";
    }
    else
    {
        cout << "Proxy 127.0.0.1:8080 uzerinde dinleniyor ";
    }
}


