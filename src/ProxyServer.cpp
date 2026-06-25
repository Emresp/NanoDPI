#include "../include/ProxyServer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ws2tcpip.h>
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

    setSystemProxy(false);
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

    setSystemProxy(true);

    //paketleri her zaman yakalayabilmesi için sonsuz döngü
    while (true)
    {
        //Yakalanan istekleri tutabilmek için değişken
            sockaddr_in clientAddress;

        //tutulan isteklerin boyutu
        int clientSize=sizeof(clientAddress);

        //Adresler bu kısımda yakalanır
        //ilk parametre hangi sokete bakması gerektiği
        //ikinci parameterede ise casting işlemi ile yakalanan parametlerin hangi adrese gitmesi gerektiği
        //üçüncü parametre ile ise boyut tutulur
        //yakalanan isteklerin işlenebilmesi için fonksiyona verilmesi gerekir, fonksiyona verebilmek için clientSockete atadık
        SOCKET clientSocket=accept(serverSocket, (sockaddr*)&clientAddress, &clientSize);

        //istek eğer boşsa durmasın hemen devam etsin çünkü bilgisayarımızda saniylerde 100lerce istek gidiyor
        if (clientSocket == INVALID_SOCKET)
        {
            continue;
        }

        //istekleri işlerken bu döngünün içinde direk fonksiyuna verirsek zaman kaybı yaşarız çünkü diğer isteği yaklamak için işlemin bitmesi beklenir
        //Beklemeden dolayı diğer istekleri kaybedebiliriz ve pingimiz çıkar
        //Bu durumların önüne geçebilmek için thread yani paralel işlem gücünü kullanacz. Gelen istek thread dolayı döngüden çıkar ramde başka bir yerde işlem görür
        //Thread sayesinde işlemlerin birbrini beklemsine gerek kalmaz her istek bağımsız ve paralel olarak işlenir
        thread(&ProxyServer::handleClient, this, clientSocket).detach();
        //detach ile ise işlendikten sonra kendi kendini ramde temizlemesi ve ramde birikme yapmamamsı için gerekli olan fonksiyondur

    }
}

void ProxyServer::handleClient(SOCKET clientSocket) {
    //Gelen istekleri tutucak değişken
    char buffer[8192] = {0};

    //ilk değişken yakaladığımız istek
    //ikinci değişken veriyi yazacağımız yer
    //stringlerin sonuna 1baytlık \0 karakteri geldiği içiçn -1 yaptık yer kısmında
    //0 deafult değer
    int bytesRead=recv(clientSocket, buffer, sizeof(buffer)-1, 0);

    //Yakalanan istek baytlarını stringe çeviriyoruz
    string request(buffer);

    //Kelime kelime bölmek için
    stringstream ss(request);
    //bölünmüş kelimelerin ilk üçünü değişken içine alıyoruz
    string method, url, version;

    ss >> method >> url >> version;

    if(bytesRead > 0)
    {
        string host;
        string port;

        //url'i port ve host olarak parçalama işlemi
        //find fonksiyonu sayesinde : işaretinin indexini öğreniriz
        size_t ikinokta = url.find(":");
        if (ikinokta != string::npos)
        {
            host = url.substr(0, ikinokta);

            port =url.substr(ikinokta+1);
        }
        else
        {
            host = url;
            port = "80";
        }

        cout << "Yontem: " << method << " | Host: " << host << " | Port: " << port << endl;

        //DNS rehberinden hangi ip adresleri getirmesi gerektiğini filtreledik
        struct addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));//klasik çöp değerleri sıfırlama işlemi
        hints.ai_family = AF_UNSPEC;//ipv4 ipv6 fark etmez
        hints.ai_socktype = SOCK_STREAM; //TCP

        //Dönen ip adreslerini linked list olarak tutucaz
        struct addrinfo* result=nullptr;

        //isteklerde bulunan adreslerin dnsini öğrenmek için getaddrinfo fonksiyonu kullandık
        //.c_str metotları string olarak tuttuğumuz bilgileri bilgisyarın anlayabilceği hale çevirir
        //hints ile filtrelediğimiz özzelikte bulunan dnsleri verir ve soket uyuşmazlığı yaşamayız biz sadece tcp bakmalıyız mesele
        //result ise DNSni öğrenmek istediğimiz adresi dnslerini toplar ve linked list şeklinde tutar
        //getaddrinfo fonksiyonumuz bu işlemleri yapar ve sadece integer değer döner işlemin başarılı olup olmadığı hakkında
        int dnsStatus=getaddrinfo(host.c_str(), port.c_str(), &hints, &result);

        if(dnsStatus != 0)
        {
            //adres bulunamadıysa ya da başka bir hata olduysa
            cerr << "getaddrinfo() failed ";
            return;
        }

        cout << "[+] DNS Basarili! " << host << " icin IP listesi alindi." << endl;

        //Bulduğumuz ip adreslerini test edebilmek için socket açıyoruz
        SOCKET targetSocket=INVALID_SOCKET;

        //Linked list yani ip adreslerinin olduğu listede gezebilmek için pointer
        struct addrinfo* ptr=nullptr;

        //Listenin en başından başlayarak gezen for döngüsü
        for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
        {
            //Socket bilgilerini giriyoruz ve socket fonksiyonun geri dönüşünü almak için targetsockete eştiliyoruz
            //Socket özzelikleriyle birlikte başarılı şekilde oluşabiliyorsa zaten işletim sistemini onu oluşturur
            targetSocket=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

            //Soket kontrolü
            if (targetSocket == INVALID_SOCKET)
            {
                continue;
            }

            //İp adresi ile bağlantı kurmak için gerekli fonksiyon
            //ilk parametersi az önce açtığımız soket üzerinden bağlantı denemsi yapılcağını beliritri
            //İkinci parametere bağlanamsını istediğimiz ip adresi
            //Üçüncü parametre güvenlik amaçlı bellekte fazla yer okumaması için şu anki ip adresin uzunluğunu gireriz
            //durumu kontrol etmek için değişkene atadık
            int connect_kontrol=connect(targetSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

            //Bağlantı kontrolü
            if (connect_kontrol==0)
            {
                break;
            }
            else
            {
                closesocket(targetSocket);
            }
        }

        freeaddrinfo(result);

        if (targetSocket == INVALID_SOCKET)
        {
            cout<<"DNS not connect";
        }

        cout<<"DNS connect";
    }
}

void ProxyServer::setSystemProxy(bool enable)
{
    HKEY hKey;

    //HKEY_CURRENT_USER parametersi aktif açık olan oturumu vfonksiyona verir
    //İkinci parametere register defterinde değişmesi gereken dosyayanın adresi
    //0 parametersi varsayılan kullan demektir
    //KEY_SET_VALUE klasörü sadece değiştirmek için açıyoruz anlamına gelir işletim sistemine söylüyoruz
    //Klasör açıldığı zaman &hKey paramtersi ile klasörün adresini tutuyoruz
    RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_SET_VALUE, &hKey);

    if (enable)
    {
        //Proxyi aktif ediyoruz true
        DWORD proxyEnable = 1;
        //ilk parametere  RegOpenKeyExA fonksiyonu ile açtığımız dosyayı verir
        //ikinci parametre değişicek olan ayarın adı
        //0 parametersi default değer
        RegSetValueExA(hKey, "ProxyEnable", 0, REG_DWORD, (const BYTE*)&proxyEnable, sizeof(proxyEnable));

        // 2. ProxyServer adresini bizim programımız olarak ayarla
        const char* proxyServer = "127.0.0.1:8080";
        RegSetValueExA(hKey, "ProxyServer", 0, REG_SZ, (const BYTE*)proxyServer, strlen(proxyServer));
    }
    else
    {
        DWORD proxyEnable = 0;
        RegSetValueExA(hKey, "ProxyEnable", 0, REG_DWORD, (const BYTE*)&proxyEnable, sizeof(proxyEnable));
    }
    RegCloseKey(hKey);

    InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOptionA(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

}


