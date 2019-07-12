/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "as7265x.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BANDWIDTH 25
#define lambda_50nm 50
#define GAIN 16
#define SURFACE_DIFFUSER 6.09

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */
as7265x_channels_t calibrated_channels;
as7265x_raw_channels_t raw_channels;
as7265x_named_channels_t named_channels;

uint16_t i2c_status;
uint8_t sort_flag = 0;

struct calibrated_channel
{
    float nm410, nm435, nm460, nm485, nm510, nm535, nm560,
    nm585, nm610, nm645, nm680, nm705, nm730, nm760, nm810, nm860, nm900, nm940, intensity, intensity_per_bw;
};

struct calibrated_channel calibrated_channel;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

void as7265x_init(void)
{
	as7265x_indicator_disable(&hi2c2);
	as7265x_set_gain (&hi2c2, AS7265X_GAIN_16X);
	as7265x_set_integration_time (&hi2c2, 59); //integration_time: 1-255 in 2.8ms units
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM7)
	    {
		as7265x_set_measurement_mode(&hi2c2, AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
//
		HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);

		while (!as7265x_is_data_available(&hi2c2))
		    {

		    }
//
		as7265x_get_all_calibrated_values(&hi2c2, &calibrated_channels);

		calibrated_channel.nm410 = calibrated_channels.channel[12];
		calibrated_channel.nm435 = calibrated_channels.channel[13];
		calibrated_channel.nm460 = calibrated_channels.channel[14];
		calibrated_channel.nm485 = calibrated_channels.channel[15];
		calibrated_channel.nm510 = calibrated_channels.channel[16];
		calibrated_channel.nm535 = calibrated_channels.channel[17];

		calibrated_channel.nm560 = calibrated_channels.channel[6];
		calibrated_channel.nm585 = calibrated_channels.channel[7];
		calibrated_channel.nm610 = calibrated_channels.channel[0];
		calibrated_channel.nm645 = calibrated_channels.channel[8];
		calibrated_channel.nm680 = calibrated_channels.channel[1];
		calibrated_channel.nm705 = calibrated_channels.channel[9];

		calibrated_channel.nm730 = calibrated_channels.channel[2];
		calibrated_channel.nm760 = calibrated_channels.channel[3];
		calibrated_channel.nm810 = calibrated_channels.channel[4];
		calibrated_channel.nm860 = calibrated_channels.channel[5];
		calibrated_channel.nm900 = calibrated_channels.channel[10];
		calibrated_channel.nm940 = calibrated_channels.channel[11];

		calibrated_channel.intensity = ((calibrated_channel.nm410 + calibrated_channel.nm435)*12.5) + ((calibrated_channel.nm435 + calibrated_channel.nm460)*12.5) +
			((calibrated_channel.nm460 + calibrated_channel.nm485)*12.5) + ((calibrated_channel.nm485 + calibrated_channel.nm510)*12.5) +
			((calibrated_channel.nm510 + calibrated_channel.nm535)*12.5) + ((calibrated_channel.nm535 + calibrated_channel.nm560)*12.5) +
			((calibrated_channel.nm560 + calibrated_channel.nm585)*12.5) + ((calibrated_channel.nm585 + calibrated_channel.nm610)*12.5) +
			((calibrated_channel.nm610 + calibrated_channel.nm645)*12.5) + ((calibrated_channel.nm645 + calibrated_channel.nm680)*12.5) +
			((calibrated_channel.nm680 + calibrated_channel.nm705)*12.5) + ((calibrated_channel.nm705 + calibrated_channel.nm730)*12.5) +
			((calibrated_channel.nm730 + calibrated_channel.nm760)*12.5) + ((calibrated_channel.nm760 + calibrated_channel.nm810)*12.5) +
			((calibrated_channel.nm810 + calibrated_channel.nm860)*12.5) + ((calibrated_channel.nm860 + calibrated_channel.nm900)*12.5) +
			((calibrated_channel.nm900 + calibrated_channel.nm940)*12.5);

		calibrated_channel.intensity /= GAIN;

		calibrated_channel.intensity /=SURFACE_DIFFUSER;

		calibrated_channel.intensity_per_bw = calibrated_channel.intensity / lambda_50nm;


//		as7265x_order_calibrated_channels(&hi2c2, &calibrated_channels);
//
//		as7265x_get_all_raw_values(&hi2c2, &raw_channels);
//
		HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
	    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_I2C2_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  as7265x_init();

  HAL_TIM_Base_Start_IT(&htim7);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		// trigger one-shot conversion
//		as7265x_set_measurement_mode(&hi2c2, AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
//
//		HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);
//
////		if(HAL_I2C_IsDeviceReady(&hi2c2,(uint16_t) 0x92,2,100) == HAL_OK)
////		    {
////			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);
////			HAL_Delay(300);
////		    }
////			else if(HAL_I2C_IsDeviceReady(&hi2c2,(uint16_t) 0x93,2,100) == HAL_OK)
////			    {
////				HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
////				HAL_Delay(300);
////			    }
////
//		// wait for data available : TODO: should delay in loop to reduce I2C bus traffic
//		while (!as7265x_is_data_available(&hi2c2))  HAL_Delay(5);
//
//
//		as7265x_get_all_calibrated_values(&hi2c2, &calibrated_channels);
//
////		as7265x_get_ordered_channel_wavelengths();
//		as7265x_order_calibrated_channels(&hi2c2, &calibrated_channels);
//
//		HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);

    }

//		HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  as7265x_soft_reset(&hi2c2);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 299;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 60000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PG13 PG14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
