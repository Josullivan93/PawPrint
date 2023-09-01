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

#define SLOT_NUMBER 512

//static FIFO_out_file FIFO_out[512];
static st_fifo_raw_slot raw_slot[SLOT_NUMBER];
static st_fifo_out_slot out_slot[SLOT_NUMBER];
static st_fifo_out_slot acc_slot[SLOT_NUMBER];
static st_fifo_out_slot gyr_slot[SLOT_NUMBER];
static st_fifo_out_slot mag_slot[SLOT_NUMBER];

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
	RegDat = 0x11;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL3 , &RegDat); // Set BDR for XL and Gyro (12.5Hz both)
	RegDat = 0x46;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FIFO_CTRL4 , &RegDat);//41: FIFO mode, bqtch Timestamp, no temp   56: Set to continuous mode & batch Temp and Timestamp
	RegDat = 0x00; //RegDat = 0x41;
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
//	RegDat = 0x00;
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
	RegDat = 0x10;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL1_XL, &RegDat);//Set acc scale and sample rate  - ctrl 1 xl
	RegDat = 0x10;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_CTRL2_G, &RegDat);// Gyro scale and sample rate - ctrl 2 g

	/* Enable I2C Master to begin Data collection */
	RegDat = 0x40;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat); // Enable SHUB Reg Access
	RegDat = 0x4C;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_MASTER_CONFIG , &RegDat);// Master config - enable pullups & accel trigger drdy & number external sensors
	RegDat = 0x00;
	LSM6DSO_WriteReg(i2cHandle, LSM6DSO_REG_FUNC_CFG_ACCESS, &RegDat);

}

// Custom FIFO fetch function for use with double buffer
void pawprint_readFIFO_DB(I2C_HandleTypeDef *i2cHandle, char *buffer, int *writeIndex) {


	/* Init utility for FIFO decompression */
	st_fifo_conf conf;
	  conf.device = ST_FIFO_LSM6DSO;
	  conf.bdr_xl = 13.0f;
	  conf.bdr_gy = 13.0f;
	  conf.bdr_vsens = 13.0f;

	  st_fifo_init(&conf);

	// Variables
	HAL_StatusTypeDef statusOut = 0x00; // Success/Fail for each read/write
	uint16_t out_slot_size;
	uint16_t slots = 0;
	uint8_t FIFOstatus[2];
	uint16_t FIFOdepth = 0;
    uint16_t acc_samples;
    uint16_t gyr_samples;
    uint16_t mag_samples;
    int outLength = 0;
    int count = 0;

	// Read Watermark - BDU enabled therefore all bits MUST be read in sequence MSB to LSB
	statusOut = LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_STATUS1, &FIFOstatus[0],2);

	if( ((FIFOstatus[1] >> 7) & 0x01) || ((FIFOstatus[1] >> 6) & 0x01) || ((FIFOstatus[1] >> 5) & 0x01) ){

		statusOut = FIFOdepth = (((uint16_t)FIFOstatus[1] & 0x03) << 8 ) + (uint16_t)FIFOstatus[0];

		while( FIFOdepth-- ){

			statusOut = LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_DATA_OUT_TAG, (uint8_t *)&raw_slot[slots].fifo_data_out[0]);

			LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_DATA_OUT_X_L, &raw_slot[slots].fifo_data_out[1],6);

			slots++;

			}

		/* Uncompress FIFO samples and filter based on sensor type */
		      st_fifo_decode(out_slot, raw_slot, &out_slot_size, slots);
		      st_fifo_sort(out_slot, out_slot_size);
		      acc_samples = st_fifo_get_sensor_occurrence(out_slot,
		                                                  out_slot_size,
		                                                  ST_FIFO_ACCELEROMETER);
		      gyr_samples = st_fifo_get_sensor_occurrence(out_slot,
		                                                  out_slot_size,
		                                                  ST_FIFO_GYROSCOPE);
		      mag_samples = st_fifo_get_sensor_occurrence(out_slot,
		      		                                      out_slot_size,
														  ST_FIFO_EXT_SENSOR0);


		      /* Count how many acc and gyro samples */
		      st_fifo_extract_sensor(acc_slot, out_slot, out_slot_size,
		                                   ST_FIFO_ACCELEROMETER);
		      st_fifo_extract_sensor(gyr_slot, out_slot, out_slot_size,
		                                   ST_FIFO_GYROSCOPE);
		      st_fifo_extract_sensor(mag_slot, out_slot, out_slot_size,
		    		  	  	  	  	  	   ST_FIFO_EXT_SENSOR0);


		      for (int i = 0; i < acc_samples; i++) {
		        outLength = snprintf(&buffer[count],sizeof(&buffer)-*writeIndex ,"%lu,%d,%.3f,%.3f,%.3f\r\n",
		        	    		    			acc_slot[i].timestamp,
		        	    						acc_slot[i].sensor_tag,
		        								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.x),
		        								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.y),
		        								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.z));

		        count += outLength;
		      }

		      for (int i = 0; i < gyr_samples; i++) {
		      		        outLength = snprintf(&buffer[count],sizeof(&buffer)-*writeIndex ,"%lu,%d,%.3f,%.3f,%.3f\r\n",
		      		        	    		    			gyr_slot[i].timestamp,
		      		        	    						gyr_slot[i].sensor_tag,
															lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.x),
															lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.y),
															lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.z));

		      		        count += outLength;
		      		      }

		      for (int i = 0; i < mag_samples; i++) {
		      		        outLength = snprintf(&buffer[count],sizeof(&buffer)-*writeIndex ,"%lu,%d,%.3f,%.3f,%.3f\r\n",
		      		        	    		    			mag_slot[i].timestamp,
		      		        	    						mag_slot[i].sensor_tag,
		      		        								lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.x),
		      		        								lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.y),
		      		        								lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.z));

		      		        count += outLength;
		      		      }

		      slots = 0;
		      *writeIndex += count;

		}

}

/* FIFO read and decode - uses st_fifo repo from STMicro git */

void pawprint_readFIFO( I2C_HandleTypeDef *i2cHandle , char *outBUFFER, int *bufferLength, int *writeIndex){

	st_fifo_conf FIFOconf;
	uint8_t FIFOstatus[2];
	uint16_t FIFOdepth = 0;
	HAL_StatusTypeDef statusOUT = 0x00;
//	FIFO_out_file FIFO_out;

	FIFOconf.device = ST_FIFO_LSM6DSO;
	FIFOconf.bdr_xl = 12.5; // 104 - as batching timestamp can set to 0
	FIFOconf.bdr_gy = 12.5;
	FIFOconf.bdr_vsens = 12.5;

	st_fifo_init(&FIFOconf);
	uint16_t out_slot_size = 0;

	/* Confirm watermark has been met LSM6DSO_REG_FIFO_STATUS2 */
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_STATUS1, &FIFOstatus[0]); // Getting strange results reading multiple registers here
	LSM6DSO_ReadReg(i2cHandle, LSM6DSO_REG_FIFO_STATUS2, &FIFOstatus[1]);

	if (((FIFOstatus[1] >> 7)  & 0x01)||((FIFOstatus[1] >> 6)  & 0x01)||((FIFOstatus[1] >> 5)  & 0x01)){ // Check WTM threshold bit, overrun bit and full bit
		/* Get number of samples in FIFO*/
		FIFOdepth = ((((uint16_t)FIFOstatus[1] & 0x03) << 8) + (uint16_t)FIFOstatus[0]);

//		st_fifo_raw_slot *raw_slot;
//		st_fifo_out_slot *out_slot;
//
//		raw_slot = malloc(FIFOdepth * sizeof(st_fifo_raw_slot));
//		out_slot = malloc(FIFOdepth * 3 * sizeof(st_fifo_out_slot));


		st_fifo_raw_slot raw_slot[FIFOdepth];
		st_fifo_out_slot out_slot[FIFOdepth];

		int slots = 0;

		int streamSize = FIFOdepth;

		while(FIFOdepth--) {

			statusOUT = LSM6DSO_ReadRegs(i2cHandle, LSM6DSO_REG_FIFO_DATA_OUT_TAG, &raw_slot[slots].fifo_data_out[0],7 );
			if(statusOUT != 0x00){
				slots--;
			}
			slots++;
		}

		st_fifo_decode(out_slot, raw_slot, &out_slot_size, streamSize);
		st_fifo_sort(out_slot, out_slot_size);

		uint16_t acc_samples = st_fifo_get_sensor_occurrence(out_slot, out_slot_size, ST_FIFO_ACCELEROMETER);
		uint16_t gyr_samples = st_fifo_get_sensor_occurrence(out_slot, out_slot_size, ST_FIFO_GYROSCOPE);
		//uint16_t temp_samples = st_fifo_get_sensor_occurrence(out_slot, out_slot_size, ST_FIFO_TEMPERATURE);
		uint16_t mag_samples = st_fifo_get_sensor_occurrence(out_slot, out_slot_size, ST_FIFO_EXT_SENSOR0);
		//uint16_t ext_temp_samples = st_fifo_get_sensor_occurrence(out_slot, out_slot_size, ST_FIFO_EXT_SENSOR1);


		st_fifo_out_slot acc_slot[acc_samples * sizeof(st_fifo_out_slot)];
		st_fifo_out_slot gyr_slot[gyr_samples * sizeof(st_fifo_out_slot)];
		st_fifo_out_slot mag_slot[mag_samples * sizeof(st_fifo_out_slot)];

//		st_fifo_out_slot *acc_slot = malloc(acc_samples * sizeof(st_fifo_out_slot));
//		st_fifo_out_slot *gyr_slot = malloc(gyr_samples * sizeof(st_fifo_out_slot));
//		//st_fifo_out_slot *temp_slot = malloc(temp_samples * sizeof(st_fifo_out_slot));
//		st_fifo_out_slot *mag_slot = malloc(mag_samples * sizeof(st_fifo_out_slot));
		//st_fifo_out_slot *ext_temp_slot = malloc(ext_temp_samples * sizeof(st_fifo_out_slot));

		st_fifo_extract_sensor(acc_slot, out_slot, out_slot_size, ST_FIFO_ACCELEROMETER);
		st_fifo_extract_sensor(gyr_slot, out_slot, out_slot_size, ST_FIFO_GYROSCOPE);
		//st_fifo_extract_sensor(temp_slot, out_slot, out_slot_size, ST_FIFO_TEMPERATURE);
		st_fifo_extract_sensor(mag_slot, out_slot, out_slot_size, ST_FIFO_EXT_SENSOR0);
		//st_fifo_extract_sensor(ext_temp_slot, out_slot, out_slot_size, ST_FIFO_EXT_SENSOR1);

//
//		for (int i = 0; i < acc_samples; i++){
//			FIFO_out[row_count].timestamp = acc_slot[i].timestamp;
//			FIFO_out[row_count].sensor_tag = acc_slot[i].sensor_tag;
//			FIFO_out[row_count].sensor_data.x = lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.x);
//			FIFO_out[row_count].sensor_data.y = lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.y);
//			FIFO_out[row_count].sensor_data.z = lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.z);
//			FIFO_out[row_count].sensor_data.temp = 0;
//		}
//
//	    for (int i = 0; i < gyr_samples; i++) {
//	    	FIFO_out[row_count].timestamp = gyr_slot[i].timestamp;
//	    	FIFO_out[row_count].sensor_tag = gyr_slot[i].sensor_tag;
//	    	FIFO_out[row_count].sensor_data.x = lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.x);
//	    	FIFO_out[row_count].sensor_data.y = lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.y);
//	    	FIFO_out[row_count].sensor_data.z = lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.z);
//	    	FIFO_out[row_count].sensor_data.temp = 0;
//	    }
//
//	    for (int i = 0; i < mag_samples; i++) {
//	    	FIFO_out[row_count].timestamp = mag_slot[i].timestamp;
//	    	FIFO_out[row_count].sensor_tag = mag_slot[i].sensor_tag;
//	    	FIFO_out[row_count].sensor_data.x = lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.x);
//	    	FIFO_out[row_count].sensor_data.y = lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.y);
//	    	FIFO_out[row_count].sensor_data.z = lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.z);
//	    	FIFO_out[row_count].sensor_data.temp = 0;
//	    }


		if (*bufferLength != 0 ){
			*writeIndex = *bufferLength;
			snprintf(&outBUFFER[*writeIndex],(BUFFER_SIZE-(*writeIndex)) ,"\nB,R,E,A,K\n");
		}

		// Convert to real units & add to formatted strings
	    for (int i = 0; i < acc_samples; i++) {
	    	uint8_t outLength = 0;

	    	outLength = snprintf(&outBUFFER[*writeIndex],(BUFFER_SIZE-(*writeIndex)) ,"\n%lu,%u,%.3f,%.3f,%.3f\n",
	    		    			acc_slot[i].timestamp,
	    						acc_slot[i].sensor_tag,
								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.x),
								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.y),
								lsm6dso_from_fs2_to_mg(acc_slot[i].sensor_data.z));

	    	*bufferLength += outLength;
	    	*writeIndex += outLength;
	    }

	    for (int i = 0; i < gyr_samples; i++) {
	    	uint8_t outLength = 0;

	    	outLength = snprintf(&outBUFFER[*writeIndex],(BUFFER_SIZE-*writeIndex) , "\n%lu,%u,%.3f,%.3f,%.3f\n",
	    		    		    			gyr_slot[i].timestamp,
	    		    						gyr_slot[i].sensor_tag,
											lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.x),
											lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.y),
											lsm6dso_from_fs125_to_mdps(gyr_slot[i].sensor_data.z));
	    	*bufferLength += outLength;
	    	*writeIndex += outLength;
	    }

	    for (int i = 0; i < mag_samples; i++) {
	    	uint8_t outLength = 0;

	    	outLength = snprintf(&outBUFFER[*writeIndex],(BUFFER_SIZE-*writeIndex) , "\n%lu,%u,%.3f,%.3f,%.3f\n",
	    		    		    			mag_slot[i].timestamp,
	    		    						mag_slot[i].sensor_tag,
											lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.x),
											lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.y),
											lis2mdl_from_lsb_to_mgauss(mag_slot[i].sensor_data.z)
											);

	    	*bufferLength += outLength;
	    	*writeIndex += outLength;

	    }
	}

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

//
//	for (int i = 0; i <= *bufferLength/clusterSize; i++) {
//
//		int readIndex_end = *readIndex + (clusterSize-1);
//		*bufferLength -= ((readIndex_end - *readIndex )+ 1);
//		if (readIndex_end > BUFFER_SIZE){
//
//			readIndex_end -= BUFFER_SIZE;
//
//			f_write(SDFile, &outBUFFER[*readIndex], ((BUFFER_SIZE - *readIndex)+1), &byteCount);
//			f_write(SDFile, &outBUFFER, readIndex_end, &byteCount);
//
//		}
//		else {
//			f_write(SDFile, &outBUFFER[*readIndex], clusterSize, &byteCount);
//		}
//		*readIndex = (readIndex_end+1);
//		if (*readIndex > BUFFER_SIZE){
//			*readIndex = 0;
//		}
//	}
//
//	f_close( SDFile );
//
//}

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

