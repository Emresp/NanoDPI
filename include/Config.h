#pragma once
#ifndef NANODPI_CONFIG_H
#define NANODPI_CONFIG_H

namespace Config
{
    inline constexpr int NANO_PROXY_PORT = 8081;

    inline constexpr bool DPI_SPLIT_ENABLED = true;
    inline constexpr int DPI_SPLIT_POINT = 1;
    inline constexpr int DPI_SPLIT_DELAY_MS = 10;
}

#endif //NANODPI_CONFIG_H