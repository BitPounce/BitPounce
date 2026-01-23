#pragma once

#include <stdint.h>
#include <cstring>
#include "BitPounce/Core/PlatformTools.h"
#include "RefObj.h"

namespace BitPounce
{
    struct Buffer;

    class BufferBase : public RefObj
    {
        public:
        uint8_t* Data = nullptr;
        uint64_t Size = 0;

        virtual ~BufferBase()
        {  
            Set(nullptr);
        }

        template<typename T>
        T* As()
        {
            return (T*)Data;
        }

        template<typename T>
        const T* As() const
        {
            return (const T*)Data;
        }

        operator bool() const
        {
            return Data && Size > 0;
        }

        virtual void Free() override
        {

        }
    };

    class DiskBuffer : public BufferBase
    {
        public:
        DiskBuffer() = default;

        DiskBuffer(uint64_t size)
        {
            Allocate(size);
        }

        DiskBuffer(const DiskBuffer& other)
        {
            if (other.Data && other.Size)
            {
                Allocate(other.Size);
                DiskMemCpy(Data, other.Data, other.Size);
            }
            
        }

        virtual void Free() override
        {
            Release();
        }

        Buffer ToBuffer();

        static DiskBuffer Copy(const DiskBuffer& other)
        {
            DiskBuffer result(other.Size);
            DiskMemCpy(result.Data, other.Data, other.Size);
            return result;
        }

        void Allocate(uint64_t size)
        {
            Release();

            if (size == 0)
                return;

            Data = (uint8_t*)DiskAlloc(size, Data);
            Size = size;
            Set(Data);
        }

        void Release()
        {
            if (Data)
                DiskFree(Size, Data);

            Data = nullptr;
            Size = 0;
            
        }
    };

    class Buffer : public BufferBase
    {
        public:
        Buffer() = default;

        Buffer(uint64_t size)
        {
            Allocate(size);
        }

        Buffer(const Buffer& other)
        {
            if (other.Data && other.Size)
            {
                Allocate(other.Size);
                memcpy(Data, other.Data, other.Size);
            }
        }

        virtual void Free() override
        {
            Release();
        }

        static Buffer Copy(const Buffer& other)
        {
            Buffer result(other.Size);
            memcpy(result.Data, other.Data, other.Size);
            return result;
        }

        DiskBuffer ToBuffer()
        {
            DiskBuffer result(Size);
            DiskMemCpy(result.Data, Data, Size);
            return result;
        }

        void Allocate(uint64_t size)
        {
            Release();

            if (size == 0)
                return;

            Data = new uint8_t[size];
            Size = size;
            Set(Data);
        }

        void Release()
        {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }
    };

    inline Buffer DiskBuffer::ToBuffer()
    {
        Buffer result(Size);
        DiskMemCpy(result.Data, Data, Size);
        return result;
    }
}
