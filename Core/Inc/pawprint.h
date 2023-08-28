/*
 * pawprint.h
 *
 *  Created on: Aug 27, 2023
 *      Author: J-OSu
 */

#ifndef INC_PAWPRINT_H_
#define INC_PAWPRINT_H_

#include "math.h"
#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"
#include "st_fifo.h"


/* FIFO Tag Enum */
typedef enum
	{
	  LSM6DSO_GYRO_NC_TAG    = 1,
	  LSM6DSO_XL_NC_TAG,
	  LSM6DSO_TEMPERATURE_TAG,
	  LSM6DSO_TIMESTAMP_TAG,
	  LSM6DSO_CFG_CHANGE_TAG,
	  LSM6DSO_XL_NC_T_2_TAG,
	  LSM6DSO_XL_NC_T_1_TAG,
	  LSM6DSO_XL_2XC_TAG,
	  LSM6DSO_XL_3XC_TAG,
	  LSM6DSO_GYRO_NC_T_2_TAG,
	  LSM6DSO_GYRO_NC_T_1_TAG,
	  LSM6DSO_GYRO_2XC_TAG,
	  LSM6DSO_GYRO_3XC_TAG,
	  LSM6DSO_SENSORHUB_SLAVE0_TAG,
	  LSM6DSO_SENSORHUB_SLAVE1_TAG,
	  LSM6DSO_SENSORHUB_SLAVE2_TAG,
	  LSM6DSO_SENSORHUB_SLAVE3_TAG,
	  LSM6DSO_STEP_COUNTER_TAG,
	  LSM6DSO_SENSORHUB_NACK_TAG  = 0x19,
	}lsm6dso_tags_t;

void pawprint_init( I2C_HandleTypeDef *i2cHandle );

void pawprint_readFIFO( I2C_HandleTypeDef *i2cHandle );

float_t lsm6dso_from_fs2_to_mg(int16_t lsb);

#endif /* INC_PAWPRINT_H_ */
