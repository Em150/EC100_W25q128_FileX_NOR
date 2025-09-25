/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ALIGN_32BYTES (uint32_t fx_nor_ospi_media_memory[LX_STM32_OSPI_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;

//FX_MEDIA sram_disk;
/* FileX file instance */
FX_FILE fx_file;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
void Formateo();
/* USER CODE END PFP */
/**
  * @brief  Application FileX Initialization.
  * @param  None
  * @retval int
  */
UINT MX_FileX_Init(void)
{
  UINT ret = FX_SUCCESS;
  /* USER CODE BEGIN MX_FileX_Init */

  /* USER CODE END MX_FileX_Init */

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE BEGIN MX_FileX_Init 1*/

  /* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/* USER CODE BEGIN 1 */
void Formateo()
{
	UINT status;
	status = fx_media_format(
	            &sdio_disk,                 /* Control block */
	            fx_stm32_levelx_nor_driver, /* Driver entry */
	            (VOID *)LX_NOR_OSPI_DRIVER_ID,                  /* Device info pointer */
	            (UCHAR *)fx_nor_ospi_media_memory,/* Media buffer */
				sizeof(fx_nor_ospi_media_memory),                        /* Media buffer size en bytes */
	            "DEAD",                     /* Volume Name (4 caracteres) */
	            1,                          /* Número de FATs */
	            32,                         /* Directorio raíz (entradas) */
	            0,                          /* Sectores ocultos */
	            32768,                      /* Total sectores lógicos */
	            512,                        /* Tamaño de sector lógico en bytes */
	            8,                          /* Sectores por clúster */
	            1,                          /* Número de cabezas */
	            1                           /* Sectores por pista */
	        );                                     // Sectors per track

	  /* Check the format status */
	  if (status != FX_SUCCESS)
	  {
	    Error_Handler();
	  }
}
/* USER CODE END 1 */
