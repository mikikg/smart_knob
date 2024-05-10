/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "wwdg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25qxx.h"
#include <stdio.h>

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  MX_TIM11_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_WWDG_Init();
  /* USER CODE BEGIN 2 */

  char buffer[20]; // Rezerviši dovoljno mesta za string koji ćeš generisati

  const char myInfo[] =
        "\n"
        "/--------------------------------------\\\n"
        "| Smart Button Controller V0.1 2024    |\n"
        "| https://forum.yu3ma.net              |\n"
        "| Type 'h' for help                    |\n"
        "\\--------------------------------------/\n";
  const char myHelp[] =
        "\n"
        "/--------------------------------------\\\n"
        "| Smart Button Controller V0.1         |\n"
        "\\--------------------------------------/\n"
        "Command overview \n"
        "h H        - Help\n"
        "D          - Dump vars\n"
        "d          - Dump vars and names\n"
        "i I        - Machine Info\n"
        "c          - Main loop counts per 100ms\n"
        "x          - Dump NVRAM in RAW mode\n"
        "X          - Dump NVRAM in HEX mode\n"
        "r          - Read USART3 diag data\n"
        "R          - Clear USART3 diag data\n"
        "q          - Set QUICK BOOT flag\n"
        "+          - Increment menu/value\n"
        "-          - Decrement menu/value\n"
        "e          - Toggle view/edit mode\n"
        "s          - Toggle start/stop mode\n"
        "b          - Turn off boot info\n"
        "B          - Turn on boot info\n"
        "~          - Reset controller\n"
        "r[xxxx]    - Read MCU register at ADR\n"
        "\n";
int adc_value;
HAL_ADC_Start(&hadc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_UART_Transmit_DMA(&huart1, myInfo, sizeof myInfo-1);
	while (1){
		//for (int x = 0; x < 1000000; x++) {GPIOC->BSRR=GPIO_BSRR_BS13;}
		//for (int x = 0; x < 1000000; x++) {GPIOC->BSRR=GPIO_BSRR_BR13;}
		//CDC_Transmit_FS (buff, sizeof buff);
		//HAL_UART_Transmit(&huart1, buff, sizeof buff, 10);

		if (UART_KBD_rx_ready==1) {
			UART_KBD_rx_ready=0;
			//HAL_UART_Transmit_DMA(&huart1, UART_KBD_RX_buff, UART_KBD_RX_packet_len);
			//HAL_UART_Transmit(&huart1, UART_KBD_RX_buff, UART_KBD_RX_packet_len, 10);
			switch (UART_KBD_RX_buff[0]) {
				case 'h':
				case 'H':
					HAL_UART_Transmit_DMA(&huart1, myHelp, sizeof myHelp-1);
				break;
				case '~': //reset controller
                    NVIC_SystemReset();
                break;
                case 'r': //read ADC
                    adc_value = HAL_ADC_GetValue(&hadc1);
                    sprintf(buffer, "Broj=%hu\n", adc_value); // Formatiraj broj i snimi rezultat u bafer
					HAL_UART_Transmit_DMA(&huart1, buffer, strlen(buffer));
                break;
			}

		}
		if (!(GPIOA->IDR & GPIO_IDR_ID0 !=0)) {
			adc_value = HAL_ADC_GetValue(&hadc1);
			sprintf(buffer, "Broj=%hu\n", adc_value); // Formatiraj broj i snimi rezultat u bafer
			HAL_UART_Transmit_DMA(&huart1, buffer, strlen(buffer));
		}

		//HAL_Delay(10);
	    HAL_WWDG_Refresh(&hwwdg);
		//GPIOC->ODR ^= (1 << 13);
	}
  }
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
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
