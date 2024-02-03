#include "../includes/headers.hpp"

using send_packet_t = bool(__thiscall*)(void*, RakNet::BitStream*, unsigned int, unsigned int, char, unsigned int, unsigned short, bool);

void __cdecl AcceptConnectionRequest(void* remoteSystem)
{
    *((unsigned long *)remoteSystem + 0x32C) = 5;

    RakNet::BitStream bitStream;
    bitStream.Write((unsigned char)34);
    bitStream.Write(*reinterpret_cast<unsigned int*>(static_cast<char*>(remoteSystem) + 1));
    bitStream.Write(*reinterpret_cast<unsigned short*>(static_cast<char*>(remoteSystem) + 5));
    bitStream.Write((unsigned short)65535);
    bitStream.Write(*reinterpret_cast<unsigned int*>(0x4F5FE8));

    void* RakServerInterface = *reinterpret_cast<void**>(0x515D04);

    send_packet_t SendPacket = nullptr;
    SendPacket = *reinterpret_cast<send_packet_t*>(*((unsigned long *)RakServerInterface) + 0x24);
    SendPacket(RakServerInterface, &bitStream, 0, 8, 0, *reinterpret_cast<unsigned int*>(static_cast<char*>(remoteSystem) + 1), *reinterpret_cast<unsigned short*>(static_cast<char*>(remoteSystem) + 5), false);
}

void Looping()
{
    Hook::SET_CALL_HOOK(0x48A4A7, &AcceptConnectionRequest);
    
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