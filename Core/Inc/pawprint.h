/*
 * pawprint.h
 *
 *  Created on: Aug 27, 2023
 *      Author: J-OSu
 */

#ifndef INC_PAWPRINT_H_
#define INC_PAWPRINT_H_

#include "math.h"
#include "fatfs.h"
#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"
#include "st_fifo.h"


/* Simple output struct*/
typedef struct
{
	float_t timestamp;
	struct{
		float_t x;
		float_t y;
		float_t z;
	} XLdat;
	struct{
		float_t x;
		float_t y;
		float_t z;
	} GYRdat;
	struct{
		float_t x;
		float_t y;
		float_t z;
	} MAGdat;
} sensor_out;


void pawprint_init( I2C_HandleTypeDef *i2cHandle );

void pawprint_WriteSD( FIL *SDFile , char *outBUFFER, int *bufferLength);

void pawprint_readFIFO (I2C_HandleTypeDef *i2cHandle, char *buffer, int *writeIndex, int *maxoutLength, int *tag_counter, sensor_out *FIFOout);

float_t lsm6dso_from_fs2_to_mg(int16_t lsb);

float_t lsm6dso_from_fs125_to_mdps(int16_t lsb);

float_t lis2mdl_from_lsb_to_mgauss(int16_t lsb);

float_t lsm6dso_from_lsb_to_celsius(int16_t lsb);

#endif /* INC_PAWPRINT_H_ */
