/*
 * lsm6dso.c
 *
 *  Created on: Aug 26, 2023
 *      Author: J-OSu
 */

#include "lsm6dso.h"

/*
 * Low level functions to read/write to LSM6DSO
 * Main functions held in PawPrint.c(.h)
 */

HAL_StatusTypeDef	LSM6DSO_ReadReg( I2C_HandleTypeDef *i2cHandle, uint8_t reg, uint8_t *data ){

	return HAL_I2C_Mem_Read( i2cHandle, LSM6DSO_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}

HAL_StatusTypeDef	LSM6DSO_ReadRegs( I2C_HandleTypeDef *i2cHandle, uint8_t reg, uint8_t *data, uint8_t len ){

	return HAL_I2C_Mem_Read( i2cHandle, LSM6DSO_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY );
}

HAL_StatusTypeDef	LSM6DSO_WriteReg( I2C_HandleTypeDef *i2cHandle, uint8_t reg, uint8_t *data ){

	return HAL_I2C_Mem_Write( i2cHandle, LSM6DSO_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}
