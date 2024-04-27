#pragma once

#include <cstring>
#include <stdlib.h>

#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

typedef signed		__int64 int64_t;
typedef unsigned	__int64 uint64_t;

namespace RakNet
{
    class BitStream
    {
        public:
            ~BitStream()
            {
                if (copyData && numberOfBitsAllocated > 256 << 3) free(data);
            }

            BitStream() : numberOfBitsUsed(0), readOffset(0), numberOfBitsAllocated(2048), data((unsigned char*)stackData), copyData(true) {};
            BitStream(char* _data, unsigned int lengthInBytes, bool _copyData)
            {
                if (lengthInBytes <= 0) return;

                numberOfBitsUsed = lengthInBytes << 3;
                readOffset = 0;
                copyData = _copyData;
                numberOfBitsAllocated = lengthInBytes << 3;

                if(!copyData) { data = (unsigned char*)_data; memcpy(data, _data, lengthInBytes); return; }
                if(copyData && lengthInBytes <= 0) { data = 0; memcpy(data, _data, lengthInBytes); return; };
                if(lengthInBytes > 256) { data = (unsigned char*)malloc(lengthInBytes); memcpy(data, _data, lengthInBytes); return; }

                data = (unsigned char*)stackData;
                numberOfBitsAllocated = 256 << 3;

                memcpy(data, _data, lengthInBytes);
            }

            void Reset(void) { numberOfBitsUsed = 0; readOffset = 0; };

            int CopyData( unsigned char** _data ) const
            {
                *_data = new unsigned char [ BITS_TO_BYTES( numberOfBitsUsed ) ];
                memcpy( *_data, data, sizeof(unsigned char) * ( BITS_TO_BYTES( numberOfBitsUsed ) ) );
                return numberOfBitsUsed;
            }

            template <class templateType> void Write(templateType var);
            template <class templateType> void WriteCompressed(templateType var);
            template <class templateType> bool Read(templateType& var);
            template <class templateType> bool ReadCompressed(templateType& var);

            void WriteBool(const bool input)
            {
                AddBitsAndReallocate(1);

                if (input)
                {
                    if ((numberOfBitsUsed % 8) == 0)
                        data[numberOfBitsUsed >> 3] = 0x80;
                    else data[numberOfBitsUsed >> 3] |= 0x80 >> (numberOfBitsUsed % 8);
                } else { if ((numberOfBitsUsed % 8) == 0) data[numberOfBitsUsed >> 3] = 0; } 

                numberOfBitsUsed++;
            }

            void WriteFloat(const float input)
            {
                unsigned int intval = *((unsigned int*)(&input));
                static unsigned char uint32w[4]{};

                uint32w[3] = (intval >> 24) & (0x000000ff);
                uint32w[2] = (intval >> 16) & (0x000000ff);
                uint32w[1] = (intval >> 8) & (0x000000ff);
                uint32w[0] = (intval) & (0x000000ff);

                WriteBits(uint32w, sizeof(intval) * 8, true);
            }

            void WriteString(const char* input, const int numberOfBytes)
            {
                if ((numberOfBitsUsed & 7) == 0)
                {
                    AddBitsAndReallocate(BYTES_TO_BITS(numberOfBytes));
                    memcpy(data + BITS_TO_BYTES(numberOfBitsUsed), input, numberOfBytes);
                    numberOfBitsUsed += BYTES_TO_BITS(numberOfBytes);
                } else WriteBits((unsigned char*)input, numberOfBytes * 8, true);
            }

            void WriteBitStream(const BitStream* bitStream) { WriteBits(bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), false); }

            bool ReadFloat(float& output)
            {
                unsigned int val = 0;
                if (!Read(val)) return false;
                output = *((float*)(&val));
                return true;
            }

            bool ReadBool(bool& output)
            {
                if (readOffset + 1 > numberOfBitsUsed) return false;
                if (data[readOffset >> 3] & (0x80 >> (readOffset++ % 8)))
                    output = true;
                else output = false;
                return true;
            }

            bool ReadString(char* output, const int numberOfBytes) { return ReadBits((unsigned char*)output, numberOfBytes * 8); }
            bool ReadCompressedFloat(float& output) { return Read(output); };
            bool ReadCompressed(double& output) { return Read(output); }

            void ResetReadPointer(void) { readOffset = 0; }
            void ResetWritePointer(void) { numberOfBitsUsed = 0; }

            void IgnoreBits(const int numberOfBits) { readOffset += numberOfBits; }
            void SetWriteOffset(const int offset) { numberOfBitsUsed = offset; }

            int GetReadOffset(void) const { return readOffset; }
            int GetNumberOfBitsUsed(void) const { return numberOfBitsUsed; }
            int GetNumberOfBytesUsed(void) const { return BITS_TO_BYTES(numberOfBitsUsed); }
            int GetNumberOfUnreadBits(void) const { return numberOfBitsUsed - readOffset; }
            unsigned char* GetData(void) const { return data; }

            void SetData(const unsigned char* input, const int numberOfBits)
            {
                if (numberOfBits <= 0) return;
                AddBitsAndReallocate(numberOfBits);
                memcpy(data, input, BITS_TO_BYTES(numberOfBits));
                numberOfBitsUsed = numberOfBits;
            }

            void WriteBits(const unsigned char* input, int numberOfBitsToWrite, const bool rightAlignedBits = true)
            {
                if(!numberOfBitsToWrite || input == nullptr) return;

                AddBitsAndReallocate(numberOfBitsToWrite);

                int offset = 0;
                unsigned char dataByte;
                int numberOfBitsUsedMod8 = numberOfBitsUsed % 8;

                while (numberOfBitsToWrite > 0)
                {
                    dataByte = *(input + offset);
                    if (numberOfBitsToWrite < 8 && rightAlignedBits) dataByte <<= 8 - numberOfBitsToWrite;

                    if (numberOfBitsUsedMod8 == 0)
                    {
                        *(data + (numberOfBitsUsed >> 3)) = dataByte;
                    }
                    else
                    {
                        *(data + (numberOfBitsUsed >> 3)) |= dataByte >> (numberOfBitsUsedMod8);

                        if (8 - (numberOfBitsUsedMod8) < 8 && 8 - (numberOfBitsUsedMod8) < numberOfBitsToWrite)
                            *(data + (numberOfBitsUsed >> 3) + 1) = (unsigned char)(dataByte << (8 - (numberOfBitsUsedMod8)));
                    }

                    if (numberOfBitsToWrite >= 8)
                        numberOfBitsUsed += 8;
                    else numberOfBitsUsed += numberOfBitsToWrite;

                    numberOfBitsToWrite -= 8;
                    offset++;
                }
            }

            void WriteAlignedBytes(const unsigned char* input, const int numberOfBytesToWrite)
            {
                AlignWriteToByteBoundary();
                AddBitsAndReallocate(numberOfBytesToWrite << 3);
                memcpy(data + (numberOfBitsUsed >> 3), input, numberOfBytesToWrite);
                numberOfBitsUsed += numberOfBytesToWrite << 3;
            }

            bool ReadAlignedBytes(unsigned char* output, const int numberOfBytesToRead)
            {
                if (numberOfBytesToRead <= 0) return false;
                if (GetNumberOfUnreadBits() < (numberOfBytesToRead << 3)) return false;

                AlignReadToByteBoundary();
                memcpy(output, data + (readOffset >> 3), numberOfBytesToRead);
                readOffset += numberOfBytesToRead << 3;

                return true;
            }

            void AlignWriteToByteBoundary(void) { if (numberOfBitsUsed) numberOfBitsUsed += 8 - ((numberOfBitsUsed - 1) % 8 + 1); }
            void AlignReadToByteBoundary(void) { if (readOffset) readOffset += 8 - ((readOffset - 1) % 8 + 1); }

            bool ReadBit(void)
            {
                #pragma warning( disable : 4800 )
                    return (bool)(data[readOffset >> 3] & (0x80 >> (readOffset++ % 8)));
                #pragma warning( default : 4800 )
            }

            bool ReadBits(unsigned char* output, int numberOfBitsToRead, const bool alignBitsToRight = true)
            {
                if (readOffset + numberOfBitsToRead > numberOfBitsUsed) return false;

                int readOffsetMod8 = readOffset % 8;
                int offset = 0;

                memset(output, 0, BITS_TO_BYTES(numberOfBitsToRead));

                while (numberOfBitsToRead > 0)
                {
                    *(output + offset) |= *(data + (readOffset >> 3)) << (readOffsetMod8);
                    if (readOffsetMod8 > 0 && numberOfBitsToRead > 8 - (readOffsetMod8))
                        *(output + offset) |= *(data + (readOffset >> 3) + 1) >> (8 - (readOffsetMod8));

                    numberOfBitsToRead -= 8;

                    if (numberOfBitsToRead < 0)
                    {
                        if (alignBitsToRight) *(output + offset) >>= -numberOfBitsToRead;
                        readOffset += 8 + numberOfBitsToRead;
                    } else readOffset += 8;

                    offset++;
                }

                return true;
            }

            void SetNumberOfBitsAllocated(const unsigned int lengthInBits) { numberOfBitsAllocated = lengthInBits; }
        private:
            void WriteCompressed(const unsigned char* input, const int size, const bool unsignedData)
            {
                int currentByte = (size >> 3) - 1;
                unsigned char byteMatch;

                if (unsignedData)
                    byteMatch = 0;
                else byteMatch = 0xFF;

                while (currentByte > 0)
                {
                    if (input[currentByte] == byteMatch)
                    {
                        bool b = true;
                        WriteBool(b);
                    }
                    else
                    {
                        bool b = false;
                        WriteBool(b);
                        WriteBits(input, (currentByte + 1) << 3, true);
                        return;
                    }
                    currentByte--;
                }

                if ((unsignedData && ((*(input + currentByte)) & 0xF0) == 0x00) || (unsignedData == false && ((*(input + currentByte)) & 0xF0) == 0xF0))
                {
                    bool b = true;
                    WriteBool(b);
                    WriteBits(input + currentByte, 4, true);
                }
                else
                {
                    bool b = false;
                    WriteBool(b);
                    WriteBits(input + currentByte, 8, true);
                }
            }

            bool ReadCompressed(unsigned char* output, const int size, const bool unsignedData)
            {
                int currentByte = (size >> 3) - 1;
                unsigned char byteMatch, halfByteMatch;

                if (unsignedData)
                {
                    byteMatch = 0;
                    halfByteMatch = 0;
                }
                else
                {
                    byteMatch = 0xFF;
                    halfByteMatch = 0xF0;
                }

                while (currentByte > 0)
                {
                    bool b;
                    if (ReadBool(b) == false) return false;

                    if (b)
                    {
                        output[currentByte] = byteMatch;
                        currentByte--;
                    }
                    else
                    {
                        if (ReadBits(output, (currentByte + 1) << 3) == false) return false;
                        return true;
                    }
                }

                if (readOffset + 1 > numberOfBitsUsed) return false;

                bool b;

                if (ReadBool(b) == false) return false;
                if (b)
                {
                    if (ReadBits(output + currentByte, 4) == false) return false;
                    output[currentByte] |= halfByteMatch;
                }
                else { if (ReadBits(output + currentByte, 8) == false) return false; }

                return true;
            }

            void AddBitsAndReallocate(const int numberOfBitsToWrite)
            {
                if (numberOfBitsToWrite <= 0) return;
                int newNumberOfBitsAllocated = numberOfBitsToWrite + numberOfBitsUsed;

                if (numberOfBitsToWrite + numberOfBitsUsed > 0 && ((numberOfBitsAllocated - 1) >> 3) < ((newNumberOfBitsAllocated - 1) >> 3))
                {
                    newNumberOfBitsAllocated = (numberOfBitsToWrite + numberOfBitsUsed) * 2;

                    int amountToAllocate = BITS_TO_BYTES(newNumberOfBitsAllocated);
                    if (data == (unsigned char*)stackData)
                    {
                        if (amountToAllocate > 256)
                        {
                            data = (unsigned char*)malloc(amountToAllocate);
                            memcpy((void*)data, (void*)stackData, BITS_TO_BYTES(numberOfBitsAllocated));
                        }
                    } else data = (unsigned char*)realloc(data, amountToAllocate);
                }

                if (newNumberOfBitsAllocated > numberOfBitsAllocated) numberOfBitsAllocated = newNumberOfBitsAllocated;
            }

            int numberOfBitsUsed;
            int numberOfBitsAllocated;
            int readOffset;

            unsigned char* data;

            bool copyData;

            unsigned char stackData[256]{};
    };

    template <class templateType>
    inline void BitStream::Write(templateType var)
    {
        WriteBits((unsigned char*)&var, sizeof(templateType) * 8, true);
    }

    template <class templateType>
    inline void BitStream::WriteCompressed(templateType var)
    {
        WriteCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
    }

    template <class templateType>
    inline bool BitStream::Read(templateType& var)
    {
        return ReadBits((unsigned char*)&var, sizeof(templateType) * 8, true);
    }

    template <class templateType>
    inline bool BitStream::ReadCompressed(templateType& var)
    {
        return ReadCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
    }
}