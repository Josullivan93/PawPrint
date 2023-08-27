/*
 * lis2mdl.h
 *
 *  Created on: Aug 26, 2023
 *      Author: J-OSu
 */

#ifndef INC_LIS2MDL_H_
#define INC_LIS2MDL_H_

#include"stm32l4xx_hal.h" /* Needed for I2C */


/*
 * DEFINES
 */

#define LIS2MDL_I2C_ADDR			0x3D /* Address Page 24*/

#define LIS2MDL_ID					0x40 /* Who Am I  Register Page 27*/

/* REGISTERS page 27 */

#define LIS2MDL_REG_OFFSET_X_L				 0x45
#define LIS2MDL_REG_OFFSET_X_H				 0x46
#define LIS2MDL_REG_OFFSET_Y_L				 0x47
#define LIS2MDL_REG_OFFSET_Y_H				 0x48
#define LIS2MDL_REG_OFFSET_Z_L				 0x49
#define LIS2MDL_REG_OFFSET_Z_H				 0x4A
#define LIS2MDL_REG_WHO_AM_I				 0x4F
#define LIS2MDL_REG_CFG_REG_A				 0x60
#define LIS2MDL_REG_CFG_REG_B				 0x61
#define LIS2MDL_REG_CFG_REG_C				 0x62
#define LIS2MDL_REG_INT_CRTL				 0x63
#define LIS2MDL_REG_INT_SOURCE				 0x64
#define LIS2MDL_REG_INT_THS_L				 0x65
#define LIS2MDL_REG_INT_THS_H				 0x66
#define LIS2MDL_REG_STATUS					 0x67
#define LIS2MDL_REG_OUTX_L					 0x68
#define LIS2MDL_REG_OUTX_H					 0x69
#define LIS2MDL_REG_OUTY_L					 0x6A
#define LIS2MDL_REG_OUTY_H					 0x6B
#define LIS2MDL_REG_OUTZ_L					 0x6C
#define LIS2MDL_REG_OUTZ_H					 0x6D
#define LIS2MDL_REG_TEMP_OUT_L				 0x6E
#define LIS2MDL_REG_TEMP_OUT_H				 0x6F

/* Low level functions */
HAL_StatusTypeDef	LIS2MDL_ReadReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef	LIS2MDL_ReadRegs( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data, uint8_t len );

HAL_StatusTypeDef	LIS2MDL_WriteReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );

#endif /* INC_LIS2MDL_H_ */
