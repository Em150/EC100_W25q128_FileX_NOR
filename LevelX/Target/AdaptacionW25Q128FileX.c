/*
 * AdaptacionW25Q128FileX.c
 *
 *  Created on: Sep 24, 2025
 *      Author: Emilio
 */

#include "lx_stm32_ospi_driver.h"
#include "stm32h5xx_hal.h"


extern XSPI_HandleTypeDef hospi1;
extern ULONG ospi_sector_buffer[LX_STM32_OSPI_SECTOR_SIZE / sizeof(ULONG)];

extern __IO UINT ospi_rx_cplt;
extern __IO UINT ospi_tx_cplt;

uint8_t lx_stm32_ospi_get_statusW25Q125(UINT instance);
uint8_t lx_stm32_ospi_lowlevel_initW25Q125(UINT instance);
uint8_t lx_stm32_ospi_eraseW25Q125(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase);
uint8_t lx_stm32_ospi_writeW25Q125(UINT instance, ULONG *address, ULONG *buffer, ULONG words);
static uint8_t ospi_set_write_enableW25Q125(XSPI_HandleTypeDef *hxspi);
static uint8_t lx_stm32_ospi_get_statusBlockingW25Q125(XSPI_HandleTypeDef *hxspi,uint32_t timeout);


uint8_t lx_stm32_ospi_lowlevel_initW25Q125(UINT instance)
{
	uint8_t status = 0;
	XSPI_RegularCmdTypeDef s_command = {0};
	s_command.Instruction = LX_STM32_OSPI_RESET_ENABLE_CMD;
	s_command.InstructionMode       = HAL_XSPI_INSTRUCTION_1_LINE;
	if (HAL_XSPI_Command(&hospi1, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		status = 1;
	s_command.Instruction = LX_STM32_OSPI_RESET_MEMORY_CMD;
	if (HAL_XSPI_Command(&hospi1, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		status = 1;
	return status;
}
static uint8_t lx_stm32_ospi_get_statusBlockingW25Q125(XSPI_HandleTypeDef *hxspi,uint32_t timeout)
{
	XSPI_RegularCmdTypeDef CMD = { 0 };
	XSPI_AutoPollingTypeDef POLL = { 0 };
	CMD.Instruction = 0x05;
	CMD.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
	CMD.DataLength = 1;
	CMD.DataMode = HAL_XSPI_DATA_1_LINE;
	HAL_XSPI_Command(&hospi1, &CMD, 1);

	POLL.MatchMode = HAL_XSPI_MATCH_MODE_AND;
	POLL.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
	POLL.IntervalTime = 0x2FF;
	POLL.MatchValue = 0;
	POLL.MatchMask = 1;
	if(HAL_XSPI_AutoPolling(&hospi1, &POLL, timeout) != HAL_OK)
	{
		return 1;
	}
	return 0;
}
uint8_t lx_stm32_ospi_get_statusW25Q125(UINT instance)
{
	XSPI_RegularCmdTypeDef s_command ={ 0 };
	uint8_t reg,status = 0;
	s_command.Instruction = LX_STM32_OSPI_OCTAL_READ_STATUS_REG_CMD;
	s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
	s_command.DataLength = 1;
	s_command.DataMode = HAL_XSPI_DATA_1_LINE;
	HAL_XSPI_Command(&hospi1, &s_command, 10);
	HAL_XSPI_Receive(&hospi1, &reg, 10);
	if ((reg & LX_STM32_OSPI_SR_WIP) != 0)
	{
		status = 1;
	}
	return status;
}
uint8_t lx_stm32_ospi_writeW25Q125(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
	uint8_t status = 0;
	uint32_t end_addr;
	uint32_t current_addr;
	uint32_t current_size;
	uint32_t data_buffer;
	UINT timeout_start;
	XSPI_RegularCmdTypeDef s_command = { 0 };
	/* Calculation of the size between the write address and the end of the page */
	current_size = LX_STM32_OSPI_PAGE_SIZE - ((uint32_t) address % LX_STM32_OSPI_PAGE_SIZE);

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > (((uint32_t) words) * sizeof(ULONG)))
	{
		current_size = ((uint32_t) words) * sizeof(ULONG);
	}

	/* Initialize the address variables */
	current_addr = (uint32_t) address;
	end_addr = ((uint32_t) address) + ((uint32_t) words) * sizeof(ULONG);
	data_buffer = (uint32_t) buffer;

	/* Initialize the program command */
	s_command.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
	s_command.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
	s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
	s_command.DataMode = HAL_XSPI_DATA_4_LINES;
	s_command.Instruction = LX_STM32_OSPI_OCTAL_PAGE_PROG_CMD;

	do
	{
		s_command.Address = current_addr;
		s_command.DataLength = current_size;

		/* Enable write operations */
		if (ospi_set_write_enableW25Q125(&hospi1) != 0)
		{
			status = 1;
			break;
		}

		/* Configure the command */
		if (HAL_XSPI_Command(&hospi1, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			status = 1;
			break;
		}

		/* Transmission of the data */
		if (HAL_XSPI_Transmit_DMA(&hospi1, (uint8_t*) data_buffer) != HAL_OK)
		{
			status = 1;
			break;
		}
		/* Check success of the transmission of the data */

		timeout_start = HAL_GetTick();
		while (HAL_GetTick() - timeout_start < LX_STM32_OSPI_DEFAULT_TIMEOUT)
		{
			if (ospi_tx_cplt == 1)
				break;
		}

		if (ospi_tx_cplt == 0)
		{
			status = 1;
			break;
		}
		else
		{

			if (lx_stm32_ospi_get_statusBlockingW25Q125(&hospi1,3) != 0)
			{
				ospi_tx_cplt = 0;
				status = 1;
				break;
			}
			/* Update the address and data variables for next page programming */
			current_addr += current_size;
			data_buffer += current_size;
			current_size = ((current_addr + LX_STM32_OSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : LX_STM32_OSPI_PAGE_SIZE;
		}
	} while (current_addr < end_addr);
	return status;

}
uint8_t lx_stm32_ospi_eraseW25Q125(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase)
{
	uint8_t status = 0;
	XSPI_RegularCmdTypeDef s_command = {0};
	uint32_t timeout;
	s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
	s_command.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
	if (full_chip_erase)
	{
		s_command.Instruction = LX_STM32_OSPI_OCTAL_BULK_ERASE_CMD;
		s_command.AddressMode = HAL_XSPI_ADDRESS_NONE;
		timeout = 200000;	//200 segundos
	}
	else
	{
		s_command.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
		s_command.Instruction = LX_STM32_OSPI_OCTAL_SECTOR_ERASE_CMD;
		s_command.Address = (block * LX_STM32_OSPI_SECTOR_SIZE);
		s_command.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
		timeout = 3000; //3 segundos
	}
	if (ospi_set_write_enableW25Q125(&hospi1) != 0)
	{
		status = 1;
	}
	if (HAL_XSPI_Command(&hospi1, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK && status == 0)
	{
		status = 1;
	}
	if (lx_stm32_ospi_get_statusBlockingW25Q125(&hospi1, timeout) != 0 && status == 0)
	{
		status = 1;
	}
	return status;
}

static uint8_t ospi_set_write_enableW25Q125(XSPI_HandleTypeDef *hxspi)
{
  uint8_t status = 0,reg;
  XSPI_RegularCmdTypeDef  s_command = {0};
  XSPI_AutoPollingTypeDef POLL = {0};
  s_command.Instruction           = LX_STM32_OSPI_OCTAL_WRITE_ENABLE_CMD;
  s_command.InstructionMode       = HAL_XSPI_INSTRUCTION_1_LINE;

  POLL.MatchMode = HAL_XSPI_MATCH_MODE_AND;
	POLL.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
	POLL.IntervalTime = 0x2FF;
	POLL.MatchValue = 0;
	POLL.MatchMask = 2;
  if (HAL_XSPI_Command(&hospi1, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
  	status = 1;
  }
	s_command.Instruction = LX_STM32_OSPI_OCTAL_READ_STATUS_REG_CMD;
	s_command.DataLength = 1;
	s_command.DataMode = HAL_XSPI_DATA_1_LINE;
	HAL_XSPI_Command(&hospi1, &s_command, 10);
	HAL_XSPI_Receive(&hospi1, &reg, 10);
	if (!(reg & LX_STM32_OSPI_SR_WEL))
	{
		status = 1;
	}
  return status;
}
