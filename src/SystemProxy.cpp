#include "../include/SystemProxy.h"
#include "../include/Logger.h"

bool backupCurrentProxy(ProxyBackup& backup)
{
    logLine("[SystemProxy] Eski proxy ayarlari yedeklenecek");
    backup.valid = false;

    return true;
}

bool enableNanoSystemProxy()
{
    logLine("[SystemProxy] NanoDPI sistem proxy aktif edilecek");
    return true;
}

bool restoreSystemProxy(const ProxyBackup& backup)
{
    if (!backup.valid)
    {
        logLine("[SystemProxy] Geri yuklenecek proxy yedegi yok");
        return false;
    }

    logLine("[SystemProxy] Eski proxy ayarlari geri yuklenecek");
    return true;
}