/*
 * lsm6dso.h
 *
 *  Created on: Aug 26, 2023
 *      Author: J-OSu
 */

#ifndef INC_LSM6DSO_H_
#define INC_LSM6DSO_H_

#include <stdint.h>
#include "stm32l4xx_hal.h" /*Required for I2C */

/** IMU Addresses **/

/* I2C Address - 8 bit format
 * PawPrint LSM6DSO address pin tied high therefore 0xD7 read - 0xD6 write
 * Alternative low address commented for convenience
 */

#define LSM6DSO_I2C_ADDR						 0xD7
// #define LSM6DSO_I2C_ADDR						 0xD5

/* Device ID - Who am I */
#define LSM6DSO_ID								 0x6C


/** Registers - pg 44 **/
#define LSM6DSO_REG_FUNC_CFG_ACCESS				 0x01U
#define LSM6DSO_REG_PIN_CTRL					 0x02U
#define LSM6DSO_REG_FIFO_CTRL1					 0x07U
#define LSM6DSO_REG_FIFO_CTRL2					 0x08U
#define LSM6DSO_REG_FIFO_CTRL3					 0x09U
#define LSM6DSO_REG_FIFO_CTRL4                   0x0AU
#define LSM6DSO_REG_COUNTER_BDR_REG1             0x0BU
#define LSM6DSO_REG_COUNTER_BDR_REG2             0x0CU
#define LSM6DSO_REG_INT1_CTRL                    0x0DU
#define LSM6DSO_REG_INT2_CTRL                    0x0EU
#define LSM6DSO_REG_WHO_AM_I                     0x0FU
#define LSM6DSO_REG_CTRL1_XL                     0x10U
#define LSM6DSO_REG_CTRL2_G                      0x11U
#define LSM6DSO_REG_CTRL3_C                      0x12U
#define LSM6DSO_REG_CTRL4_C                      0x13U
#define LSM6DSO_REG_CTRL5_C                      0x14U
#define LSM6DSO_REG_CTRL6_C                      0x15U
#define LSM6DSO_REG_CTRL7_G                      0x16U
#define LSM6DSO_REG_CTRL8_XL                     0x17U
#define LSM6DSO_REG_CTRL9_XL                     0x18U
#define LSM6DSO_REG_CTRL10_C                     0x19U
#define LSM6DSO_REG_ALL_INT_SRC                  0x1AU
#define LSM6DSO_REG_WAKE_UP_SRC                  0x1BU
#define LSM6DSO_REG_TAP_SRC                      0x1CU
#define LSM6DSO_REG_D6D_SRC                      0x1DU
#define LSM6DSO_REG_STATUS_REG                   0x1EU
#define LSM6DSO_REG_STATUS_SPIAUX                0x1EU
#define LSM6DSO_REG_OUT_TEMP_L                   0x20U
#define LSM6DSO_REG_OUT_TEMP_H                   0x21U
#define LSM6DSO_REG_OUTX_L_G                     0x22U
#define LSM6DSO_REG_OUTX_H_G                     0x23U
#define LSM6DSO_REG_OUTY_L_G                     0x24U
#define LSM6DSO_REG_OUTY_H_G                     0x25U
#define LSM6DSO_REG_OUTZ_L_G                     0x26U
#define LSM6DSO_REG_OUTZ_H_G                     0x27U
#define LSM6DSO_REG_OUTX_L_A                     0x28U
#define LSM6DSO_REG_OUTX_H_A                     0x29U
#define LSM6DSO_REG_OUTY_L_A                     0x2AU
#define LSM6DSO_REG_OUTY_H_A                     0x2BU
#define LSM6DSO_REG_OUTZ_L_A                     0x2CU
#define LSM6DSO_REG_OUTZ_H_A                     0x2DU
#define LSM6DSO_REG_EMB_FUNC_STATUS_MAINPAGE     0x35U
#define LSM6DSO_REG_FSM_STATUS_A_MAINPAGE        0x36U
#define LSM6DSO_REG_FSM_STATUS_B_MAINPAGE        0x37U
#define LSM6DSO_REG_STATUS_MASTER_MAINPAGE       0x39U
#define LSM6DSO_REG_FIFO_STATUS1                 0x3AU
#define LSM6DSO_REG_FIFO_STATUS2                 0x3BU
#define LSM6DSO_REG_TIMESTAMP0                   0x40U
#define LSM6DSO_REG_TIMESTAMP1                   0x41U
#define LSM6DSO_REG_TIMESTAMP2                   0x42U
#define LSM6DSO_REG_TIMESTAMP3                   0x43U
#define LSM6DSO_REG_TAP_CFG0                     0x56U
#define LSM6DSO_REG_TAP_CFG1                     0x57U
#define LSM6DSO_REG_TAP_CFG2                     0x58U
#define LSM6DSO_REG_TAP_THS_6D                   0x59U
#define LSM6DSO_REG_INT_DUR2                     0x5AU
#define LSM6DSO_REG_WAKE_UP_THS                  0x5BU
#define LSM6DSO_REG_WAKE_UP_DUR                  0x5CU
#define LSM6DSO_REG_FREE_FALL                    0x5DU
#define LSM6DSO_REG_MD1_CFG                      0x5EU
#define LSM6DSO_REG_MD2_CFG                      0x5FU
#define LSM6DSO_REG_I3C_BUS_AVB                  0x62U
#define LSM6DSO_REG_INTERNAL_FREQ_FINE           0x63U
#define LSM6DSO_REG_INT_OIS                      0x6FU
#define LSM6DSO_REG_CTRL1_OIS                    0x70U
#define LSM6DSO_REG_CTRL2_OIS                    0x71U
#define LSM6DSO_REG_CTRL3_OIS                    0x72U
#define LSM6DSO_REG_X_OFS_USR                    0x73U
#define LSM6DSO_REG_Y_OFS_USR                    0x74U
#define LSM6DSO_REG_Z_OFS_USR                    0x75U
#define LSM6DSO_REG_FIFO_DATA_OUT_TAG            0x78U
#define LSM6DSO_REG_FIFO_DATA_OUT_X_L            0x79U
#define LSM6DSO_REG_FIFO_DATA_OUT_X_H            0x7AU
#define LSM6DSO_REG_FIFO_DATA_OUT_Y_L            0x7BU
#define LSM6DSO_REG_FIFO_DATA_OUT_Y_H            0x7CU
#define LSM6DSO_REG_FIFO_DATA_OUT_Z_L            0x7DU
#define LSM6DSO_REG_FIFO_DATA_OUT_Z_H            0x7EU

/* Embedded Function Registers */

#define LSM6DSO_REG_PAGE_SEL                     0x02U
#define LSM6DSO_REG_EMB_FUNC_EN_A                0x04U
#define LSM6DSO_REG_EMB_FUNC_EN_B                0x05U
#define LSM6DSO_REG_PAGE_ADDRESS                 0x08U
#define LSM6DSO_REG_PAGE_VALUE                   0x09U
#define LSM6DSO_REG_EMB_FUNC_INT1                0x0AU
#define LSM6DSO_REG_FSM_INT1_A                   0x0BU
#define LSM6DSO_REG_FSM_INT1_B                   0x0CU
#define LSM6DSO_REG_EMB_FUNC_INT2                0x0EU
#define LSM6DSO_REG_FSM_INT2_A                   0x0FU
#define LSM6DSO_REG_FSM_INT2_B                   0x10U
#define LSM6DSO_REG_EMB_FUNC_STATUS              0x12U
#define LSM6DSO_REG_FSM_STATUS_A                 0x13U
#define LSM6DSO_REG_FSM_STATUS_B                 0x14U
#define LSM6DSO_REG_PAGE_RW                      0x17U
#define LSM6DSO_REG_EMB_FUNC_FIFO_CFG            0x44U
#define LSM6DSO_REG_FSM_ENABLE_A                 0x46U
#define LSM6DSO_REG_FSM_ENABLE_B                 0x47U
#define LSM6DSO_REG_FSM_LONG_COUNTER_L           0x48U
#define LSM6DSO_REG_FSM_LONG_COUNTER_H           0x49U
#define LSM6DSO_REG_FSM_LONG_COUNTER_CLEAR       0x4AU
#define LSM6DSO_REG_FSM_OUTS1                    0x4CU
#define LSM6DSO_REG_FSM_OUTS2                    0x4DU
#define LSM6DSO_REG_FSM_OUTS3                    0x4EU
#define LSM6DSO_REG_FSM_OUTS4                    0x4FU
#define LSM6DSO_REG_FSM_OUTS5                    0x50U
#define LSM6DSO_REG_FSM_OUTS6                    0x51U
#define LSM6DSO_REG_FSM_OUTS7                    0x52U
#define LSM6DSO_REG_FSM_OUTS8                    0x53U
#define LSM6DSO_REG_FSM_OUTS9                    0x54U
#define LSM6DSO_REG_FSM_OUTS10                   0x55U
#define LSM6DSO_REG_FSM_OUTS11                   0x56U
#define LSM6DSO_REG_FSM_OUTS12                   0x57U
#define LSM6DSO_REG_FSM_OUTS13                   0x58U
#define LSM6DSO_REG_FSM_OUTS14                   0x59U
#define LSM6DSO_REG_FSM_OUTS15                   0x5AU
#define LSM6DSO_REG_FSM_OUTS16                   0x5BU
#define LSM6DSO_REG_EMB_FUNC_ODR_CFG_B           0x5FU
#define LSM6DSO_REG_STEP_COUNTER_L               0x62U
#define LSM6DSO_REG_STEP_COUNTER_H               0x63U
#define LSM6DSO_REG_EMB_FUNC_SRC                 0x64U
#define LSM6DSO_REG_EMB_FUNC_INIT_A              0x66U
#define LSM6DSO_REG_EMB_FUNC_INIT_B              0x67U

/*	Embedded Advanced Features Registers */

#define LSM6DSO_REG_MAG_SENSITIVITY_L            0xBAU
#define LSM6DSO_REG_MAG_SENSITIVITY_H            0xBBU
#define LSM6DSO_REG_MAG_OFFX_L                   0xC0U
#define LSM6DSO_REG_MAG_OFFX_H                   0xC1U
#define LSM6DSO_REG_MAG_OFFY_L                   0xC2U
#define LSM6DSO_REG_MAG_OFFY_H                   0xC3U
#define LSM6DSO_REG_MAG_OFFZ_L                   0xC4U
#define LSM6DSO_REG_MAG_OFFZ_H                   0xC5U
#define LSM6DSO_REG_MAG_SI_XX_L                  0xC6U
#define LSM6DSO_REG_MAG_SI_XX_H                  0xC7U
#define LSM6DSO_REG_MAG_SI_XY_L                  0xC8U
#define LSM6DSO_REG_MAG_SI_XY_H                  0xC9U
#define LSM6DSO_REG_MAG_SI_XZ_L                  0xCAU
#define LSM6DSO_REG_MAG_SI_XZ_H                  0xCBU
#define LSM6DSO_REG_MAG_SI_YY_L                  0xCCU
#define LSM6DSO_REG_MAG_SI_YY_H                  0xCDU
#define LSM6DSO_REG_MAG_SI_YZ_L                  0xCEU
#define LSM6DSO_REG_MAG_SI_YZ_H                  0xCFU
#define LSM6DSO_REG_MAG_SI_ZZ_L                  0xD0U
#define LSM6DSO_REG_MAG_SI_ZZ_H                  0xD1U
#define LSM6DSO_REG_MAG_CFG_A                    0xD4U
#define LSM6DSO_REG_MAG_CFG_B                    0xD5U
#define LSM6DSO_REG_FSM_LC_TIMEOUT_L             0x17AU
#define LSM6DSO_REG_FSM_LC_TIMEOUT_H             0x17BU
#define LSM6DSO_REG_FSM_PROGRAMS                 0x17CU
#define LSM6DSO_REG_FSM_START_ADD_L              0x17EU
#define LSM6DSO_REG_FSM_START_ADD_H              0x17FU
#define LSM6DSO_REG_PEDO_CMD_REG                 0x183U
#define LSM6DSO_REG_PEDO_DEB_STEPS_CONF          0x184U
#define LSM6DSO_REG_PEDO_SC_DELTAT_L             0x1D0U
#define LSM6DSO_REG_PEDO_SC_DELTAT_H             0x1D1U

/* Sensor Hub Storage Registers */

#define LSM6DSO_REG_SENSOR_HUB_1                 0x02U
#define LSM6DSO_REG_SENSOR_HUB_2                 0x03U
#define LSM6DSO_REG_SENSOR_HUB_3                 0x04U
#define LSM6DSO_REG_SENSOR_HUB_4                 0x05U
#define LSM6DSO_REG_SENSOR_HUB_5                 0x06U
#define LSM6DSO_REG_SENSOR_HUB_6                 0x07U
#define LSM6DSO_REG_SENSOR_HUB_7                 0x08U
#define LSM6DSO_REG_SENSOR_HUB_8                 0x09U
#define LSM6DSO_REG_SENSOR_HUB_9                 0x0AU
#define LSM6DSO_REG_SENSOR_HUB_10                0x0BU
#define LSM6DSO_REG_SENSOR_HUB_11                0x0CU
#define LSM6DSO_REG_SENSOR_HUB_12                0x0DU
#define LSM6DSO_REG_SENSOR_HUB_13                0x0EU
#define LSM6DSO_REG_SENSOR_HUB_14                0x0FU
#define LSM6DSO_REG_SENSOR_HUB_15                0x10U
#define LSM6DSO_REG_SENSOR_HUB_16                0x11U
#define LSM6DSO_REG_SENSOR_HUB_17                0x12U
#define LSM6DSO_REG_SENSOR_HUB_18                0x13U

/* Sensor Hub Peripheral Addresses, Register Addresses and config */

#define LSM6DSO_REG_MASTER_CONFIG                0x14U
#define LSM6DSO_REG_SLV0_ADD                     0x15U
#define LSM6DSO_REG_SLV0_SUBADD                  0x16U
#define LSM6DSO_REG_SLV0_CONFIG                  0x17U
#define LSM6DSO_REG_SLV1_ADD                     0x18U
#define LSM6DSO_REG_SLV1_SUBADD                  0x19U
#define LSM6DSO_REG_SLV1_CONFIG                  0x1AU
#define LSM6DSO_REG_SLV2_ADD                     0x1BU
#define LSM6DSO_REG_SLV2_SUBADD                  0x1CU
#define LSM6DSO_REG_SLV2_CONFIG                  0x1DU
#define LSM6DSO_REG_SLV3_ADD                     0x1EU
#define LSM6DSO_REG_SLV3_SUBADD                  0x1FU
#define LSM6DSO_REG_SLV3_CONFIG                  0x20U
#define LSM6DSO_REG_DATAWRITE_SLV0               0x21U
#define LSM6DSO_REG_STATUS_MASTER                0x22U

/* Low level functions */
HAL_StatusTypeDef	LSM6DSO_ReadReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef	LSM6DSO_ReadRegs( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data, uint8_t len );

HAL_StatusTypeDef	LSM6DSO_WriteReg( I2C_HandleTypeDef	*i2cHandle, uint8_t reg, uint8_t *data );

#endif /* INC_LSM6DSO_H_ */
