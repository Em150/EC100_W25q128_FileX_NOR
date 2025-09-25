/*
 * AdaptacionW25Q128FileX.c
 *
 *  Created on: Sep 24, 2025
 *      Author: Emilio
 */

#include "lx_stm32_ospi_driver.h"

extern XSPI_HandleTypeDef hospi1;
#include "stm32h5xx_hal.h"

INT lx_stm32_ospi_get_statusW25Q125(UINT instance);


INT lx_stm32_ospi_get_statusW25Q125(UINT instance)
{
	XSPI_RegularCmdTypeDef s_command ={ 0 };
	uint8_t reg;
	s_command.Instruction = LX_STM32_OSPI_OCTAL_READ_STATUS_REG_CMD;
	s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
	s_command.DataLength = 1;
	s_command.DataMode = HAL_XSPI_DATA_1_LINE;
	HAL_XSPI_Command(&hospi1, &s_command, 10);
	HAL_XSPI_Receive(&hospi1, &reg, 10);
	if ((reg & LX_STM32_OSPI_SR_WIP) != 0)
	{
		return 1;
	}
	return 0;
}
