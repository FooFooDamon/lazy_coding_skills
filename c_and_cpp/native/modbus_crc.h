/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Generation algorithm of Modbus CRC (Cyclic Redundancy Check) code.
 *
 * Copyright (c) 2026 Man Hung-Coeng <udc577@126.com>
 * All rights reserved.
 */

#ifndef __MODBUS_CRC_H__
#define __MODBUS_CRC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_BIT_REVERSED_POLYNOMINAL     0xA001
#define MODBUS_CRC_INIT_VALUE               0xFFFF

static inline uint16_t modbus_crc(const uint8_t *data, uint16_t len)
{
    uint16_t crc = MODBUS_CRC_INIT_VALUE;
    uint8_t lsb;

    for (uint16_t i = 0; i < len; ++i)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
        {
            lsb = (crc & 0x0001);
            crc >>= 1;
            if (lsb)
                crc ^= MODBUS_BIT_REVERSED_POLYNOMINAL;
        }
    }

    return crc;
}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __MODBUS_CRC_H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2026-11-18, Man Hung-Coeng <udc577@126.com>:
 *  01. Initial commit.
 */

