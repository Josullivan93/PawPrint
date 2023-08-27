/*
 * pawprint.h
 *
 *  Created on: Aug 27, 2023
 *      Author: J-OSu
 */

#ifndef INC_PAWPRINT_H_
#define INC_PAWPRINT_H_

#include "lsm6dso.h"
#include "lis2mdl.h"
#include "stts751.h"

void pawprint_init( I2C_HandleTypeDef *i2cHandle );

#endif /* INC_PAWPRINT_H_ */
