/*
 * pawprint.c
 *
 *  Created on: Aug 27, 2023
 *      Author: J-OSu
 */

#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"
#include "pawprint.h"

void pawprint_init( I2C_HandleTypeDef *i2cHandle ){

	//Create required variables
	uint8_t WhoAmI,MasterTemp, RegDat;

	// Create Struct instance

	// Boot Delay
	HAL_Delay(10);
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);
	// Check LSM6DSO WhoAmI
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_WHO_AM_I, &WhoAmI);

	if( WhoAmI == LSM6DSO_ID ){
		RegDat = 0x40;
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat); 										// Enable access to SHUB registers
		LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG, &MasterTemp); 									// Save Master Config to MasterTemp
		MasterTemp = MasterTemp | 0x20;
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp); 							// Start Config to 1 - sensor hub signal disable
		HAL_Delay(5);																							// Delay
		MasterTemp = (MasterTemp | 0x20) & ~(0x04);
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);				// Master On bit to 0 to enable
		MasterTemp =  MasterTemp & ~(0x04) & ~(0x20);
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);				// Start Config to 0 to restore trigger
		MasterTemp = MasterTemp & ~(0x04) & ~(0x20) & ~(0x08);
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);		// Set Pullup to 0 to disable internal pullup
		MasterTemp = 0x10;
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);											// Pass Through Enabled
		RegDat = 0x00;
		LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);										// Disable access to SHUB registers

		LIS2MDL_ReadReg(i2cHandle, LIS2MDL_REG_WHO_AM_I, &WhoAmI);

		if( WhoAmI == LIS2MDL_ID){
			HAL_Delay(10);
		}

		STTS751_ReadReg(i2cHandle, STTS751_REG_PROD_ID, &WhoAmI);

		if( WhoAmI == STTS751_PROD_ID){

			STTS751_ReadReg(i2cHandle, STTS751_REG_MAN_ID, &WhoAmI);

			if (WhoAmI == STTS751_MAN_ID){
				HAL_Delay(10);
			}
		}

	}


}
