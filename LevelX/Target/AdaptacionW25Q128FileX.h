/*
 * AdaptacionW25Q128FileX.h
 *
 *  Created on: Oct 17, 2025
 *      Author: DesarrolloWeb
 */

#ifndef TARGET_ADAPTACIONW25Q128FILEX_H_
#define TARGET_ADAPTACIONW25Q128FILEX_H_
#include "fx_user.h"
#include "stm32h5xx_hal.h"
#include "lx_stm32_ospi_driver.h"

uint8_t lx_stm32_ospi_get_statusW25Q125(UINT instance);
uint8_t lx_stm32_ospi_lowlevel_initW25Q125(UINT instance);
uint8_t lx_stm32_ospi_eraseW25Q125(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase);
uint8_t lx_stm32_ospi_writeW25Q125(UINT instance, ULONG *address, ULONG *buffer, ULONG words);
uint8_t lx_stm32_ospi_readW25Q125(UINT instance, ULONG *address, ULONG *buffer, ULONG words);
uint8_t lx_stm32_ospi_is_block_erasedW25Q125(UINT instance, ULONG block);

#endif /* TARGET_ADAPTACIONW25Q128FILEX_H_ */
