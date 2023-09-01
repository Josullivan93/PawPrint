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

#define BUFFER_SIZE 25600

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
  LSM6DSO_GAME_ROTATION_TAG,
  LSM6DSO_GEOMAG_ROTATION_TAG,
  LSM6DSO_ROTATION_TAG,
  LSM6DSO_SENSORHUB_NACK_TAG  = 0x19,
} lsm6dso_fifo_tag_t;

/* Simple output struct - strips most features of ST_fifo struct that are not used by PawPrint */
typedef struct
{
  uint32_t timestamp;
  st_fifo_sensor_type sensor_tag;
  union
    {
      struct
      {
        int16_t x;
        int16_t y;
        int16_t z;
      }; /* 3-axis mems */
      struct
      {
        int16_t temp;
      }; /* temperature sensor */
     } sensor_data;
} FIFO_out_file;

void pawprint_init( I2C_HandleTypeDef *i2cHandle );

void pawprint_readFIFO( I2C_HandleTypeDef *i2cHandle , char *outBUFFER, int *bufferLength, int *writeIndex);

void pawprint_readFIFO_DB(I2C_HandleTypeDef *i2cHandle, char *buffer, int *writeIndex);

void pawprint_WriteSD( FIL *SDFile , char *outBUFFER, int *bufferLength);

float_t lsm6dso_from_fs2_to_mg(int16_t lsb);

float_t lsm6dso_from_fs125_to_mdps(int16_t lsb);

float_t lis2mdl_from_lsb_to_mgauss(int16_t lsb);

float_t lsm6dso_from_lsb_to_celsius(int16_t lsb);

#endif /* INC_PAWPRINT_H_ */
