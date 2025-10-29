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
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Main thread stack size */
#define FX_APP_THREAD_STACK_SIZE         14000
/* Main thread priority */
#define FX_APP_THREAD_PRIO               10
/* USER CODE BEGIN PD */
#define FX_NOR_OSPI_SECTOR_SIZE 512
#define FX_NOR_OSPI_VOLUME_NAME "Memoria W25Q128"
#define FX_NOR_OSPI_NUMBER_OF_FATS 1
#define FX_NOR_OSPI_HIDDEN_SECTORS 0
#define MAX_PATH_LEN 260
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* Buffer for FileX FX_MEDIA sector cache. */
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;

/* Buffer for FileX FX_MEDIA sector cache. */
ALIGN_32BYTES (uint32_t fx_nor_ospi_media_memory[FX_NOR_OSPI_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        nor_ospi_flash_disk;

/* USER CODE BEGIN PV */



/* FileX file instance */
//FX_FILE fx_file;
uint8_t FormSD;
uint8_t FormMem;
uint8_t BorradoMem;
uint8_t CrearArchivo;
uint8_t fRead;
uint8_t fcarpeta;
uint64_t t;
uint8_t SdHaciaMEM;
uint8_t MEMHaciaSD;
ULONG TotalBytes;
ULONG bytes_escritos;
UCHAR buffer[4096];
CHAR DIRECTORIO[256];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */
void FormateoMEM();
void FormateoSD();
int Custom_Sprintf(FX_FILE *NomArchivo ,const char *format,...);
void CrearArchi(void);
void LecturaTexto(void);
void CreacionCarpeta(char *NomCarpeta);
extern void W25Q128EnableWrite(void);
extern void W25Q128BorradoCompleto();
static UINT fx_copy_all(FX_MEDIA *mediaFrom, FX_MEDIA *mediaTo);
void CopiarDesdeSdHaciaMEM(void);
void CopiarDesdeMEMHaciaSD(void);
static UINT fx_copy_file(FX_MEDIA *mediaFrom,FX_MEDIA *mediaTo, const CHAR *src_name, const CHAR *dst_name);

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

  UINT sd_status = FX_SUCCESS;

  UINT nor_ospi_status = FX_SUCCESS;

/* USER CODE BEGIN fx_app_thread_entry 0*/

/* USER CODE END fx_app_thread_entry 0*/

/* Open the SD disk driver */
  sd_status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

/* Check the media open sd_status */
  if (sd_status != FX_SUCCESS)
  {
     /* USER CODE BEGIN SD DRIVER get info error */
		while (1);
    /* USER CODE END SD DRIVER get info error */
  }

  /* Open the OCTO-SPI NOR driver */
 nor_ospi_status =  fx_media_open(&nor_ospi_flash_disk, FX_NOR_OSPI_VOLUME_NAME, fx_stm32_levelx_nor_driver, (VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory, sizeof(fx_nor_ospi_media_memory));

/* Check the media open nor_ospi_status */
  if (nor_ospi_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN OCTO-SPI NOR open error */
		while (1);
    /* USER CODE END OCTO-SPI NOR open error */
  }

/* USER CODE BEGIN fx_app_thread_entry 1*/
	while (1)
	{
		if (FormMem != 0)
		{
			DWT->CTRL |= 0x1UL;
			DWT->CYCCNT = 0;
			FormMem = 0;
			FormateoMEM();
			t = DWT->CYCCNT;
		}
		if (FormSD != 0)
		{
			DWT->CTRL |= 0x1UL;
			DWT->CYCCNT = 0;
			FormSD = 0;
			FormateoSD();
			t = DWT->CYCCNT;
		}
		if (BorradoMem != 0)
		{
			BorradoMem = 0;
			W25Q128BorradoCompleto();
		}
		if (CrearArchivo != 0)
		{
			CrearArchivo = 0;
			CrearArchi();
		}
		if (fRead)
		{
			fRead = 0;
			LecturaTexto();
		}
		if (fcarpeta)
		{
			fcarpeta = 0;
			CreacionCarpeta("Carpeta1");
		}
		if(SdHaciaMEM)
		{
			DWT->CTRL |= 0x1UL;
			DWT->CYCCNT = 0;
			SdHaciaMEM = 0;
			CopiarDesdeSdHaciaMEM();
			t = DWT->CYCCNT;
		}
		if (MEMHaciaSD)
		{
			DWT->CTRL |= 0x1UL;
			DWT->CYCCNT = 0;
			MEMHaciaSD = 0;
			CopiarDesdeMEMHaciaSD();
			t = DWT->CYCCNT;
		}
	}
/* USER CODE END fx_app_thread_entry 1*/
  }

/* USER CODE BEGIN 1 */
void CopiarDesdeSdHaciaMEM(void)
{
	uint8_t status = 0;
//	status = fx_directory_local_path_set(&sdio_disk, &PathSD, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_set(&nor_ospi_flash_disk, &PathMEM, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_set(&sdio_disk, &pathFrom, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_set(&sdio_disk, &pathTo, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}

	for(int i = 0; i < 256;i++)
		DIRECTORIO[i] = 0;
	status = fx_copy_all(&sdio_disk, &nor_ospi_flash_disk);
	if (status != 0)
	{
		while (1);
	}
//	status = fx_directory_local_path_clear(&sdio_disk);
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_clear(&nor_ospi_flash_disk);
//	if (status != 0)
//	{
//		while (1);
//	}

}
void CopiarDesdeMEMHaciaSD(void)
{
	uint8_t status = 0;
//	status = fx_directory_local_path_set(&sdio_disk, &PathSD, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_set(&nor_ospi_flash_disk, &PathMEM, "\\");
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_copy_all_root_files(&nor_ospi_flash_disk, &sdio_disk);
//	if (status != 0)
//	{
//		while (1);
//	}
	status = fx_copy_all(&nor_ospi_flash_disk, &sdio_disk);
	if (status != 0)
	{
		while (1);
	}
//	status = fx_directory_local_path_clear(&sdio_disk);
//	if (status != 0)
//	{
//		while (1);
//	}
//	status = fx_directory_local_path_clear(&nor_ospi_flash_disk);
//	if (status != 0)
//	{
//		while (1);
//	}
}
static UINT fx_copy_file(FX_MEDIA *mediaFrom,FX_MEDIA *mediaTo, const CHAR *src_name, const CHAR *dst_name)
{
	UINT status;
	FX_FILE src_file;
	FX_FILE dst_file;

	ULONG bytes_read = 0;



	/* Abrir archivo origen para lectura */
	status = fx_file_open(mediaFrom, &src_file, (CHAR* )src_name, FX_OPEN_FOR_READ);
	if (status != FX_SUCCESS)
		return status;

	/* Crear/abrir archivo destino para escritura (si existe, se sobrescribe) */
	status = fx_file_create(mediaTo, (CHAR*) dst_name);
	if ((status != FX_SUCCESS) && (status != FX_ALREADY_CREATED))
	{
		fx_file_close(&src_file);
		return status;
	}

	status = fx_file_open(mediaTo, &dst_file, (CHAR* )dst_name, FX_OPEN_FOR_WRITE);
	if (status != FX_SUCCESS)
	{
		fx_file_close(&src_file);
		return status;
	}

	/* Colocar el puntero de destino al inicio */
	status = fx_file_seek(&dst_file, 0);
	if (status != FX_SUCCESS)
		goto copy_error;

	/* Bucle de lectura/escritura */
	TotalBytes = src_file.fx_file_current_file_size;
	bytes_escritos = 0;
	status = fx_file_allocate(&dst_file, TotalBytes);
	while (bytes_escritos < TotalBytes)
	{
		if (TotalBytes - bytes_escritos  > sizeof(buffer))
			status = fx_file_read(&src_file, buffer,sizeof(buffer) , &bytes_read);
		else
			status = fx_file_read(&src_file, buffer, TotalBytes - bytes_escritos, &bytes_read);


		/* Si no hay bytes leídos, terminamos */
		if (bytes_read == 0)
		{
			status = FX_SUCCESS;
			break;
		}

		/* Escribir los bytes leídos en el archivo destino */
		status = fx_file_write(&dst_file, buffer, bytes_read);
		if (status != FX_SUCCESS)
			goto copy_error;
		else
			bytes_escritos += sizeof(buffer);
	}
	/* Asegurar datos en media */
	status = fx_media_flush(mediaTo);
	if (status != FX_SUCCESS)
		goto copy_error;

	/* Cerrar archivos */
	fx_file_close(&src_file);
	fx_file_close(&dst_file);

	return FX_SUCCESS;

	copy_error:
	/* Intentar cerrar archivos si es necesario */
	fx_file_close(&src_file);
	fx_file_close(&dst_file);
	return status;
}

static UINT fx_copy_all(FX_MEDIA *mediaFrom, FX_MEDIA *mediaTo)
{
    UINT status;
    CHAR entry_name[FX_MAX_LONG_NAME_LEN + 1];
    UINT attributes;
    ULONG size;
    UINT year, month, day, hour, minute, second;
    CHAR current_src_dir[MAX_PATH_LEN];
    CHAR current_dst_dir[MAX_PATH_LEN];

    /* Guardar directorios actuales */
    fx_directory_default_get(mediaFrom, current_src_dir);
    fx_directory_default_get(mediaTo, current_dst_dir);

    /* Obtener primera entrada del directorio actual */
    status = fx_directory_first_full_entry_find(mediaFrom,
                                                entry_name,
                                                &attributes,
                                                &size,
                                                &year,
                                                &month,
                                                &day,
                                                &hour,
                                                &minute,
                                                &second);
    if (status != FX_SUCCESS)
    {
        return status;
    }

    while (status == FX_SUCCESS)
    {
        UINT should_copy = FX_TRUE;

        /* 1️⃣ Filtros básicos */
        if ((strcmp(entry_name, ".") == 0) || (strcmp(entry_name, "..") == 0))
        {
            should_copy = FX_FALSE;
        }

        /* 2️⃣ Ignorar atributos de sistema / ocultos / volumen */
        if (should_copy &&
            ((attributes & FX_SYSTEM) || (attributes & FX_HIDDEN) || (attributes & FX_VOLUME)))
        {
            should_copy = FX_FALSE;
        }

//        /* 3️⃣ Ignorar nombres específicos de sistema */
//        if (should_copy &&
//            ((strcasecmp(entry_name, "System Volume Information") == 0) ||
//             (strcasecmp(entry_name, "$RECYCLE.BIN") == 0) ||
//             (strcasecmp(entry_name, "FOUND.000") == 0)))
//        {
//            should_copy = FX_FALSE;
//        }

        /* 4️⃣ Solo copiar si pasó los filtros */
        if (should_copy)
        {
            if (attributes & FX_DIRECTORY)
            {
                /* Crear subdirectorio en destino */
                status = fx_directory_create(mediaTo, entry_name);
                if ((status != FX_SUCCESS) && (status != FX_ALREADY_CREATED))
                {
                    return status;
                }

                /* Cambiar a subdirectorio */
                status = fx_directory_default_set(mediaFrom, entry_name);
                if (status == FX_SUCCESS)
                {
                    status = fx_directory_default_set(mediaTo, entry_name);
                }

                if (status == FX_SUCCESS)
                {
                    /* Copiar contenido del subdirectorio */
                    status = fx_copy_all(mediaFrom, mediaTo);

                    /* Regresar a directorio anterior */
                    fx_directory_default_set(mediaFrom, current_src_dir);
                    fx_directory_default_set(mediaTo, current_dst_dir);
                }

                if (status != FX_SUCCESS)
                {
                    return status;
                }
            }
            else
            {
                /* Archivo normal */
                status = fx_copy_file(mediaFrom, mediaTo, entry_name, entry_name);
                if (status != FX_SUCCESS)
                {
                    return status;
                }
            }
        }

        /* 5️⃣ Ir a la siguiente entrada */
        status = fx_directory_next_full_entry_find(mediaFrom,
                                                   entry_name,
                                                   &attributes,
                                                   &size,
                                                   &year,
                                                   &month,
                                                   &day,
                                                   &hour,
                                                   &minute,
                                                   &second);
    }

    /* Si no hay más entradas, finalizamos con éxito */
    if (status == FX_NO_MORE_ENTRIES)
    {
        status = FX_SUCCESS;
    }

    return status;
}
void FormateoSD()
{
	UINT status;
	status = fx_media_close(&sdio_disk);

	if (status == FX_SUCCESS || status == FX_NOT_OPEN)
	{
		status = fx_media_format(&sdio_disk,                              // RamDisk pointer
				fx_stm32_sd_driver,                    // Driver entry
				(VOID*) FX_NULL,                         // Device info pointer
				(UCHAR*) fx_sd_media_memory,                  // Media buffer pointer
				512,                                     // Media buffer size
				"SD Card 2GB",                       // Volume Name
				1,                                       // Number of FATs
				32,                                      // Directory Entries
				0,                                       // Hidden sectors
				4194304,										 // Total sectors
				FX_STM32_SD_DEFAULT_SECTOR_SIZE,                     // Sector size
				2,                                       // Sectors per cluster
				1,                                       // Heads
				1);                                      // Sectors per track

		/* Check the format status */
		if (status != FX_SUCCESS)
		{
			Error_Handler();
		}
	}
	else if (status != FX_SUCCESS)
	{
		/* Error closing the media, call error handler. */
		Error_Handler();
	}

	status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

/* Check the media open sd_status */
  if (status != FX_SUCCESS)
  {
     /* USER CODE BEGIN SD DRIVER get info error */
		while (1);
    /* USER CODE END SD DRIVER get info error */
  }
}
void FormateoMEM()
{
	UINT status;
	status = fx_media_close(&nor_ospi_flash_disk);

	/* Check the media close status. */
	if(status == FX_SUCCESS || status == FX_NOT_OPEN)
	{
		status =  				  fx_media_format(&nor_ospi_flash_disk,                                                           // nor_ospi_flash_disk pointer
		                                     fx_stm32_levelx_nor_driver,                                                     // Driver entry
		                                     (VOID *)LX_NOR_OSPI_DRIVER_ID,                                                  // Device info pointer
		                                     (UCHAR *) fx_nor_ospi_media_memory,                                             // Media buffer pointer
		                                     sizeof(fx_nor_ospi_media_memory),                                               // Media buffer size
		                                     FX_NOR_OSPI_VOLUME_NAME,                                                        // Volume Name
		                                     FX_NOR_OSPI_NUMBER_OF_FATS,                                                     // Number of FATs
		                                     32,                                                                             // Directory Entries
		                                     FX_NOR_OSPI_HIDDEN_SECTORS,                                                     // Hidden sectors
		                                     (LX_STM32_OSPI_FLASH_SIZE - LX_STM32_OSPI_SECTOR_SIZE)/ FX_NOR_OSPI_SECTOR_SIZE,// Total sectors minus one
		                                     FX_NOR_OSPI_SECTOR_SIZE,                                                        // Sector size
		                                     8,                                                                              // Sectors per cluster
		                                     1,                                                                              // Heads
		                                     1);                                                                             // Sectors per track
		                                   // Sectors per track

			  /* Check the format status */
			  if (status != FX_SUCCESS)
			  {
			    Error_Handler();
			  }
	}
	else if (status != FX_SUCCESS)
	{
		/* Error closing the media, call error handler. */
		Error_Handler();
	}

	status = fx_media_open(&nor_ospi_flash_disk, FX_NOR_OSPI_VOLUME_NAME, fx_stm32_levelx_nor_driver, (VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory, sizeof(fx_nor_ospi_media_memory));

	/* Check the media open nor_ospi_status */
	if (status != FX_SUCCESS)
	{
		while (1);
	}
}
void CrearArchi(void)
 {
	UINT status;
	CHAR data[1024] = "Texto aburrido";
	FX_FILE fx_file;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	status = fx_media_open(&nor_ospi_flash_disk, "STM32_SRAM_DISK", fx_stm32_levelx_nor_driver,
//			(VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory,
//			sizeof(fx_nor_ospi_media_memory));
//
//	/* Check the media open status. */
//	if (status != FX_SUCCESS) {
//		Error_Handler();
//	}
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
//	status = fx_media_close(&nor_ospi_flash_disk);
//
//	/* Check the media close status. */
//	if (status != FX_SUCCESS) {
//		/* Error closing the media, call error handler. */
//		Error_Handler();
//	}
}
void CreacionCarpeta(char *NomCarpeta)
{
	UINT status;
	CHAR data[1024] = "Texto aburrido";
	FX_FILE fx_file;
//	status = fx_media_open(&nor_ospi_flash_disk, "STM32_SRAM_DISK",
//			fx_stm32_levelx_nor_driver, (VOID *)LX_NOR_OSPI_DRIVER_ID,
//			(VOID *) fx_nor_ospi_media_memory,
//			sizeof(fx_nor_ospi_media_memory));

	uint8_t Dato[40];
	Dato[0] = '\\';
	//Dato[1] = '\\';
	sprintf((char*) Dato + 1, NomCarpeta);
	status = fx_directory_default_set(&sdio_disk, (CHAR*) Dato);
	if (status == FX_INVALID_PATH)
	{
		status = fx_directory_create(&sdio_disk, NomCarpeta);
		if (status != FX_SUCCESS)
			Error_Handler();
	}
	status = fx_directory_default_set(&sdio_disk, (CHAR*) Dato);
	if (status != FX_SUCCESS)
		Error_Handler();
	status = fx_file_delete(&sdio_disk, "STM32.txt");
	if (status == FX_NOT_FOUND)
	{
		status = fx_file_create(&sdio_disk, "STM32.txt");
		if (status != FX_SUCCESS)
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
	Custom_Sprintf(&fx_file, "tEXT %d %d", A, B);

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
// 		status = fx_media_close(&nor_ospi_flash_disk);
//
// 			/* Check the media close status. */
// 			if (status != FX_SUCCESS)
// 			{
// 				/* Error closing the media, call error handler. */
// 				Error_Handler();
// 			}
}
int Custom_Sprintf(FX_FILE *Archivo,const char *format,...)
{
	UINT status;
	char buffer[1024] ={ 0 };
	FX_FILE fx_file;
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
	FX_FILE fx_file;
//	/* Open the sdio_disk driver. */
//	status = fx_media_open(&nor_ospi_flash_disk, "STM32_SRAM_DISK", fx_stm32_levelx_nor_driver,
//			(VOID *)LX_NOR_OSPI_DRIVER_ID, (VOID *) fx_nor_ospi_media_memory, sizeof(fx_nor_ospi_media_memory));
//
//	/* Check the media open status. */
//	if (status != FX_SUCCESS)
//	{
//		Error_Handler();
//	}
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

//	/* Close the media. */
//	status = fx_media_close(&nor_ospi_flash_disk);
//
//	/* Check the media close status. */
//	if (status != FX_SUCCESS)
//	{
//		/* Error closing the media, call error handler. */
//		Error_Handler();
//	}
}
/* USER CODE END 1 */
