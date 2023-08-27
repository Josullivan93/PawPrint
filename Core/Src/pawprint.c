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
	uint8_t LSM6DSO_WHO,MasterTemp, RegDat, LIS2MDL_WHO, STTS_PROD, STTS_MAN;

	// Create Struct instance

	// Boot Delay
	HAL_Delay(20);
	/* Reset Sensor to defaults*/
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat); // Ensure accessing correct registers
	RegDat = 0x05;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL3_C, &RegDat); // Software reset
	HAL_Delay(100);

	// Check LSM6DSO WhoAmI
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_WHO_AM_I, &LSM6DSO_WHO);

	/****** Enable PassThrough ******/
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

	/*** LIS2MDL Configuration ***/

	LIS2MDL_ReadReg(i2cHandle, LIS2MDL_REG_WHO_AM_I, &LIS2MDL_WHO);

	while( LIS2MDL_WHO != LIS2MDL_ID)

	/* Reset sensor to defaults */
	RegDat = 0x23;
	LIS2MDL_WriteReg(i2cHandle, LIS2MDL_REG_CFG_A, &RegDat); //Reset registers by soft rst of cfg A to 1
	HAL_Delay(5);
	RegDat = 0x43;
	LIS2MDL_WriteReg(i2cHandle, LIS2MDL_REG_CFG_A, &RegDat); //ReBOOT of cfg A to 1
	HAL_Delay(20);

	/* Setup */
	RegDat = 0x10;
	LIS2MDL_WriteReg(i2cHandle, LIS2MDL_REG_CFG_C, &RegDat);// bdu set 1
	RegDat = 0x8C;
	LIS2MDL_WriteReg(i2cHandle, LIS2MDL_REG_CFG_A, &RegDat);// offset temp on, operating mode Default, & data rate  100Hz

	/**** Insert Calibration Code here - Requires Moving and Input trigger via USB ******/

	/*** STTS Setup ***/

	STTS751_ReadReg(i2cHandle, STTS751_REG_PROD_ID, &STTSPROD); // ProdID is 00 for this version of therm - therefore also use manID
	STTS751_ReadReg(i2cHandle, STTS751_REG_MAN_ID, &STTSMAN);

	while( STTS_PROD != STTS751_PROD_ID && STTS_MAN != STTS751_MAN_ID)

	RegDat = 0x04;
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_CONV_RATE, &RegDat); // 1Hz
	RegDat = 0x4B;
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_TEMP_H_LIMIT_H, &RegDat); // Thermal Limit - default is 85 lowered to 75, low limit at freezing
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_THERM_LIMIT, &RegDat); //Therm limit for THERM pin - not used in this design but set regardless for consistency

	// No reset function - for safety ensure all thermal registers are set and not assumed to be correct
	RegDat = 0x00;
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_TEMP_H_LIMIT_L, &RegDat);
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_TEMP_L_LIMIT_H, &RegDat);
	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_TEMP_L_LIMIT_L, &RegDat);

	LIS2MDL_WriteReg(i2cHandle, STTS751_REG_CONFIG, &RegDat);// Mask1 enabled - event pin active (b7), Run/standby mode (b6)


}
