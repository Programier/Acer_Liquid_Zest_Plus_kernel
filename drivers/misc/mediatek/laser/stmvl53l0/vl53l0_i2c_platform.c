/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

/*!
 * \file   VL53L0_platform.c
 * \brief  Code function defintions for Doppler Testchip Platform Layer
 *
 */


//#include <windows.h>
//#include <stdio.h>    // sprintf(), vsnprintf(), printf()

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "vl53l0_i2c_platform.h"
#include "vl53l0_def.h"

//#include "ranging_sensor_comms.h"
//#include "comms_platform.h"

#include "vl53l0_platform_log.h"

#ifdef VL53L0_LOG_ENABLE
#define trace_print(level, ...) trace_print_module_function(TRACE_MODULE_PLATFORM, level, TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) trace_print_module_function(TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif

/**
 * @def I2C_BUFFER_CONFIG
 *
 * @brief Configure Device register I2C access
 *
 * @li 0 : one GLOBAL buffer \n
 *   Use one global buffer of MAX_I2C_XFER_SIZE byte in data space \n
 *   This solution is not multi-Device compliant nor multi-thread cpu safe \n
 *   It can be the best option for small 8/16 bit MCU without stack and limited ram  (STM8s, 80C51 ...)
 *
 * @li 1 : ON_STACK/local \n
 *   Use local variable (on stack) buffer \n
 *   This solution is multi-thread with use of i2c resource lock or mutex see VL6180x_GetI2CAccess() \n
 *
 * @li 2 : User defined \n
 *    Per Device potentially dynamic allocated. Requires VL6180x_GetI2cBuffer() to be implemented.
 * @ingroup Configuration
 */
#define I2C_BUFFER_CONFIG 1
/** Maximum buffer size to be used in i2c */
#define VL53L0_MAX_I2C_XFER_SIZE   64 /* Maximum buffer size to be used in i2c */

#if I2C_BUFFER_CONFIG == 0
    /* GLOBAL config buffer */
    uint8_t i2c_global_buffer[VL53L0_MAX_I2C_XFER_SIZE];

    #define DECL_I2C_BUFFER
    #define VL53L0_GetLocalBuffer(Dev, n_byte)  i2c_global_buffer

#elif I2C_BUFFER_CONFIG == 1
    /* ON STACK */
    uint8_t LocBuffer[VL53L0_MAX_I2C_XFER_SIZE];
    #define VL53L0_GetLocalBuffer(Dev, n_byte)  LocBuffer
#elif I2C_BUFFER_CONFIG == 2
    /* user define buffer type declare DECL_I2C_BUFFER  as access  via VL53L0_GetLocalBuffer */
    #define DECL_I2C_BUFFER
#else
#error "invalid I2C_BUFFER_CONFIG "
#endif


char  debug_string[VL53L0_MAX_STRING_LENGTH_PLT];


#define STATUS_OK              0x00
#define STATUS_FAIL            0x01

extern int VL53L0_I2CWrite(uint8_t *buff, uint8_t len);
extern int VL53L0_I2CRead(uint8_t *buff, uint8_t len);

int32_t VL53L0_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
	int32_t status = STATUS_OK;
	uint8_t *buffer;

#ifdef VL53L0_LOG_ENABLE
	int32_t i = 0;
	char value_as_str[VL53L0_MAX_STRING_LENGTH_PLT];
	char *pvalue_as_str;

	pvalue_as_str =  value_as_str;

	for (i = 0 ; i < count ; i++) {
		snprintf(pvalue_as_str, sizeof(pvalue_as_str),
			"%02X", *(pdata + i));

		pvalue_as_str += 2;
	}
	trace_i2c("Write reg : 0x%04X, Val : 0x%s\n", index, value_as_str);
#endif
	if ((count + 1) > VL53L0_MAX_I2C_XFER_SIZE)
		return STATUS_FAIL;
	buffer =  VL53L0_GetLocalBuffer(NULL, (count+1));
	buffer[0] = index;
	memcpy(&buffer[1], pdata, count);
	status = VL53L0_I2CWrite(buffer, (count+1));

	return status;
}

int32_t VL53L0_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
	int32_t status = STATUS_OK;
	uint8_t *buffer;

#ifdef VL53L0_LOG_ENABLE
	int32_t 	 i = 0;
	char   value_as_str[VL53L0_MAX_STRING_LENGTH_PLT];
	char *pvalue_as_str;
#endif

	if ((count + 1) > VL53L0_MAX_I2C_XFER_SIZE)
		return STATUS_FAIL;

	buffer =  VL53L0_GetLocalBuffer(NULL, 1);
	buffer[0] = index;
	status = VL53L0_I2CWrite((uint8_t *)buffer, (uint8_t)1);
	if (!status) {
		pdata[0] = index;
		status = VL53L0_I2CRead( pdata, count);
	}

#ifdef VL53L0_LOG_ENABLE
	pvalue_as_str =  value_as_str;

	for (i = 0 ; i < count ; i++) {
		snprintf(pvalue_as_str, sizeof(value_as_str),
			"%02X", *(pdata+i));
		pvalue_as_str += 2;
	}

	trace_i2c("Read  reg : 0x%04X, Val : 0x%s\n", index, value_as_str);
#endif

	return status;
}



int32_t VL53L0_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    int32_t status = STATUS_OK;
    const int32_t cbyte_count = 1;

    status = VL53L0_write_multi(address, index, &data, cbyte_count);

    return status;

}


int32_t VL53L0_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    int32_t status = STATUS_OK;

    uint8_t  buffer[BYTES_PER_WORD];

    // Split 16-bit word into MS and LS uint8_t
    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    status = VL53L0_write_multi(address, index, buffer, BYTES_PER_WORD);

    return status;

}


int32_t VL53L0_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    int32_t status = STATUS_OK;
    uint8_t  buffer[BYTES_PER_DWORD];

    // Split 32-bit word into MS ... LS bytes
    buffer[0] = (uint8_t) (data >> 24);
    buffer[1] = (uint8_t)((data &  0x00FF0000) >> 16);
    buffer[2] = (uint8_t)((data &  0x0000FF00) >> 8);
    buffer[3] = (uint8_t) (data &  0x000000FF);

    status = VL53L0_write_multi(address, index, buffer, BYTES_PER_DWORD);

    return status;

}


int32_t VL53L0_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    int32_t status = STATUS_OK;
    int32_t cbyte_count = 1;

    status = VL53L0_read_multi(address, index, pdata, cbyte_count);

    return status;

}


int32_t VL53L0_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    int32_t  status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_WORD];

    status = VL53L0_read_multi(address, index, buffer, BYTES_PER_WORD);
	*pdata = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];

    return status;

}

int32_t VL53L0_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    int32_t status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_DWORD];

    status = VL53L0_read_multi(address, index, buffer, BYTES_PER_DWORD);
    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    return status;

}




