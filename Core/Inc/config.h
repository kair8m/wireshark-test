/*
 * config.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Kair
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_
#include "stdint.h"
/**
 * @brief config structure
 */
#pragma pack(4)
typedef struct sconfig{
	uint32_t t1; /**< interval */
	uint32_t t2; /**< interval */
	uint32_t t3; /**< interval */
	uint32_t A;/**< maximum brightness [%] */
}config_t;
#pragma pack(4)
config_t config_read(void);

void config_save(config_t* cfg);

#endif /* INC_CONFIG_H_ */
