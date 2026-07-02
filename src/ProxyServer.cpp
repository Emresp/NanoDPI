#include "../include/ProxyServer.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib") //Ağ kütüphanesi derleyici ile bağlamak için
using namespace std;

//Veri bitene kadar send çağrısını tekrarlar
//Kendimiz maneul send çağırdığımız zaman hepsini kontrol etmeye biliyor bu fonksiyon sayesinde oto yaptık
bool sendAll(SOCKET s, const char* data, int len)
{
    int totalSent = 0;
    while (totalSent < len)
    {
        int sent = send(s, data + totalSent, len - totalSent, 0);
        if (sent <= 0) return false;
        totalSent += sent;
    }
    return true;
}

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
        cerr << "WSAStartup() failed "<< WSAGetLastError() << endl;
        return;
    }
    else
    {
        cout << "WSAStartup() succeeded "<< endl;
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
        return;
    }
    else
    {
        cout << "socket() succeeded ";
    }

    //sockaddr_in yapısında serverAddress değişkeni oluşturduk bu değişken ıp ve portu birlikte tutar
    sockaddr_in serverAddress;

    ZeroMemory(&serverAddress, sizeof(serverAddress));

    //yukarıda ıpv4 ile oluşturduğumuz socketi burada da eşlebilmesi için ipv4 olarak seçiyoruz
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //inet_addr ip adresini bilgisayarın anlayabileceği şekle getirir
    serverAddress.sin_port = htons(Config::NANO_PROXY_PORT);// Little Endian yani pc tersten yazdığı için sayıları biz internetin anlayabileceği düz şekle htons ile getirdik

    //oluşturduğumuz serverAddress değişkenini  kendi oluşturduğumuz sockete mbağlıyoruz bind ediyoruz
    //(sockaddr*)&serverAddress casting işlemi var
    SOCKET bind_control=bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));

    //bind işlemi kontrol
    if (bind_control == SOCKET_ERROR)
    {
        cerr << "bind() failed "<< endl;
        return;
    }
    else
    {
        cout << "bind() succeeded "<< endl;
    }

    //oluşturduğumuz soketi dinleyebilmek izleybilmek için gerekli  fonksiyon
    //ilk parametre hangi soketi dinlemesi gerektiği
    //ikinci parametre ise maksimum isteği al anlamına gelir
    SOCKET listen_control=listen(serverSocket,SOMAXCONN);

    //listen işlemi kontrolü
    if (listen_control == SOCKET_ERROR)
    {
        cerr << "listen() failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        return;
    }
    else
    {
        logLine("Proxy 127.0.0.1:" + to_string(Config::NANO_PROXY_PORT) + " uzerinde dinleniyor");
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

    if (method != "CONNECT")
    {
        // Tarayıcıya "Ben sadece HTTPS destekliyorum" cevabı veriyoruz
        const char* msg = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(clientSocket, msg, strlen(msg), 0);
        closesocket(clientSocket);
        return; // Fonksiyondan çık, işlemi bitir
    }

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
            port = "443";
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
            logLine("getaddrinfo() failed: " + to_string(dnsStatus) + " | Host: " + host);
            closesocket(clientSocket);
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
                cout << "Target connect failed" << endl;
                closesocket(clientSocket);
                return;
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
                //windowstaki Nagle algoritmasını kapatmak için gerekli fonksiyon
                //Nagle algoritması çok küçük bir paketi tek başına göndermek yerine bekler arkasından gelen paketle birleştirip gönderir
                //Bu olay ise bizim ISS anlamasın diye parça parça gönderme işlemimize ters düşer
                int flag = 1;
                setsockopt(targetSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

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
            cout << "Target connect failed" << endl;
            closesocket(clientSocket);
            return;
        }

        cout << "Target connected" << endl;

        //İp adresi ile bağlantı sağlandıysa gelen isteğe karşı cevap olarak 200 OK cevabını verdik bağlantı başarılı anlamında
        if (method == "CONNECT")
        {
            const char* okMessage = "HTTP/1.1 200 Connection Established\r\n\r\n";

            if (!sendAll(clientSocket, okMessage, (int)strlen(okMessage)))
            {
                closesocket(clientSocket);
                closesocket(targetSocket);
                return;
            }
        }

        fd_set sockets_tepsisi;

        bool ilkPaketMi = true;

        //Split işleminde kullanılan değişkenleri daha kolay değiştirmek için
        const int DPI_SPLIT_POINT = 1;
        const int DPI_DELAY_MS = 10;


        while (true)
        {
            //Soketleri senkronize edebilmek için gerekli olan veri yapısı
            //Hedef yani doğru adreslerin olduğu soket ile kullanıcnın isteklerinin olduğu clientssocket değişkenini aynı kümeye koyduk
            FD_ZERO(&sockets_tepsisi);
            FD_SET(clientSocket, &sockets_tepsisi);
            FD_SET(targetSocket, &sockets_tepsisi);

            //Kümelediğimiz soketlere bakarak içine veri gelen soket sayısını döndürür
            int activity = select(0, &sockets_tepsisi, nullptr, nullptr, nullptr);

            if (activity == SOCKET_ERROR)
            {
                break;
            }

            //Durum kontrolü yapılır kümelediğimiz soketlerden hangisi veri akışı yapıyor
            //Eğer clientten sunucya istek gidiyorsa
            if (FD_ISSET(clientSocket, &sockets_tepsisi))
            {
                char tunelBuffer[8192];
                //Okuma işlemi
                int bytesRcvd = recv(clientSocket, tunelBuffer, sizeof(tunelBuffer), 0);

                if (bytesRcvd <= 0)
                {
                    break;
                }

                // İlk client paketini bölüyoruz.
                // HTTPS CONNECT sonrası ilk gelen veri genelde TLS ClientHello olur.
                if (Config::DPI_SPLIT_ENABLED && ilkPaketMi && bytesRcvd > Config::DPI_SPLIT_POINT)
                {
                    //Giden byteların nerden bölünceğini tutan değişken
                    int bolunmeNoktasi = Config::DPI_SPLIT_POINT;

                    if (!sendAll(targetSocket, tunelBuffer, bolunmeNoktasi))
                    {
                        break;
                    }

                    //Böldüğümüz kısımların arasına bekleme süresi koyuruz bunun sebebi işletim sistemiz ya da herhangi başka bir ağ sağlayıcısı istekleri bekletip gönderirse diye
                    Sleep(Config::DPI_SPLIT_DELAY_MS);

                    if (!sendAll(targetSocket, tunelBuffer + bolunmeNoktasi, bytesRcvd - bolunmeNoktasi))
                    {
                        break;
                    }

                    logLine("[*] DPI Split: ClientHello bolundu " + to_string(bolunmeNoktasi) + " + " + to_string(bytesRcvd - bolunmeNoktasi) + " byte");

                    ilkPaketMi = false;
                }
                else
                {
                    if (!sendAll(targetSocket, tunelBuffer, bytesRcvd))
                    {
                        break;
                    }
                }
            }

            //Sunucudan clientte istek gidiyorsa
            if (FD_ISSET(targetSocket, &sockets_tepsisi))
            {
                char tunelBuffer[8192];
                //okuma işlemi
                //İlk parametre ile gelen veri okundu yani sunucudan gelen veriyi okuduk tunnelBuffer'ın içine yazdık
                //İkinci parametre verinin okunurken yazıldığı yer yani taşıyacağımız değişken
                //Üçüncü parametre ise taşma olmaması için kovamızın büyüklüğünü söylüyor ve tek seferde o kadarlık veri taşıyor
                int bytesRcvd = recv(targetSocket, tunelBuffer, sizeof(tunelBuffer), 0);

                if (bytesRcvd <= 0) break;

                if (!sendAll(clientSocket, tunelBuffer, bytesRcvd))
                {
                    break;
                }
            }
        }

        closesocket(clientSocket);
        if (targetSocket != INVALID_SOCKET) {
            closesocket(targetSocket);
        }
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

        //proxy adresi değiştime yani işletim sisteminde ara sunucu ayarı yapar
        string proxyAdres = "127.0.0.1:" + to_string(Config::NANO_PROXY_PORT);
        RegSetValueExA(hKey, "ProxyServer", 0, REG_SZ, (const BYTE*)proxyAdres.c_str(), proxyAdres.length());
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


