/*
 * Project : KIAPI cpp-framework
 * Author: WooChang Seo (wcseo@kiapi.or.kr) 
 * Date: 2025-10-04
 */  

#ifndef _ENDIAN_H_H____
#define _ENDIAN_H_H____

#include <endian.h>

// #include <bit>

#include <stdint.h>
#include <cstdint>
#include <iostream>
#include <cstring>

#include <type_traits>

 
// Big-endian 16비트 데이터 타입
struct be16
{
    uint16_t value;

    be16(uint16_t v) : value((v))
    {
    }

    operator uint16_t() const
    {
        return be16toh(value);
    } // 변환 연산자
    
    be16 operator=(uint16_t &v) const
    {
        return be16(htobe16(v));
    }
};

template <typename T>
struct BE {

    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, 
                  "BE only supports 16, 32, or 64-bit types.");

    T value;

    // 생성자: 빅엔디안 값 저장
    BE(T v = 0) {
        switch (sizeof(T)) {
        case 2:
            value = htobe16(v);
            break;
        case 4:
            value = htobe32(v);
            break;
        case 8:
            value = htobe64(v);
            break;
        default:
            throw std::logic_error("Unsupported size");
        }
    }

    BE(uint8_t *ptr)
    {
        //value = (T)*ptr;
        memcpy(&value,ptr,sizeof(T));
    }

    // 변환 연산자: 빅엔디안 -> 호스트 엔디안
    operator T() const {
        switch (sizeof(T)) {
        case 2:
            return be16toh(value);
        case 4:
            return be32toh(value);
        case 8:
            return be64toh(value);
        default:
            throw std::logic_error("Unsupported size");
        }
    }

    // 대입 연산자: 호스트 엔디안 -> 빅엔디안 변환
    BE& operator=(const T& v) {
        switch (sizeof(T)) {
        case 2:
            value = htobe16(v);
            break;
        case 4:
            value = htobe32(v);
            break;
        case 8:
            value = htobe64(v);
            break;
        default:
            throw std::logic_error("Unsupported size");
        }
        return *this;
    }
    BE& operator+=(const T& v) {
        switch (sizeof(T)) {
        case 2:
            value += htobe16(v);
            break;
        case 4:
            value += htobe32(v);
            break;
        case 8:
            value += htobe64(v);
            break;
        default:
            throw std::logic_error("Unsupported size");
        }
        return *this;
    }
 
}__attribute__((packed));




#endif