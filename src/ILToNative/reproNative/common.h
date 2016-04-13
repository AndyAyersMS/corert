// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// common.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __COMMON_H
#define __COMMON_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

#include <new>

#ifndef WIN32
#include <pthread.h>
#include <alloca.h>
#endif

using namespace std;

class MethodTable;
class Object;

template <typename T>
class Array
{
};

int __initialize_runtime();
void __shutdown_runtime();

extern "C" Object * __allocate_object(MethodTable * pMT);
extern "C" Object * __allocate_array(size_t elements, MethodTable * pMT);
Object * __allocate_string(int32_t len);
__declspec(noreturn) void __throw_exception(void * pEx);
Object * __load_string_literal(const char * string);

extern "C" Object * __castclass_class(void * p, MethodTable * pMT);
extern "C" Object * __isinst_class(void * p, MethodTable * pMT);

extern "C" void __range_check_fail();
void __range_check(void * a, size_t elem);

Object * __get_commandline_args(int argc, char * argv[]);

namespace AsmDataFormat
{
    typedef uint8_t byte;
    typedef uint32_t UInt32;

    static UInt32 ReadUInt32(byte **ppStream)
    {
        UInt32 result = *(UInt32*)(*ppStream); // Assumes little endian and unaligned access
        *ppStream += 4;
        return result;
    }
    static int DecodeUnsigned(byte** ppStream, byte* pStreamEnd, UInt32 *pValue)
    {
        if (*ppStream >= pStreamEnd)
            return -1;

        UInt32 value = 0;
        UInt32 val = **ppStream;
        if ((val & 1) == 0)
        {
            value = (val >> 1);
            *ppStream += 1;
        }
        else if ((val & 2) == 0)
        {
            if (*ppStream + 1 >= pStreamEnd)
                return -1;

            value = (val >> 2) |
                (((UInt32)*(*ppStream + 1)) << 6);
            *ppStream += 2;
        }
        else if ((val & 4) == 0)
        {
            if (*ppStream + 2 >= pStreamEnd)
                return -1;

            value = (val >> 3) |
                (((UInt32)*(*ppStream + 1)) << 5) |
                (((UInt32)*(*ppStream + 2)) << 13);
            *ppStream += 3;
        }
        else if ((val & 8) == 0)
        {
            if (*ppStream + 3 >= pStreamEnd)
                return -1;

            value = (val >> 4) |
                (((UInt32)*(*ppStream + 1)) << 4) |
                (((UInt32)*(*ppStream + 2)) << 12) |
                (((UInt32)*(*ppStream + 3)) << 20);
            *ppStream += 4;
        }
        else if ((val & 16) == 0)
        {
            if (*ppStream + 4 >= pStreamEnd)
                return -1;
            *ppStream += 1;
            value = ReadUInt32(ppStream);
        }
        else
        {
            return -1;
        }

        *pValue = value;
        return 0;
    }
}

// POD version of EEType to use for static initialization
struct RawEEType
{
    uint16_t    m_componentSize;
    uint16_t    m_flags;
    uint32_t    m_baseSize;
    MethodTable * m_pBaseType;
    uint16_t    m_usNumVtableSlots;
    uint16_t    m_usNumInterfaces;
    uint32_t    m_uHashCode;
};

#if !USE_MRT
#define EEType MethodTable
#endif

struct ReversePInvokeFrame
{
    void*   m_savedPInvokeTransitionFrame;
    void*   m_savedThread;
};

void __reverse_pinvoke(ReversePInvokeFrame* pRevFrame);
void __reverse_pinvoke_return(ReversePInvokeFrame* pRevFrame);

struct StaticGcDesc
{
    struct GCSeries
    {
        uint32_t m_size;
        uint32_t m_startOffset;
    };

    uint32_t m_numSeries;
    GCSeries m_series[0];
};

struct SimpleModuleHeader
{
    void* m_pStaticsGcDataSection;
    StaticGcDesc* m_pStaticsGcInfo;
    StaticGcDesc* m_pThreadStaticsGcInfo;
};

void __register_module(SimpleModuleHeader* pModule);

// TODO: this might be wrong...
typedef size_t UIntNative;

inline bool IS_ALIGNED(UIntNative val, UIntNative alignment)
{
    //ASSERT(0 == (alignment & (alignment - 1)));
    return 0 == (val & (alignment - 1));
}

template <typename T>
inline bool IS_ALIGNED(T* val, UIntNative alignment)
{
    //ASSERT(0 == (alignment & (alignment - 1)));
    return IS_ALIGNED(reinterpret_cast<UIntNative>(val), alignment);
}

#pragma warning(disable:4102)

#define RAW_MIN_OBJECT_SIZE (3*sizeof(void*))

#define AlignBaseSize(s) ((s < RAW_MIN_OBJECT_SIZE) ? RAW_MIN_OBJECT_SIZE : ((s + (sizeof(intptr_t)-1) & ~(sizeof(intptr_t)-1))))

#define ARRAY_BASE (2*sizeof(void*))

#endif // __COMMON_H
