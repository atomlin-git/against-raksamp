#pragma once

#include <iostream>
#include <unordered_map>
#include <windows.h>

#include "hook.hpp"
#include "bitstream.hpp"

std::unordered_map <unsigned int, unsigned char> vers = {
    {0x9B26A, 0}, // R1
    {0x9CB3A, 1}, // R2
    {0x9CC2A, 2}, // R3
    {0x988EA, 3}, // RC1
    {0x98F5A, 4}, // RC2
};

using send_packet_t = bool(__thiscall*)(void*, RakNet::BitStream*, unsigned int, unsigned int, char, unsigned int, unsigned short, bool);