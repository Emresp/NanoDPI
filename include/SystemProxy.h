#pragma once
#ifndef NANODPI_SYSTEMPROXY_H
#define NANODPI_SYSTEMPROXY_H

#include <string>
#include <windows.h>

struct ProxyBackup
{
    bool valid = false;

    DWORD proxyEnable = 0;

    std::string proxyServer;
    std::string proxyOverride;
    std::string autoConfigUrl;

    bool hasProxyServer = false;
    bool hasProxyOverride = false;
    bool hasAutoConfigUrl = false;
};

bool backupCurrentProxy(ProxyBackup& backup);
bool enableNanoSystemProxy();
bool restoreSystemProxy(const ProxyBackup& backup);

#endif //NANODPI_SYSTEMPROXY_H