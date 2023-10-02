/*
 * pawprint.c - Rush to deploy resulted in single large function - this will be broken up in future for more discrete control
 *
 *  Created on: Aug 27, 2023
 *      Author: Jack O'Sullivan
 */

#include "usbd_cdc_if.h"
#include "stdio.h"
#include "math.h"
#include "fatfs.h"
#include "st_fifo.h"
#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"
#include "pawprint.h"

/* Private Variables */

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
	RegDat = 0x32;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL1 , &RegDat);// Set Watermark level to 50
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL2 , &RegDat);
	RegDat = 0x44;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL3 , &RegDat); // Set BDR for XL and Gyro (104Hz both)
	RegDat = 0x41;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL4 , &RegDat);//41: FIFO mode, bqtch Timestamp, no temp   56: Set to continuous mode & batch Temp and Timestamp
	RegDat = 0x00;
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
	RegDat = 0xCE;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV0_CONFIG, &RegDat);// Set bytes to read 0E Default 104Hz

	// STTS751 Peripheral enable
//	RegDat = STTS751_I2C_ADDR;
//	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_ADD, &RegDat);// Set address slv1
//	RegDat = STTS751_REG_TEMP_H;
//	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_SUBADD, &RegDat);// Set start register for data output
//	RegDat = 0x0B;
//	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_SLV1_CONFIG, &RegDat);// Set bytes to read - status between L and H registers so need to read 3
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
	RegDat = 0x4C;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &RegDat);// Master config - enable pullups & accel trigger drdy & number external sensors
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);

}

void pawprint_WriteSD( FIL *SDFile , char *outBUFFER, int *bufferLength){

	int failcount = 0;
	unsigned int byteCount = 0;
	int blockSize = 5120;
	int readChunk = ((*bufferLength / blockSize) * blockSize);

	f_open(SDFile, "Out.csv", FA_OPEN_APPEND | FA_WRITE); // Open file for appending

	for (int i = 0; i <= (readChunk/blockSize); i++){
		f_write(SDFile, &outBUFFER[i*blockSize], blockSize, &byteCount); // Write largest 512 multiple sector
		if (byteCount != blockSize){

			failcount++;

		}


			f_sync(SDFile);

	}

	f_close(SDFile);

	*bufferLength -= readChunk; // Get remainder length and set buffer length to accommodate it

	memmove(outBUFFER, &outBUFFER[readChunk], *bufferLength);  // Move remaining data to index 0

}

// Custom FIFO read function with single additional buffer - Trying to address missing data issue
// Will attempt to align on single line per time stamp
// SD write file incorporated to ensure  buffer is unloaded ASAP and does not overrun due to FIFO depth
// Perhaps add line to get maximum line size and check that if added to buffer would not overflow

void pawprint_readFIFO (I2C_HandleTypeDef *i2cHandle, char *buffer, int *writeIndex, int *maxoutLength, int *tag_counter, sensor_out *FIFOout) {

	// Things needed to import with function
	// Char Buffer of size 40KB
	// Buffer write index
	// Previous max line size
	// Time at start of timer in UNIX to microsecond - reset by writing AAh to TIMESTAMP2 register when grabbing RTC time to sync
	// BDR value
	// Previous Timestamp
	// Previous Tag count

	HAL_StatusTypeDef statusOut = 0x00; // Success/Fail for each read/write
	uint8_t FIFOstatus[2];
	uint16_t FIFOdepth = 0;
	int outLength = 0;

	// Set Sensor BDR - Move to external input
	float bdr_xl = 104.0f;
	float bdr_gyr = 104.0f;
	float bdr_max = fmaxf(bdr_xl, bdr_gyr);

	// Period length converted to LSB - 1lsb = 25us
	float batch_period = (1000000*(1/bdr_max))/25;

	// Read timestamp_endcount bit to check for timestamp overrun
		// Enable a bit that forces a check for timestamp < old timestamp? Or other overrun check?

	// Read watermark flag and if/else
	LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_STATUS1, FIFOstatus,2);

	if( ((FIFOstatus[1] >> 7) & 0x01) || ((FIFOstatus[1] >> 6) & 0x01) || ((FIFOstatus[1] >> 5) & 0x01) ){

		// if full that flips binary bit indicating data should be cleaned until next timestamp
		//if ((FIFOstatus[1] >> 6) & 0x01) {
		//	overrunStatus = 1;
		//}

		// get fifo depth
		FIFOdepth = (((uint16_t)FIFOstatus[1] & 0x03) << 8 ) + (uint16_t)FIFOstatus[0];

		while(FIFOdepth--){

			uint8_t FIFOdat[7] = {0};

			statusOut = LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_DATA_OUT_TAG, &FIFOdat[0], 7);

			// Get tag type & counter
			uint8_t tag = (FIFOdat[0] & 0xF8) >> 0x03;
			uint8_t new_tag_counter = (FIFOdat[0] & 0x06) >> 0x01;

			// Is tag_counter same as previous?
			if ((new_tag_counter != *tag_counter)){

					// send old data to buffer with snprintf
					outLength = snprintf(&buffer[*writeIndex],40000-*writeIndex ,"%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
																	(((float_t)FIFOout->timestamp) * 25.0f / 1000000.0f),
							        								lsm6dso_from_fs2_to_mg(FIFOout->XLdat.x),
							        								lsm6dso_from_fs2_to_mg(FIFOout->XLdat.y),
							        								lsm6dso_from_fs2_to_mg(FIFOout->XLdat.z),

									        						lsm6dso_from_fs125_to_mdps(FIFOout->GYRdat.x),
																	lsm6dso_from_fs125_to_mdps(FIFOout->GYRdat.y),
																	lsm6dso_from_fs125_to_mdps(FIFOout->GYRdat.z),

																	lis2mdl_from_lsb_to_mgauss(FIFOout->MAGdat.x),
																	lis2mdl_from_lsb_to_mgauss(FIFOout->MAGdat.y),
																	lis2mdl_from_lsb_to_mgauss(FIFOout->MAGdat.z));

					if (outLength < 0){
						// Could be the reason for errors in output? Write if error occurs for inspection
						outLength += snprintf(&buffer[*writeIndex],40000-*writeIndex ,"~,~,E,R,R,O,R,!,~,~\n");
					}

					// clear data - Can I assign this in one? As not changing TimeStamp couldnt work it out
					FIFOout->XLdat.x = 0 ;
					FIFOout->XLdat.y = 0 ;
					FIFOout->XLdat.z = 0 ;
					FIFOout->GYRdat.x = 0 ;
					FIFOout->GYRdat.y = 0 ;
					FIFOout->GYRdat.z = 0 ;
					FIFOout->MAGdat.x = 0 ;
					FIFOout->GYRdat.y = 0 ;
					FIFOout->GYRdat.z = 0 ;
					//*FIFOout = (sensor_out) {0};

					// update max line length - for debug and not used?
					if (outLength > *maxoutLength){
						*maxoutLength = outLength;
					}
					// update buffer write index
					*writeIndex += outLength;

				uint8_t diff_tag_counter = 0;

				if(new_tag_counter < *tag_counter)
				{

					diff_tag_counter = new_tag_counter + 4u - *tag_counter;

				}
				else
				{
					diff_tag_counter = new_tag_counter - *tag_counter;
				}

				*tag_counter = new_tag_counter;

				// Update Timestamp based on counter change - works reliably only where <4 batch periods have passed
				//uint32_t old_timestamp = FIFOout.timestamp;
				FIFOout->timestamp += (batch_period * diff_tag_counter);

				// Need to account for timestamp overrun here as well in future - for now ignored as limit is 1.2 days
				// Potentially: if Timestamp > (Maxtime * (timer_reset_count+1)) then timer_reset_count++
				// Timestamp reset bit exists but do not see reason to use?

			}

			// switch to ID sensor from tag
			switch (tag)
			{
				case 0x01: // Gyroscope
					FIFOout->GYRdat.x = (FIFOdat[2] << 8) | FIFOdat[1] ;
					FIFOout->GYRdat.y = (FIFOdat[4] << 8) | FIFOdat[3];
					FIFOout->GYRdat.z = (FIFOdat[6] << 8) | FIFOdat[5];
					break;

				case 0x02: // Accelerometer
					FIFOout->XLdat.x = (FIFOdat[2] << 8) | FIFOdat[1];
					FIFOout->XLdat.y = (FIFOdat[4] << 8) | FIFOdat[3];
					FIFOout->XLdat.z = (FIFOdat[6] << 8) | FIFOdat[5];
					break;

				case 0x04: // Timestamp
					FIFOout->timestamp = (FIFOdat[4] << 24) | (FIFOdat[3] << 16) | (FIFOdat[2] << 8) | FIFOdat[1];
					break;

				case 0x0E: // Magnetometer
					FIFOout->MAGdat.x = (FIFOdat[2] << 8) | FIFOdat[1];
					FIFOout->MAGdat.y = (FIFOdat[4] << 8) | FIFOdat[3];
					FIFOout->MAGdat.z = (FIFOdat[6] << 8) | FIFOdat[5];
					break;

			}
			// count each loop before data reset and include in struct for debugging - can then check if greater than number of sensors

		}

	}

}


float_t lsm6dso_from_fs2_to_mg(int16_t lsb)
{
  return ((float_t)lsb) * 0.061f;
}

float_t lsm6dso_from_fs125_to_mdps(int16_t lsb)
{
  return ((float_t)lsb) * 4.375f;
}

float_t lis2mdl_from_lsb_to_mgauss(int16_t lsb)
{
  return ((float_t)lsb * 1.5f);
}

float_t lsm6dso_from_lsb_to_celsius(int16_t lsb)
{
  return (((float_t)lsb / 256.0f) + 25.0f);
}

