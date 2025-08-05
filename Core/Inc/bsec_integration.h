/**
 * Copyright (C) Bosch Sensortec GmbH. All Rights Reserved. Confidential.
 *
 * Disclaimer
 *
 * Common:
 * Bosch Sensortec products are developed for the consumer goods industry. They may only be used
 * within the parameters of the respective valid product data sheet. Bosch Sensortec products are
 * provided with the express understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive systems or any system or device
 * that may lead to bodily harm or property damage if the system or device malfunctions. In addition,
 * Bosch Sensortec products are not fit for use in products which interact with motor vehicle systems.
 * The resale and/or use of products are at the purchaser's own risk and his own responsibility. The
 * examination of fitness for the intended use is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for
 * incidental, or consequential damages, arising from any product use not covered by the parameters of
 * the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch
 * Sensortec for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products, particularly with regard to
 * product safety and inform Bosch Sensortec without delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid
 * technical specifications of the product series. They are therefore not intended or fit for resale to third
 * parties or for use in end products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series. Bosch Sensortec
 * assumes no liability for the use of engineering samples. By accepting the engineering samples, the
 * Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering
 * samples.
 *
 * Special:
 * This software module (hereinafter called "Software") and any information on application-sheets
 * (hereinafter called "Information") is provided free of charge for the sole purpose to support your
 * application work. The Software and Information is subject to the following terms and conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch Sensortec products by
 * personnel who have special experience and training. Do not use this Software if you do not have the
 * proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or implied warranties,
 * including without limitation, the implied warranties of merchantability and fitness for a particular
 * purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for the functional impairment
 * of this Software in terms of fitness, performance and safety. Bosch Sensortec and their
 * representatives and agents shall not be liable for any direct or indirect damages or injury, except as
 * otherwise stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no
 * responsibility for the consequences of use of such Information nor for any infringement of patents or
 * other rights of third parties which may result from its use. No license is granted by implication or
 * otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are
 * subject to change without notice.
 *
 * It is not allowed to deliver the source code of the Software to any third party without permission of
 * Bosch Sensortec.
 *
 */

/*!
 * @file bsec_integration.h
 *
 * @brief Contains BSEC integration API
 */

#ifndef __BSEC_INTEGRATION_H__
#define __BSEC_INTEGRATION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "bme69x.h"
#include "bsec_interface.h"
#include "bsec_datatypes.h"

#define NUM_OF_SENS    	                UINT8_C(8)
#define BSEC_INSTANCE_SIZE              UINT16_C(3272)
#define BSEC_CHECK_INPUT(x, shift)		(x & (1 << (shift-1)))
#define BSEC_TOTAL_HEAT_DUR             UINT16_C(140)

/* Note : 
          For the classification output from BSEC algorithm set OUTPUT_MODE to CLASSIFICATION (default).
          For the regression output from BSEC algorithm set OUTPUT_MODE to REGRESSION.
          For the LP, ULP, CONT output from BSEC algorithm set OUTPUT_MODE to IAQ
*/
#define CLASSIFICATION	1
#define REGRESSION		2
#define IAQ				3
#define OUTPUT_MODE		IAQ

/* Note : 
        Set the appropriate "SAMPLE_RATE" based on configured "OUTPUT_MODE".
		    * If the "OUTPUT_MODE"  value is "CLASSIFICATION" (or) "REGRESSION", the "SAMPLE_RATE" assigned is "BSEC_SAMPLE_RATE_SCAN" (default).
			* If the "OUTPUT_MODE" value is "IAQ", the "SAMPLE_RATE" assigned is "BSEC_SAMPLE_RATE_ULP" (default).
			* For the "OUTPUT_MODE" as "IAQ" the other supported "SAMPLE_RATE" is "BSEC_SAMPLE_RATE_CONT" (or) "BSEC_SAMPLE_RATE_LP".
*/
#if (OUTPUT_MODE == CLASSIFICATION || OUTPUT_MODE == REGRESSION)
#define SAMPLE_RATE		    BSEC_SAMPLE_RATE_SCAN
#define NUM_USED_OUTPUTS    UINT8_C(9)
#elif (OUTPUT_MODE == IAQ)
#define SAMPLE_RATE		    BSEC_SAMPLE_RATE_ULP
#define NUM_USED_OUTPUTS    UINT8_C(14)
#endif

/*
 *	The default offset provided has been determined by testing the sensor in LP and ULP mode on application board 3.0
 *	Please update the offset value after testing this on your product 
 */
#define TEMP_OFFSET_LP		(0.1495f)
#define TEMP_OFFSET_ULP		(0.466f)

#define TVOC_EQUIVALENT_ENABLE    UINT8_C(3)
#define TVOC_EQUIVALENT_DISABLE   UINT8_C(0)

/* Structure to store the BSEC output values */
typedef struct{
    int64_t timestamp;
	float gas_estimate_1;
	float gas_estimate_2;
	float gas_estimate_3;
	float gas_estimate_4;
	float raw_pressure;
	float raw_temp;
	float raw_humidity;
	float raw_gas;
	uint8_t raw_gas_index;
	uint8_t gas_accuracy_1;
	uint8_t gas_accuracy_2;
	uint8_t gas_accuracy_3;
	uint8_t gas_accuracy_4;
	float iaq;
	uint8_t iaq_accuracy;
	float compensated_temperature;
	float compensated_humidity;
	float static_iaq;
	float stabStatus;
	float runInStatus;
	float co2_equivalent;
	float breath_voc_equivalent;
	float gas_percentage;
	float tvoc_equivalent;
	uint8_t tvoc_equivalent_accuracy;
}outputs_t;

/* Structure with the return value from bsec_iot_init() */
typedef struct{
	/*! Result of API execution status */
	int8_t bme69x_status;
	/*! Result of BSEC library */
	bsec_library_return_t bsec_status;
}return_values_init;

/* function pointer to BME communication interface configuration */
typedef void (*bme69x_interface_fct)(struct bme69x_dev *bme, uint8_t intf, uint8_t sen_no);

/* function pointer to the function processing obtained BSEC outputs */
typedef void (*output_ready_fct)(char *output);

/* function pointer to the function saving BSEC state to NVM */
typedef void (*state_save_fct)(const uint8_t *state_buffer, uint32_t length);

/* function pointer to the function loading a previous BSEC state from NVM */
typedef uint32_t (*state_load_fct)(uint8_t *state_buffer, uint32_t n_buffer);

/* function pointer to the function loading the BSEC configuration string from NVM */
typedef uint32_t (*config_load_fct)(uint8_t *state_buffer, uint32_t n_buffer);

/* function pointer to the system specific timestamp derivation function */
typedef uint32_t (*get_timestamp_ms_fct)();

/* function pointer to the system specific timestamp derivation function */
typedef void (*tvoc_equivalent_calibration_fct)();


/*!
 * @brief       Initialize the bme69x sensor and the BSEC library
 *
 * @param[in]   sample_rate    		bsec subscription sample rate
 * @param[in]   bme69x_intf_init    pointer to the bme sensor communication initialization function
 * @param[in]   state_load          pointer to the system-specific state save function
 * @param[in]   config_load         pointer to the config buffer read function
 *
 * @return      return_values_init	struct with the result of the API and the BSEC library
 */ 
return_values_init bsec_iot_init(float sample_rate, bme69x_interface_fct bme69x_intf_init, state_load_fct state_load, config_load_fct config_load);

/*!
 * @brief       Runs the main (endless) loop that queries sensor settings, applies them, and processes the measured data
 * @param[in]   state_save    				pointer to the system-specific state save derivation function
 * @param[in]   get_timestamp_ms    		pointer to the system-specific timestamp derivation function
 * @param[in]   output_ready    			pointer to the system-specific output ready derivation function
 * @param[in]   tvoc_equivalent_calibration	pointer to the system-specific tvoc equivalent derivation function
 */ 
void bsec_iot_loop(state_save_fct state_save, get_timestamp_ms_fct get_timestamp_ms, output_ready_fct output_ready,
					tvoc_equivalent_calibration_fct tvoc_equivalent_calibration);

/**
 * @brief Function to assign the memory block to the bsec instance
 *
 * @param[in] mem_block     reference to the memory block
 * @param[in] sens_no       sensor no
 */
void allocate_memory(uint8_t *mem_block, uint8_t sens_no);

/**
 * @brief Function to enable or disable the baseline tracker in the BSEC
 *
 * @param[in] data     Baselline tracker flag enable or disable value
 *                     TRUE  -> TVOC equivalent baseline adaption ON
 *                     FALSE -> TVOC equivalent baseline adaption OFF
 */
void set_tvoc_equivalent_baseline(bool data);

/**
 * @brief Function to get the sample rate
 *
 * @return     Return the sample rate value
 */
float get_sample_rate();

#ifdef __cplusplus
}
#endif

#endif /* __BSEC_INTEGRATION_H__ */