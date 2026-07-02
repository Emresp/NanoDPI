#include "../include/ProxyServer.h"
#include "../include/Logger.h" // LOGGER EKLENDİ

#include <windows.h>
// <iostream>'a artık burada gerek kalmadı, Logger hallediyor.

ProxyServer* globalServer = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal)
{
    if (signal == 0 || signal == 2)
    {
        logLine("\n[!] Program kapatiliyor...");
        logLine("[!] Windows proxy ayarlari eski haline getiriliyor (Guvenli Cikis)");

        if (globalServer != nullptr)
        {
            globalServer->setSystemProxy(false);
        }

        Sleep(500);
        ExitProcess(0);
        return TRUE;
    }
    return FALSE;
}

int main()
{
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    logLine("========================================");
    logLine("          NanoDPI Web Bypass            ");
    logLine("========================================");
    logLine("[*] Kapatmak icin X tusuna basabilirsiniz.");
    logLine("[*] Cikista internetiniz otomatik duzelecektir.");
    logLine("========================================");

    ProxyServer sunucu;
    globalServer = &sunucu;

    sunucu.setSystemProxy(true);
    sunucu.start();
    sunucu.setSystemProxy(false);

    return 0;
}