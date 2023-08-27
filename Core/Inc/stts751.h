/*
 * stts751.h
 *
 *  Created on: Aug 26, 2023
 *      Author: J-OSu
 */

#ifndef INC_STTS751_H_
#define INC_STTS751_H_

#include"stm32l4xx_hal.h" /* Needed for I2C */

/*
 * DEFINES
 */

#define STTS751_I2C_ADDR								0x72 /* Address = 0x72 */

#define STTS751_PROD_ID									0x00 /* Product ID  Register Page 21*/
#define STTS751_MAN_ID									0x53 /* Manufacturers ID  Register Page 21*/

/* REGISTERS page 13 */

#define STTS751_REG_TEMP_H								0x00
#define STTS751_REG_STATUS								0x01
#define STTS751_REG_TEMP_L								0x02
#define STTS751_REG_CONFIG								0x03
#define STTS751_REG_CONV_RATE							0x04
#define STTS751_REG_TEMP_H_LIMIT_H						0x05
#define STTS751_REG_TEMP_H_LIMIT_L						0x06
#define STTS751_REG_TEMP_L_LIMIT_H						0x07
#define STTS751_REG_TEMP_L_LIMIT_L						0x08
#define STTS751_REG_ONE_SHOT							0x0F
#define STTS751_REG_THERM_LIMIT							0x20
#define STTS751_REG_THERM_HYSTERESIS					0x21
#define STTS751_REG_SMBUS_TIMEOUT_ENABLE				0x22
#define STTS751_REG_PROD_ID								0xFD
#define STTS751_REG_MAN_ID								0xFE
#define STTS751_REG_REV_NUM								0xFF

/* Low level functions */
HAL_StatusTypeDef	STTS751_ReadReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef	STTS751_ReadRegs( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data, uint8_t len );

HAL_StatusTypeDef	STTS751_WriteReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );

#endif /* INC_STTS751_H_ */
