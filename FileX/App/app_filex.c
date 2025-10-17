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
#include "stdio.h"
#include <stdarg.h>
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Main thread stack size */
#define FX_APP_THREAD_STACK_SIZE         50000
/* Main thread priority */
#define FX_APP_THREAD_PRIO               10
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* USER CODE BEGIN PV */
ALIGN_32BYTES (uint32_t fx_nor_ospi_media_memory[LX_STM32_OSPI_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;

//FX_MEDIA sram_disk;
/* FileX file instance */
FX_FILE fx_file;
uint8_t formatear;
uint8_t FormateoMemeoria;
uint8_t CrearArchivo;
uint8_t fRead;
uint8_t fcarpeta;
int t;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */
void Formateo();
int Custom_Sprintf(FX_FILE *NomArchivo ,const char *format,...);
void CrearArchi(void);
void LecturaTexto(void);
void CreacionCarpeta(char *NomCarpeta);
extern void W25Q128EnableWrite(void);
extern void W25Q128BorradoCompleto();
/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
*/
UINT MX_FileX_Init(VOID *memory_ptr)
{
  UINT ret = FX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  VOID *pointer;

/* USER CODE BEGIN MX_FileX_MEM_POOL */

/* USER CODE END MX_FileX_MEM_POOL */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*Allocate memory for the main thread's stack*/
  ret = tx_byte_allocate(byte_pool, &pointer, FX_APP_THREAD_STACK_SIZE, TX_NO_WAIT);

/* Check FX_APP_THREAD_STACK_SIZE allocation*/
  if (ret != FX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

/* Create the main thread.  */
  ret = tx_thread_create(&fx_app_thread, FX_APP_THREAD_NAME, fx_app_thread_entry, 0, pointer, FX_APP_THREAD_STACK_SIZE,
                         FX_APP_THREAD_PRIO, FX_APP_PREEMPTION_THRESHOLD, FX_APP_THREAD_TIME_SLICE, FX_APP_THREAD_AUTO_START);

/* Check main thread creation */
  if (ret != FX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

/* USER CODE BEGIN MX_FileX_Init */

/* USER CODE END MX_FileX_Init */

/* Initialize FileX.  */
  fx_system_initialize();

/* USER CODE BEGIN MX_FileX_Init 1*/

/* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/**
 * @brief  Main thread entry.
 * @param thread_input: ULONG user argument used by the thread entry
 * @retval none
*/
 void fx_app_thread_entry(ULONG thread_input)
 {

/* USER CODE BEGIN fx_app_thread_entry 0*/
	 while (1)
	   {
	     /* USER CODE END WHILE */

	     /* USER CODE BEGIN 3 */
	 	  if(formatear == 1)
	 	  {
	 		  DWT->CTRL |= 0x1UL;
	 		  DWT->CYCCNT = 0;
	 		  formatear = 0;
	 	  	  Formateo();
	 	  	  t = DWT->CYCCNT;
	 	  }
	 	  if(FormateoMemeoria == 1)
	 	  {
	 		  FormateoMemeoria = 0;
	 		  W25Q128BorradoCompleto();
	 	  }
	 	  if(CrearArchivo != 0)
	 	  {
	 		  CrearArchivo = 0;
	 		  CrearArchi();
	 	  }
	 	  if (fRead)
	 	  {
	 		fRead = 0;
	 		LecturaTexto();
	 	  }
	 		if (fcarpeta) {
	 			fcarpeta = 0;
	 			CreacionCarpeta("Carpeta1");
	 		}
	   }
/* USER CODE END fx_app_thread_entry 0*/

/* USER CODE BEGIN fx_app_thread_entry 1*/

/* USER CODE END fx_app_thread_entry 1*/
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
							(LX_STM32_OSPI_FLASH_SIZE - LX_STM32_OSPI_SECTOR_SIZE)/ 512,/* Total sectores lógicos */
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
void CrearArchi(void)
 {
	UINT status;
	CHAR data[1024] = "Texto aburrido";
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	status = fx_media_open(&sdio_disk, "STM32_SRAM_DISK", fx_stm32_levelx_nor_driver,
			(VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory,
			sizeof(fx_nor_ospi_media_memory));

	/* Check the media open status. */
	if (status != FX_SUCCESS) {
		Error_Handler();
	}
	//fx_file_delete(&sdio_disk, "STM32.TXT");
	status = fx_file_delete(&sdio_disk, "STM32.txt");
	if (status == FX_NOT_FOUND || status == FX_SUCCESS) {
		status = fx_file_create(&sdio_disk, "STM32.txt");
		if (status != FX_SUCCESS) {
			Error_Handler();
		}
	}

	status = fx_file_open(&sdio_disk, &fx_file, "STM32.txt", FX_OPEN_FOR_WRITE);

	/* Check the file open status. */
	if (status != FX_SUCCESS) {
		/* Error opening file, call error handler. */
		Error_Handler();
	}

	/* Seek to the beginning of the test file. */
	status = fx_file_seek(&fx_file, 0);

	/* Check the file seek status. */
	if (status != FX_SUCCESS) {
		/* Error performing file seek, call error handler. */
		Error_Handler();
	}
	/* Write a string to the test file. */
	int A = 0;
	int B = -34252;
	status = fx_file_write(&fx_file, data, strlen(data));

	/* Check the file write status. */
	if (status != FX_SUCCESS) {
		/* Error writing to a file, call error handler. */
		Error_Handler();
	}
	Custom_Sprintf(&fx_file, "tEXT %d %d", A, B);

	/* Close the test file. */
	status = fx_file_close(&fx_file);

	/* Check the file close status. */
	if (status != FX_SUCCESS) {
		/* Error closing the file, call error handler. */
		Error_Handler();
	}

	status = fx_media_flush(&sdio_disk);

	/* Check the media flush  status. */
	if (status != FX_SUCCESS) {
		/* Error closing the file, call error handler. */
		Error_Handler();
	}
	status = fx_media_close(&sdio_disk);

	/* Check the media close status. */
	if (status != FX_SUCCESS) {
		/* Error closing the media, call error handler. */
		Error_Handler();
	}
}
void CreacionCarpeta(char *NomCarpeta)
 {
 	UINT status;
 		CHAR data[1024] = "Texto aburrido";
	status = fx_media_open(&sdio_disk, "STM32_SRAM_DISK",
			fx_stm32_levelx_nor_driver, (VOID *)LX_NOR_OSPI_DRIVER_ID,
			(VOID *) fx_nor_ospi_media_memory,
			sizeof(fx_nor_ospi_media_memory));

 	uint8_t Dato[40];
 	Dato[0] = '\\';
 	//Dato[1] = '\\';
 	sprintf((char *)Dato +1,NomCarpeta);
 	status = fx_directory_default_set(&sdio_disk,(CHAR *)Dato);
 	if(status == FX_INVALID_PATH)
 	{
 		status = fx_directory_create(&sdio_disk,NomCarpeta);
 		if (status != FX_SUCCESS)
 			Error_Handler();
 	}
 	status = fx_directory_default_set(&sdio_disk,(CHAR *)Dato);
 	if (status != FX_SUCCESS)
 		Error_Handler();
 	status = fx_file_delete(&sdio_disk, "STM32.txt");
 		if(status == FX_NOT_FOUND)
 		{
 			  status = fx_file_create(&sdio_disk, "STM32.txt");
 			  if(status != FX_SUCCESS)
 			  {
 				  Error_Handler();
 			  }
 		}

 		status = fx_file_open(&sdio_disk, &fx_file, "STM32.txt", FX_OPEN_FOR_WRITE);

 		/* Check the file open status. */
 		if (status != FX_SUCCESS)
 		{
 			/* Error opening file, call error handler. */
 			Error_Handler();
 		}

 		/* Seek to the beginning of the test file. */
 		status = fx_file_seek(&fx_file, 0);

 		/* Check the file seek status. */
 		if (status != FX_SUCCESS)
 		{
 			/* Error performing file seek, call error handler. */
 			Error_Handler();
 		}
 		/* Write a string to the test file. */
 		int A = 0;
 		int B = -34252;
 		status = fx_file_write(&fx_file, data, strlen(data));

 		/* Check the file write status. */
 		if (status != FX_SUCCESS)
 		{
 			/* Error writing to a file, call error handler. */
 			Error_Handler();
 		}
 		Custom_Sprintf(&fx_file, "tEXT %d %d", A , B);

 		/* Close the test file. */
 		status = fx_file_close(&fx_file);

 		/* Check the file close status. */
 		if (status != FX_SUCCESS)
 		{
 			/* Error closing the file, call error handler. */
 			Error_Handler();
 		}

 		status = fx_media_flush(&sdio_disk);

 		/* Check the media flush  status. */
 		if (status != FX_SUCCESS)
 		{
 			/* Error closing the file, call error handler. */
 			Error_Handler();
 		        }
 		status = fx_media_close(&sdio_disk);

 			/* Check the media close status. */
 			if (status != FX_SUCCESS)
 			{
 				/* Error closing the media, call error handler. */
 				Error_Handler();
 			}
 }
int Custom_Sprintf(FX_FILE *Archivo,const char *format,...)
{
	UINT status;
	char buffer[1024] ={ 0 };
	va_list args;
	va_start(args, format);
	int i;

	i = vsprintf(buffer, format, args);
	va_end(args);
	//HAL_GPIO_WritePin(DebugGPIO_GPIO_Port, DebugGPIO_Pin, GPIO_PIN_RESET);
	fx_file_relative_seek(Archivo,0,FX_SEEK_END);
	//HAL_GPIO_WritePin(DebugGPIO_GPIO_Port, DebugGPIO_Pin, GPIO_PIN_SET);
	status = fx_file_write(&fx_file, buffer, strlen(buffer));
	//HAL_GPIO_WritePin(DebugGPIO_GPIO_Port, DebugGPIO_Pin, GPIO_PIN_RESET);
		/* Check the file write status. */
	if (status != FX_SUCCESS)
		{
			/* Error writing to a file, call error handler. */
			Error_Handler();
		}
	return i;
}
void LecturaTexto(void)
{
	UINT status;
	ULONG bytes_read;
	CHAR read_buffer[50];

	/* Open the sdio_disk driver. */
	status = fx_media_open(&sdio_disk, "STM32_SRAM_DISK", fx_stm32_levelx_nor_driver,
			(VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory, sizeof(fx_nor_ospi_media_memory));

	/* Check the media open status. */
	if (status != FX_SUCCESS)
	{
		Error_Handler();
	}
	/* Open the test file. */
	status = fx_file_open(&sdio_disk, &fx_file, "STM32.txt", FX_OPEN_FOR_READ);

	/* Check the file open status. */
	if (status != FX_SUCCESS)
	{
		/* Error opening file, call error handler. */
		Error_Handler();
	}

	/* Seek to the beginning of the test file. */
	status = fx_file_seek(&fx_file, 0);

	/* Check the file seek status. */
	if (status != FX_SUCCESS)
	{
		/* Error performing file seek, call error handler. */
		Error_Handler();
	}

	/* Read the first 28 bytes of the test file. */
	status = fx_file_read(&fx_file, read_buffer, 22, &bytes_read);

	/* Check the file read status. */
	if ((status != FX_SUCCESS) || (bytes_read != 22))
	{
		/* Error reading file, call error handler. */
		Error_Handler();
	}

	/* Close the test file. */
	status = fx_file_close(&fx_file);

	/* Check the file close status. */
	if (status != FX_SUCCESS)
	{
		/* Error closing the file, call error handler. */
		Error_Handler();
	}

	/* Close the media. */
	status = fx_media_close(&sdio_disk);

	/* Check the media close status. */
	if (status != FX_SUCCESS)
	{
		/* Error closing the media, call error handler. */
		Error_Handler();
	}
}
/* USER CODE END 1 */
