/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "config.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t reset = 0;

config_t config_table[] = {
		{
				.t1 = 100,
				.t2 = 1000,
				.t3 = 100,
				.A = 100
		},
		{
				.t1 = 100,
				.t2 = 1000,
				.t3 = 100,
				.A = 20
		},
		{
				.t1 = 1200,
				.t2 = 100,
				.t3 = 5000,
				.A = 100
		}
};

config_t cfg = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/**
 * @brief rises led to maximum brightness
 * @param cfg - config structure that contains certain data
 */
void rise(config_t cfg);
/**
 * @brief resets to low level
 * @param cfg - config structure that contains certain data
 */
void fall(config_t cfg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void tim1_irq(void){
	static int i = 0;
	static int state = 1;
	switch(state){
	case 1:
		if(i == (float)cfg.A/100.0 * 65535.0){
			state = 2;
			HAL_TIM_Base_Stop_IT(&htim1);
			TIM4->CCR1 = i;
		}
		else{
			TIM4->CCR1 = i++;
		}
		break;
	case 2:
		if(i == 0){
			state = 1;
			HAL_TIM_Base_Stop_IT(&htim1);
			TIM4->CCR1 = 0;
			i = 0;
		}
		else{
			TIM4->CCR1 = i--;
		}
		break;
	}
}


void delay_us(uint64_t delay){
	htim1.Instance->CNT = 0; // reset counter
	HAL_TIM_Base_Start(&htim1);
	uint16_t init = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
	while(((uint16_t)__HAL_TIM_GET_COUNTER(&htim1)-init) < delay);
	HAL_TIM_Base_Stop(&htim1);
}

void ext_irq(void){
	reset = 1;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	uint8_t next_cfg = 0;
	cfg = config_read();
	uint8_t error = 1;
	for(int i = 0; i < 3; i++){
		if (memcmp(&cfg, &config_table[i], sizeof(config_t)) == 0) {
			next_cfg = i + 1;
			if (next_cfg >= 3)
				next_cfg = 0;
			error = 0;
		}
	}
	if (error){
		memcpy(&cfg, &config_table[0], sizeof(config_t));
		config_save(&cfg);
	}
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM4_Init();
	MX_TIM1_Init();
	/* USER CODE BEGIN 2 */
	int i = 0;
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	int state = 1;//rising state
	while (!reset)
	{
		switch(state){
		// rising state
		case 1:
			rise(cfg);
			state++;
			break;
			// halt state
		case 2:
			HAL_Delay(cfg.t2);
			state++;
			break;
			// falling state
		case 3:
			fall(cfg);
			state++;
			break;
		default:
			HAL_Delay(1000);
			state = 1;
			break;
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	// save next configuration
	config_save(&config_table[next_cfg]);
	NVIC_SystemReset();
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void rise(config_t cfg){
	uint32_t max = (float)cfg.A/100.0 * 65535.0;
	__HAL_TIM_SET_AUTORELOAD(&htim1, 84000000.0/(float)(max) * (float)cfg.t1/1000.0 - 1);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_Base_Start_IT(&htim1);
	while(TIM4->CCR1 < max);
}


void fall(config_t cfg){
	uint32_t max = (float)cfg.A/100.0 * 65535.0;
	__HAL_TIM_SET_AUTORELOAD(&htim1, 84000000.0/(float)(max) * (float)cfg.t3/1000.0 - 1);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_Base_Start_IT(&htim1);
	while(TIM4->CCR1 > 0);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
