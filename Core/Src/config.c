/*
 * config.c
 *
 *  Created on: Sep 19, 2020
 *      Author: Kair
 */
#include "config.h"
#include "stm32f4xx.h"

config_t config_read(void){
	// config is located at the begining of the 5th sector
	return *(config_t*)((volatile void*)0x08020000);
}

void config_save(config_t* cfg){
	// first erase sector where config is located
	static uint32_t l_error;
	static FLASH_EraseInitTypeDef EraseInitStruct = {
			.Sector = FLASH_SECTOR_5,
			.NbSectors = 1,
			.VoltageRange = FLASH_VOLTAGE_RANGE_3,
			.TypeErase = FLASH_TYPEERASE_SECTORS
	};
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&EraseInitStruct, &l_error);
	HAL_FLASH_Lock();

	uint32_t l_address = 0x08020000;
	uint32_t l_end = l_address + sizeof(config_t);
	void *ptr = (uint32_t*)cfg;

	HAL_FLASH_Unlock();
	while(l_address < l_end){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,l_address,*(uint32_t*)ptr) == HAL_OK){
			ptr+=sizeof(uint32_t);
			l_address+=sizeof(uint32_t);
		}
	}
	HAL_FLASH_Lock();
}
