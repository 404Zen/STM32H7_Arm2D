/**
 * @file ext_flash.h
 * @brief Brief description of the header file
 * @author 404zen
 * @date 2026-01-31
 * @version 1.0
 */

#ifndef __EXT_FLASH__
#define __EXT_FLASH__


#include <stdint.h>
#include "octospi.h"
#include "stm32h7xx_hal_ospi.h"



#define W25Qxx_FLASH_ID                 0xEF4017            // Windbond W25Q64JV JEDEC ID
// #define W25Qxx_FLASH_ID_SWAP            0x1740EF            // ...

// #define W25Qxx_CMD_JedecID              0x9F                // JEDEC ID 
// #define W25Qxx_CMD_FastReadQuad_IO      0xEB                // Fast Read Quad I/O

/* Return Value */
/* >=0 OK; < 0 Error */
#define EXT_FLASH_ERROR_UNKNOW                      -100
#define EXT_FLASH_PAGE_READ_DATA_FAILED             -11
#define EXT_FLASH_PAGE_READ_CMD_FAILED              -10
#define EXT_FLASH_PAGE_WRITE_LEN_ERROR              -9
#define EXT_FLASH_PAGE_WRITE_DATA_FAILED            -8
#define EXT_FLASH_PAGE_WRITE_CMD_FAILED             -7
#define EXT_FLASH_AUTO_POLLING_TIMEOUT              -6
#define EXT_FLASH_AUTO_POLLING_SEND_FAILED          -5
#define EXT_FLASH_ERASE_CMD_FAILED                  -4
#define EXT_FLASH_WRITE_ENABLE_FAILED               -3
#define EXT_FLASH_MAPPED_FAILED                     -2
#define EXT_FLASH_GET_JEDEC_ID_FAILED               -1
#define EXT_FLASH_RET_OK                            0

#define W25Qxx_PAGE_SIZE                            256                 // 256 bytes per page

#define SECTOR_SIZE                                 0x1000
#define HALF_BLOCK_SIZE                             0x8000
#define BLOCK_SIZE                                  0x10000
#define SECTOR_NUM_OF_HALF_BLOCK                    (HALF_BLOCK_SIZE / SECTOR_SIZE)
#define SECTOR_NUM_OF_BLOCK                         (BLOCK_SIZE / SECTOR_SIZE)

#define W25QXX_STATUS_REG1_BUSY_POS                 0
#define W25QXX_STATUS_REG1_BUSY_MASK                (0x01 << W25QXX_STATUS_REG1_BUSY_POS)

int32_t OSPI_Get_FlashID(void);
int8_t OSPI_ExtFlash_Mapped(void);

int8_t  OSPI_W25Qxx_WriteEnable(void);
int8_t  OSPI_W25Qxx_SectorErase(uint32_t address);
int8_t  OSPI_W25Qxx_Erase(uint32_t address, uint32_t erase_cmd);
int8_t  OSPI_W25Qxx_WritePage(uint32_t dest_addr, uint32_t num_bytes, uint8_t *p_data);
int8_t  OSPI_W25Qxx_WriteBuffer(uint32_t dest_addr, uint32_t num_bytes, uint8_t *p_data);

int8_t  OSPI_W25Qxx_ReadBuffer(uint32_t src_addr, uint32_t num_bytes, uint8_t *p_data);

void OSPI_W25Qxx_Test(void);

#endif