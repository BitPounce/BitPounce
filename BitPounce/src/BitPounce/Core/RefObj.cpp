#include "bp_pch.h"
#include <unordered_map>
#include <mutex>
#include "RefObj.h"

namespace BitPounce
{
    static std::unordered_map<void*, size_t> s_CountMap;
    static std::mutex s_Mutex;

    RefObj::RefObj()
        : m_ptr(nullptr)
    {
    }

    RefObj::RefObj(void* ptr)
        : m_ptr(nullptr)
    {
        Set(ptr);
    }

    RefObj::RefObj(const RefObj& other)
        : m_ptr(nullptr)
    {
        Set(other.m_ptr);
    }

    RefObj::RefObj(RefObj&& other) noexcept
        : m_ptr(nullptr)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
        if (m_ptr)
        {
            s_CountMap[m_ptr]++;
        }
    }

    RefObj& RefObj::operator=(const RefObj& other)
    {
        if (this != &other)
        {
            Set(other.m_ptr);
        }
        return *this;
    }

    RefObj& RefObj::operator=(RefObj&& other) noexcept
    {
        if (this != &other)
        {
            Set(nullptr);

            std::lock_guard<std::mutex> lock(s_Mutex);
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
            if (m_ptr)
            {
                s_CountMap[m_ptr]++;
            }
        }
        return *this;
    }

    RefObj::~RefObj()
    {
        Set(nullptr);
    }

    void* RefObj::Get()
    {
        return m_ptr;
    }

    void RefObj::Set(void* ptr)
    {
        std::lock_guard<std::mutex> lock(s_Mutex);

        if (m_ptr)
        {
            auto it = s_CountMap.find(m_ptr);
            if (it != s_CountMap.end())
            {
                if (--it->second == 0)
                {
                    s_CountMap.erase(it);
                    Free();
                }
            }
        }

        m_ptr = ptr;

        if (m_ptr)
        {
            s_CountMap[m_ptr]++;
        }
    }
}
