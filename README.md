# NanoDPI 🚀

NanoDPI, C++ ile sıfırdan geliştirilmiş, Uygulama Katmanı (Layer 7) seviyesinde çalışan hafif, hızlı ve bağımsız bir yerel proxy (Local Proxy) sunucusudur. 

Temel amacı, İnternet Servis Sağlayıcıları (ISS) tarafından uygulanan **Derin Paket İnceleme (DPI - Deep Packet Inspection)** cihazlarını atlatarak Discord, YouTube ve diğer engellenmiş web sitelerine erişimi güvenli bir şekilde sağlamaktır.

## ⚙️ Özellikler

* **TLS ClientHello Parçalama:** DPI cihazlarını atlatmak için HTTPS bağlantılarının başlangıcındaki TLS başlığını (ClientHello) ilk bayttan böler, araya milisaniyelik gecikmeler koyarak ISS filtrelerini "kör" eder.
* **Otomatik Sistem Entegrasyonu:** Windows Kayıt Defterine (Registry) anlık müdahale ederek sistem proxy ayarlarını otomatik olarak `127.0.0.1:8081` adresine yönlendirir. Kullanıcının manuel ayar yapmasına gerek kalmaz.
* **Güvenli Kapanış (Graceful Shutdown):** Windows sinyal yakalama (Signal Handling) mimarisi kullanılarak, program aniden kapatılsa bile (Örn: Çarpıya basıldığında) saniyeler içinde Windows proxy ayarlarını eski orijinal haline döndürür. İnternet bağlantınızı bozmaz.
* **Çoklu İş Parçacığı (Multi-threading):** Gelen her bağlantı isteği bağımsız bir `thread` üzerinde işlenir. Bu sayede yüzlerce eşzamanlı sekme açıldığında bile darboğaz (bottleneck) yaşanmaz.
* **Thread-Safe Loglama:** `std::mutex` kullanılarak terminal ekranındaki logların birbirine girmesi engellenmiştir.

## ⚠️ Sınırlamalar (Neler Çalışmaz?)

Bu proje bir **Uygulama Katmanı (Layer 7) TCP Proxy'sidir.**

Chrome, Edge, Firefox veya Discord Web gibi proxy ayarlarını dinleyen "kibar" uygulamalarda kusursuz çalışır. Ancak doğrudan Ağ Katmanına (Layer 3/4) inen ve kendi raw UDP/TCP soketlerini açan istemcilerde **çalışmaz.**
* **Desteklenmeyenler:** Roblox, Counter-Strike 2, League of Legends gibi rekabetçi oyunlar istemcileri ve doğrudan UDP/QUIC protokolü kullanan masaüstü uygulamaları.

*(Ağ katmanı seviyesindeki oyun bypass işlemleri için projenin WinDivert tabanlı V2 mimarisi planlanmaktadır.)*

## 🛠️ Teknik Altyapı ve Çalışma Mantığı

Proje saf C++ ve Windows soket kütüphanesi (`winsock2`) kullanılarak geliştirilmiştir. 

1. **İstek Yakalama:** Tarayıcıdan gelen HTTP `CONNECT` istekleri yerel sunucuda yakalanır.
2. **DNS Çözümleme:** İstenilen hedefin IP adresi `getaddrinfo` ile çözümlenir.
3. **Parçalama (Fragmentation):** Hedef sunucu ile bağlantı kurulduğunda, Nagle algoritması (`TCP_NODELAY`) kapatılır. Tarayıcıdan gelen ilk paket (ClientHello) belirlenen noktadan (`Config::DPI_SPLIT_POINT = 1`) ikiye bölünür.
4. **Tünelleme:** İlk parça gönderilir, sistem uyutulur (`Sleep(10)`), ardından kalanı gönderilir. Sonrasında istemci ve sunucu arasındaki veri şeritleri izole edilerek (Select multiplexing) hızlı bir aktarım sağlanır.

## 🚀 Kurulum ve Kullanım

### Gereksinimler
* Windows İşletim Sistemi
* MinGW (g++) veya MSVC Derleyicisi
* CMake (Sürüm 4.0 veya üzeri)
cd build
cmake ..
cmake --build . --config Release

### Kullanım
1. Oluşturulan NanoDPI.exe dosyasına çift tıklayarak çalıştırın.
2. Siyah terminal ekranı açılacak ve "Sistem Proxy ayarları otomatik açıldı" mesajı belirecektir.
3. Artık tarayıcınızdan Discord'a veya istediğiniz siteye girebilirsiniz.
4. Programı sonlandırmak için pencerenin sağ üst köşesindeki X (Kapat) tuşuna basmanız yeterlidir. Tüm internet ayarlarınız otomatik olarak güvenli bir şekilde eski haline dönecektir.
