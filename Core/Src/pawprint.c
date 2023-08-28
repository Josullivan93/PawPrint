/*
 * pawprint.c - Rush to deploy resulted in single large function - this will be broken up in future for more discrete control
 *
 *  Created on: Aug 27, 2023
 *      Author: Jack O'Sullivan
 */

#include "usbd_cdc_if.h"
#include "math.h"
#include "st_fifo.h"
#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"
#include "pawprint.h"

/* Private Variables */

static uint8_t tx_buffer[1000];
static st_fifo_raw_slot raw_slot[126];
static st_fifo_out_slot out_slot[126];
static st_fifo_out_slot acc_slot[126];
static st_fifo_out_slot gyr_slot[126];

/* Initialisation */

void pawprint_init( I2C_HandleTypeDef *i2cHandle ){

	//Create required variables
	uint8_t LSM6DSO_WHO,MasterTemp, RegDat, LIS2MDL_WHO, STTS_PROD, STTS_MAN;

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
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);// Enable access to SHUB registers
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG, &MasterTemp);// Save Master Config to MasterTemp
	MasterTemp |= 0x20;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);// Start Config to 1 - sensor hub signal disable
	HAL_Delay(5);																							// Delay
	MasterTemp = (MasterTemp | 0x20) & ~(0x04);
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);// Master On bit to 0 to enable
	MasterTemp =  MasterTemp & ~(0x04) & ~(0x20);
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);// Start Config to 0 to restore trigger
	MasterTemp = MasterTemp & ~(0x04) & ~(0x20) & ~(0x08);
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &MasterTemp);// Set Pullup to 0 to disable internal pullup
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

	STTS751_ReadReg(i2cHandle, STTS751_REG_PROD_ID, &STTS_PROD); // ProdID is 00 for this version of therm - therefore also use manID
	STTS751_ReadReg(i2cHandle, STTS751_REG_MAN_ID, &STTS_MAN);

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

	/*** Disable Pass Through ***/
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);// Enable access to SHUB registers
	HAL_Delay(100);		// delay 100
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &RegDat);	// Disable pass through
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);	// Disable SHUB access

	/*** Configure LSM6DSO ***/
	RegDat = 0xE2;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL9_XL , &RegDat); // Disable I3C MIPI

	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL1_XL , &RegDat); // Ensure Acc and Gyro Powered down
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL2_G , &RegDat);

	/**** FIFO set up ****/
	RegDat = 0x80;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL1 , &RegDat);// Set Watermark level to 256 = 25% 512
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL2 , &RegDat);
	RegDat = 0x44;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL3 , &RegDat); // Set BDR for XL and Gyro (104Hz both)
	RegDat = 0x56;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL4 , &RegDat);// Set to continuous mode & batch Temp and Timestamp
	RegDat = 0x41;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_TAP_CFG0 , &RegDat); // Latch interrupt & clear on read
	RegDat = 0x38;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_INT1_CTRL , &RegDat); // FIFO int on INT1

	/*** Peripheral address / subaddress config ***/
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat); // Enable SHUB access

	// LIS2MDL Peripheral enable
	RegDat = LIS2MDL_I2C_ADDR;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV0_ADD, &RegDat);// Set address slv0
	RegDat = LIS2MDL_REG_OUTX_L;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV0_SUBADD, &RegDat);// Set start register for data output
	RegDat = 0x0E;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV0_CONFIG, &RegDat);// Set bytes to read

	// STTS751 Peripheral enable
	RegDat = STTS751_I2C_ADDR;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_ADD, &RegDat);// Set address slv1
	RegDat = STTS751_REG_TEMP_H;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_SUBADD, &RegDat);// Set start register for data output
	RegDat = 0x0B;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_CONFIG, &RegDat);// Set bytes to read - status between L and H registers so need to read 3
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);// Disable SHUB access

	/* Configure Xl and Gyro */
	RegDat = 0x44;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL3_C, &RegDat);// Enable block update and auto increment registers
	RegDat = 0x28;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL4_C, &RegDat);	// Int 1 and mask DRDY until Filter settles
	RegDat = 0x10;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL6_C, &RegDat);// XL High performance disabled
	RegDat = 0x80;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL7_G, &RegDat);// Gyro High performance disabled
	RegDat = 0x20;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL10_C, &RegDat);// enable timestamps
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL1_XL, &RegDat);//Set acc scale and sample rate  - ctrl 1 xl
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL2_G, &RegDat);// Gyro scale and sample rate - ctrl 2 g

	/* Enable I2C Master to begin Data collection */
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat); // Enable SHUB Reg Access
	RegDat = 0x09;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &RegDat);// Master config - enable pullups & accel trigger drdy & number external sensors
	RegDat = (RegDat ^ 0x04);
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &RegDat);// Master config - enable I2C Master
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);

}

void pawprint_readFIFO( I2C_HandleTypeDef *i2cHandle ){

	uint8_t FIFO_STATUS1 = 0, FIFO_STATUS2 = 0, FIFO_tag_reg = 0, FIFO_tag = 0, FIFO_counter_old = 0;
	uint16_t FIFO_Depth = 0, FIFO_Slots = 0, Out_FIFO_Slots, acc_samples;
	//lsm6dso_tags_t	lsm6dso_tags;

	/* Confirm watermark has been met LSM6DSO_REG_FIFO_STATUS2 */
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_STATUS2, &FIFO_STATUS2);
	HAL_Delay(1000);
	if (((FIFO_STATUS2 >> 7)  & 0x01)||((FIFO_STATUS2 >> 6)  & 0x01)||((FIFO_STATUS2 >> 5)  & 0x01)){ // Check WTM threshold bit, overrun bit and full bit
		/* Get number of samples in FIFO*/
		LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_STATUS1, &FIFO_STATUS1);
		FIFO_Depth = ((((uint16_t)FIFO_STATUS2 & 0x03) << 8) + (uint16_t)FIFO_STATUS1);
		/* Loop until FIFO Empty */
		while (FIFO_Depth--){
			/* Read Tag and store */
			LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_STATUS1, (uint8_t *)&raw_slot[FIFO_Slots].fifo_data_out[0]);
			/* Read Raw Data */
			LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_DATA_OUT_X_L, &raw_slot[FIFO_Slots].fifo_data_out[1],6 );
			//FIFO_tag = (FIFO_tag_reg >> 3); // Take tag register and right shift to leave only the tag
			FIFO_Slots++;
			//switch(FIFO_tag) {
			//case LSM6DSO_XL_NC_TAG:
				// Get data out
				// Convert to mg

		}
		/* Filter based on sensor type */
		st_fifo_decode(out_slot, raw_slot, &Out_FIFO_Slots, FIFO_Slots);
		st_fifo_sort(out_slot, Out_FIFO_Slots);
		acc_samples = st_fifo_get_sensor_occurrence(out_slot,Out_FIFO_Slots, ST_FIFO_ACCELEROMETER);

		 /* Count how many acc and gyro samples */
		      st_fifo_extract_sensor(acc_slot, out_slot, Out_FIFO_Slots,
		                             ST_FIFO_ACCELEROMETER);
		      st_fifo_extract_sensor(gyr_slot, out_slot, Out_FIFO_Slots,
		                             ST_FIFO_GYROSCOPE);

		      for (int i = 0; i < acc_samples; i++) {
		        sprintf((char *)tx_buffer, "ACC:\t%u\t%d\t%4.2f\t%4.2f\t%4.2f\r\n",
		                (unsigned int)acc_slot[i].timestamp,
		                acc_slot[i].sensor_tag,
		                lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.x),
		                lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.y),
		                lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.z));
		      }

	}


}

float_t lsm6dso_from_fs2_to_mg(int16_t lsb)
{
  return ((float_t)lsb) * 0.061f;
}


