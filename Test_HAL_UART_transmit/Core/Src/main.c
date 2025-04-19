/* USER CODE BEGIN Header */

/*
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

//this structure is to handle data points for current micro range.
struct train_model {
	float true_value;
	float model_value;
};


// this structure is to handle data points for current mili range
struct train_model_current_m{
	float true_value_current_m;
	float model_value_current_m;
};

//this structure is for voltage circuit
struct train_model_volt{
	float true_value_volt;
	float model_value_volt;
};

//this flag is to identify what value extracted in the last call
int flag_state;
float flag_state_value;

//this structure is to store resulted function for line of best fit.
struct line_of_best_fit {
	float slope;
	float intercept;
};

//this is unfinished code to handle storing of data.
struct to_store_data {

};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Number of ADC inputs to get to calculate the average.
#define NMSAMPLES 800

//to storing
#define MAX_STORED_READINGS 10
#define FLASH_STORAGE_ADDRESS 0x0800FC00

struct line_of_best_fit result = { 0, 0 };


//this function is to calculate the line of best fit for enterd data
struct line_of_best_fit calculate_line_of_best_fit(struct train_model data[],
		int size) {

	if (size < 2) {
		printf("Not enough data points to calculate the line of best fit.\n");
		return result;
	}

	float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x_squared = 0;

	// Iterate through the data to calculate sums
	for (int i = 0; i < size; i++) {
		float y = data[i].true_value;
		float x = data[i].model_value;

		sum_x += x;
		sum_y += y;
		sum_xy += x * y;
		sum_x_squared += x * x;
	}

	// Calculate slope (m) and intercept (b)
	result.slope = (size * sum_xy - sum_x * sum_y)
			/ (size * sum_x_squared - sum_x * sum_x);
	result.intercept = (sum_y - result.slope * sum_x) / size;

	return result;
}

struct line_of_best_fit calculate_line_of_best_fit_volt(struct train_model_volt data[],
		int size) {

	if (size < 2) {
		printf("Not enough data points to calculate the line of best fit.\n");
		return result;
	}

	float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x_squared = 0;

	// Iterate through the data to calculate sums
	for (int i = 0; i < size; i++) {
		float y = data[i].true_value_volt;
		float x = data[i].model_value_volt;

		sum_x += x;
		sum_y += y;
		sum_xy += x * y;
		sum_x_squared += x * x;
	}

	// Calculate slope (m) and intercept (b)
	result.slope = (size * sum_xy - sum_x * sum_y)
			/ (size * sum_x_squared - sum_x * sum_x);
	result.intercept = (sum_y - result.slope * sum_x) / size;

	return result;
}

struct line_of_best_fit calculate_line_of_best_fit_current_m(struct train_model_current_m data[],
		int size) {

	if (size < 2) {
		printf("Not enough data points to calculate the line of best fit.\n");
		return result;
	}

	float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x_squared = 0;

	// Iterate through the data to calculate sums
	for (int i = 0; i < size; i++) {
		float y = data[i].true_value_current_m;
		float x = data[i].model_value_current_m;

		sum_x += x;
		sum_y += y;
		sum_xy += x * y;
		sum_x_squared += x * x;
	}

	// Calculate slope (m) and intercept (b)
	result.slope = (size * sum_xy - sum_x * sum_y)
			/ (size * sum_x_squared - sum_x * sum_x);
	result.intercept = (sum_y - result.slope * sum_x) / size;

	return result;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PAGEError;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t Tx_data_buffer[27];

uint8_t rx_index;
uint8_t rx_data[1];
uint8_t rx_buffer[5];
uint8_t transfer_cplt;


char buffer[50];
float samples[NMSAMPLES];
float samples_for_ADC_1[NMSAMPLES];
float samples_for_ADC_2[NMSAMPLES];
float average = 0;
float average_ADC_1 = 0;
float average_ADC_2 = 0;
uint8_t i;

uint16_t ADC_1 = 0;
uint16_t ADC_2 = 0;
uint16_t ADC_data_multi[2];
char data_buffer_multi[20];

//To storing
float stored_readings[MAX_STORED_READINGS];
uint8_t num_stored_readings = 0;

//these two are the resistors that i'm consider to get the perfect adc input whithin the rage
int Res_1 = 10;
float Res_2 = 0.1;

int Res_3 = 560000;
int Res_4 = 280000;

uint8_t j;

struct train_model *train_data;
struct train_model_volt *train_data_volt;
struct train_model_current_m *train_data_current_m;


int k = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */

// to storing
static void store_in_flash(uint32_t address, float* data, uint32_t length);
static void read_from_flash(uint32_t address, float* data, uint32_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t convCompleted = 0 ;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  convCompleted = 1;
}


void softwareReset(void)
{
    NVIC_SystemReset(); // Trigger a system reset
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	int initial_capacity = 10;
	int current_capacity = initial_capacity;
	int current_size = 10;


	train_data = (struct train_model*) malloc(current_capacity * sizeof(struct train_model));
	    if (train_data == NULL) {
	        printf("Memory allocation failed for current.\n");
	        return 1;
	    }

	// Initialize the train_data with some initial values
	train_data[0].true_value = 11.37; train_data[0].model_value = 0.023400;
	train_data[1].true_value = 22.34; train_data[1].model_value = 0.024690;
	train_data[2].true_value = 44.29; train_data[2].model_value = 0.026950;
	train_data[3].true_value = 44; train_data[3].model_value = 0.026788;
	train_data[4].true_value = 104.65; train_data[4].model_value = 0.033405;
	train_data[5].true_value = 159.52; train_data[5].model_value = 0.039376;
	train_data[6].true_value = 247.4; train_data[6].model_value = 0.049138;
	train_data[7].true_value = 407; train_data[7].model_value = 0.065841;
	train_data[8].true_value = 768.5; train_data[8].model_value = 0.104812;
	train_data[9].true_value = 1009.5; train_data[9].model_value = 0.130713;



	int initial_capacity_current_m = 7;
	int current_capacity_current_m = initial_capacity_current_m;
	int current_size_current_m = 7;


	train_data_current_m = (struct train_model_current_m*) malloc(current_capacity_current_m * sizeof(struct train_model_current_m));
	    if (train_data_current_m == NULL) {
	        printf("Memory allocation failed for current_m.\n");
	        return 1;
	    }

	// Initialize the train_data with some initial values
	train_data_current_m[0].true_value_current_m = 17.33; train_data_current_m[0].model_value_current_m = 7.754025;
	train_data_current_m[1].true_value_current_m = 20.171; train_data_current_m[1].model_value_current_m = 8.544757;
	train_data_current_m[2].true_value_current_m = 26.46; train_data_current_m[2].model_value_current_m = 10.517456;
	train_data_current_m[3].true_value_current_m = 44.69; train_data_current_m[3].model_value_current_m = 16.024487;
	train_data_current_m[4].true_value_current_m = 65.90; train_data_current_m[4].model_value_current_m = 21.729105;
	train_data_current_m[5].true_value_current_m = 78; train_data_current_m[5].model_value_current_m = 25.561581;
	train_data_current_m[6].true_value_current_m = 83.40; train_data_current_m[6].model_value_current_m = 29.176516;



	int initial_capacity_volt = 9;
	int current_capacity_volt = initial_capacity_volt;
	int current_size_volt = 9;


	train_data_volt = (struct train_model_volt*) malloc(current_capacity_volt * sizeof(struct train_model_volt));
	    if (train_data_volt == NULL) {
	        printf("Memory allocation failed for voltage.\n");
	        return 1;
	    }

	// Initialize the train_data with some initial values
	train_data_volt[1].true_value_volt = 2.005; train_data_volt[1].model_value_volt = 2.771551;
	train_data_volt[0].true_value_volt = 1.005; train_data_volt[0].model_value_volt = 1.896158;
	train_data_volt[2].true_value_volt = 3.005; train_data_volt[2].model_value_volt = 3.626873;
	train_data_volt[3].true_value_volt = 4.005; train_data_volt[3].model_value_volt = 4.486199;
	train_data_volt[4].true_value_volt = 5.005; train_data_volt[4].model_value_volt = 5.361621;
	train_data_volt[5].true_value_volt = 5.005; train_data_volt[5].model_value_volt = 5.361621;
	train_data_volt[6].true_value_volt = 6.005; train_data_volt[6].model_value_volt = 6.233072;
	train_data_volt[7].true_value_volt = 9.005; train_data_volt[7].model_value_volt = 8.815059;
	train_data_volt[8].true_value_volt = 12.005; train_data_volt[8].model_value_volt = 11.417202;


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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

	HAL_UART_Receive_IT(&huart2, rx_buffer, 4);

	HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADC_data_multi, 2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//		HAL_UART_Transmit(&huart2, "Wouldd You Like To Train The Model: \r\n",
//				strlen("Wouldd You Like To Train The Model: \r\n"), 100);


		HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADC_data_multi, 2);

		while (!convCompleted);

		for (uint8_t i = 0; i < hadc1.Init.NbrOfConversion; i++) {
			ADC_1 = (uint16_t) ADC_data_multi[0];
			ADC_2 = (uint16_t) ADC_data_multi[1];
		}

//		sprintf(data_buffer_multi, "check_1 %hu \r\t\t", ADC_1);
//		HAL_UART_Transmit(&huart2, (uint8_t*) data_buffer_multi,
//				strlen(data_buffer_multi), HAL_MAX_DELAY);
//
//		sprintf(data_buffer_multi, "check_2 %hu \r\n", ADC_2);
//		HAL_UART_Transmit(&huart2, (uint8_t*) data_buffer_multi,
//				strlen(data_buffer_multi), HAL_MAX_DELAY);


		while(k<NMSAMPLES) {
			samples_for_ADC_1[k] = ADC_1;
			samples_for_ADC_2[k] = ADC_2;
			k++;
		}

		k = 0;
		for(int z = 0; z<NMSAMPLES; z++){
			average_ADC_1 += samples_for_ADC_1[z];
			average_ADC_2 += samples_for_ADC_2[z];
		}

		average_ADC_1 /= NMSAMPLES;
		average_ADC_2 /= NMSAMPLES;

//		sprintf(buffer, "Analog read for Current = %.4f \r\n", average_ADC_1);
//		HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//		HAL_MAX_DELAY);
//
//		sprintf(buffer, "Vnalog read for Current = %.4f \r\n", average_ADC_2);
//		HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//		HAL_MAX_DELAY);

//		sprintf(data_buffer_multi, "check_1 %hu \r\t\t", ADC_1);
//		HAL_UART_Transmit(&huart2, (uint8_t*) data_buffer_multi, strlen(data_buffer_multi), HAL_MAX_DELAY);
//
//		sprintf(data_buffer_multi, "check_2 %hu \r\n", ADC_2);
//		HAL_UART_Transmit(&huart2, (uint8_t*) data_buffer_multi, strlen(data_buffer_multi), HAL_MAX_DELAY);

		HAL_Delay(10);
	}
	free(train_data);
//	average_ADC_1 = 0;
//	average_ADC_2 = 0;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(huart);
	/* NOTE: This function should not be modified, when the callback is needed,
	 the HAL_UART_RxCpltCallback could be implemented in the user file
	 */

	uint8_t i;

	if (huart->Instance == USART2) {


		rx_buffer[4] = '\0';

		if (strcmp((char*) rx_buffer, "yess\0") == 0) {
			HAL_UART_Transmit(&huart2, "inside_yes", strlen("inside_yes"), 100);

		} else if (strcmp((char*) rx_buffer, "nouA\0") == 0 || strcmp((char*) rx_buffer, "nooV\0") == 0 || strcmp((char*) rx_buffer, "nomA\0") == 0) {

			if(strcmp((char*) rx_buffer, "nouA\0") == 0){

				float result_current = (((3.3f / 4095) * average_ADC_1) / Res_1);

				struct line_of_best_fit result = calculate_line_of_best_fit(
						train_data, 10);

				float current_to_show = (result.slope) * result_current
						+ (result.intercept);

//				sprintf(buffer, "Analog read for Current = %.6f \r\n", average_ADC_1);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);
//
//
//				sprintf(buffer, "Result_currrent %.6f \r\n", result_current);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);


//				sprintf(buffer, "Intersept Reading %.6f \r\n", result.intercept);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);

//				sprintf(buffer, "Slope Reading %.6f \r\n", result.slope);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);

				sprintf(buffer, "%.6f \r\n",
						current_to_show - 2.000);
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
				HAL_MAX_DELAY);

				flag_state = 1;
				flag_state_value = current_to_show;

				}
			else if(strcmp((char*) rx_buffer, "nomA\0") == 0){
				float result_current_m = (((3.3f / 4095) * average_ADC_1) / Res_2);

				struct line_of_best_fit result = calculate_line_of_best_fit_current_m(train_data_current_m, 7);

				float current_to_show_m = (result.slope) * result_current_m
						+ (result.intercept);

//				sprintf(buffer, "Analog read for Current = %.6f \r\n",
//						average_ADC_1);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);
//
//				sprintf(buffer, "Result_currrent %.6f \r\n", result_current_m);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);

//				sprintf(buffer, "Intersept Reading %.6f \r\n",
//						result.intercept);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);
//
//				sprintf(buffer, "Slope Reading %.6f \r\n", result.slope);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);

				sprintf(buffer, "%.6f \r\n",
						current_to_show_m);
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
				HAL_MAX_DELAY);

				flag_state = 2;
				flag_state_value = current_to_show_m;
			}
			else{

				float result_intermediate_volt = ((3.3f / 4095) * average_ADC_2);

				float result_volt = ((2*Res_3 + Res_4)/(Res_4))*result_intermediate_volt;

				struct line_of_best_fit result = calculate_line_of_best_fit_volt(
						train_data_volt, 9);

				float voltage_to_show = (result.slope) * result_volt
						+ (result.intercept);

//				sprintf(buffer, "Analog read for Voltage = %.6f \r\n", average_ADC_2);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);
//
//				sprintf(buffer, "      Result_volt =  %.6f \r\n",
//						result_volt);
//				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//				HAL_MAX_DELAY);


				sprintf(buffer, "%.6f \r\n",
						voltage_to_show);
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
				HAL_MAX_DELAY);

				flag_state = 3;
				flag_state_value = voltage_to_show;
			}
//
//			HAL_UART_Transmit(&huart2, "inside_no", strlen("inside_no"), 100);

//			for (i = 0; i < NMSAMPLES; i++) {
//				HAL_ADC_Start(&hadc1);
//				HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
//				samples[i] = HAL_ADC_GetValue(&hadc1);
//			}
//
//			for (i = 0; i < NMSAMPLES; i++) {
//				average += samples[i];
//			}
//			average /= NMSAMPLES;
//
//			float result_current = (((3.3f / 4095) * average) / Res_1);
//
//			struct line_of_best_fit result = calculate_line_of_best_fit(
//					train_data, 5);
//
//			float current_to_show = (result.slope) * result_current
//					+ (result.intercept);
//
//			sprintf(buffer, "analog average  %.2f \r\n", average);
//			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//			HAL_MAX_DELAY);
//
////			sprintf(buffer, "Intersept Reading %.6f \r\n", result.intercept);
////			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
////			HAL_MAX_DELAY);
//
////			sprintf(buffer, "Slope Reading %.6f \r\n", result.slope);
////			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
////			HAL_MAX_DELAY);
////
////			sprintf(buffer, "Sense current %.6f \r\n", result_current);
////			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
////			HAL_MAX_DELAY);
//
//			sprintf(buffer, "Average Current To Reading %.6f \r\n",
//					current_to_show);
//			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//			HAL_MAX_DELAY);
//
//			sprintf(buffer, "                             \r\n");
//			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer),
//			HAL_MAX_DELAY);
//
//			average = 0; // clear the previous average value;

		}
		else if(strcmp((char*) rx_buffer, "nooR\0") == 0){
			softwareReset();
		}
//		else if(strcmp((char*) rx_buffer, "nooS\0") == 0){
//			if (num_stored_readings < MAX_STORED_READINGS) {
//
//				if (flag_state == 1) {
//					stored_readings[num_stored_readings] = flag_state_value;
//					store_in_flash(FLASH_STORAGE_ADDRESS, stored_readings,
//							num_stored_readings + 1);
//					num_stored_readings++;
//
//					char msg[] = "Reading stored successfully\r\n";
//					HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
//											HAL_MAX_DELAY);
//
//				} else if (flag_state == 2) {
//
//				} else if (flag_state == 3) {
//
//				}
//
//			} else {
//				char msg[] = "Storage full! Cannot store more readings\r\n";
//				HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
//						HAL_MAX_DELAY);
//			}
//
//		}
		else {
			HAL_UART_Transmit(&huart2, "Error",
					strlen("Error"), 100);
		}
		HAL_UART_Transmit(&huart2, "\r\n", 2, 100);

//		for (i = 0; i < 5; i++) {
//			rx_buffer[i] = 0;
//		}

		HAL_UART_Receive_IT(&huart2, rx_buffer, 4);
	}
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
	__disable_irq();
	while (1) {
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
