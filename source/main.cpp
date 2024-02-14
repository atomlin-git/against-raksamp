#include "../includes/headers.hpp"

//function, key, interface
unsigned int addresses[][3] = { { 0x487CC7, 0x4F6CF0, 0x4F6E34 }, { 0x48A4A7, 0x4F5FE8, 0x515D04 }, { 0x48A4A7, 0x4F5FE8, 0x515D04 }, { 0x485707, 0x4F43F0, 0x4F455C }, { 0x485CC7, 0x4F4BE0, 0x4F4D4C } };
unsigned char version = 0;

using send_packet_t = bool(__thiscall*)(void*, RakNet::BitStream*, unsigned int, unsigned int, char, unsigned int, unsigned short, bool);

void __cdecl AcceptConnectionRequest(void* remoteSystem)
{
    void* RakServerInterface = *reinterpret_cast<void**>(addresses[version][2]);
    *((unsigned long *)remoteSystem + 0x32C) = 5;

    RakNet::BitStream bitStream;
    bitStream.Write((unsigned char)34);
    bitStream.Write(*reinterpret_cast<unsigned int*>(static_cast<char*>(remoteSystem) + 1));
    bitStream.Write(*reinterpret_cast<unsigned short*>(static_cast<char*>(remoteSystem) + 5));
    bitStream.Write((unsigned short)65535);
    bitStream.Write(*reinterpret_cast<unsigned int*>(addresses[version][1]));

    send_packet_t SendPacket = nullptr;
    SendPacket = *reinterpret_cast<send_packet_t*>(*((unsigned long *)RakServerInterface) + 0x24);
    SendPacket(RakServerInterface, &bitStream, 0, 8, 0, *reinterpret_cast<unsigned int*>(static_cast<char*>(remoteSystem) + 1), *reinterpret_cast<unsigned short*>(static_cast<char*>(remoteSystem) + 5), false);
}

void Looping()
{
    unsigned int base = reinterpret_cast<unsigned int>(GetModuleHandleA(NULL));
    unsigned int hexVersion = reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew)->OptionalHeader.AddressOfEntryPoint;

    switch(hexVersion)
    {
        case 0x9B26A: version = 0; break; // R1
        case 0x9CB3A: version = 1; break; // R2
        case 0x9CC2A: version = 2; break; // R3
        case 0x988EA: version = 3; break; // RC1
        case 0x98F5A: version = 4; break; // RC2
        default: return;
    }

    Hook::SET_CALL_HOOK(addresses[version][0], &AcceptConnectionRequest);
    
    while(1) Sleep(25);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            std::thread(Looping).detach();
            break;
        }
    }

    return TRUE;
}