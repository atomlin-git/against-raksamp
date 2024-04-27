#include "../includes/headers.hpp"

//function, key, interface
unsigned int addresses[][3] = { { 0x487CC7, 0x4F6CF0, 0x4F6E34 }, { 0x48A4A7, 0x4F5FE8, 0x515D04 }, { 0x48A4A7, 0x4F5FE8, 0x515D04 }, { 0x485707, 0x4F43F0, 0x4F455C }, { 0x485CC7, 0x4F4BE0, 0x4F4D4C } };
unsigned char version = 0;

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

extern "C" bool __stdcall Load(void **ppData)
{
    unsigned int hexVersion = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<unsigned int>(GetModuleHandleA(NULL)) + reinterpret_cast<IMAGE_DOS_HEADER*>(reinterpret_cast<unsigned int>(GetModuleHandleA(NULL)))->e_lfanew)->OptionalHeader.AddressOfEntryPoint;
    if (vers.find(hexVersion) == vers.end()) return 0; 
    version = vers[hexVersion];
    Hook::call(addresses[version][0], &AcceptConnectionRequest);
    return 1;
}

extern "C" unsigned int __stdcall Supports() { return 0x0200 | 0x10000; };