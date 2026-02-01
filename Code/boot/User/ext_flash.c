/**
 * @file ext_flash.c
 * @brief Brief description of the source file
 * @author 404zen
 * @date 2026-01-31
 * @version 1.0
 */

#include "ext_flash.h"
#include "main.h"
#include "octospi.h"
#include "stm32h7xx_hal_def.h"
#include <math.h>
#include <stdint.h>
#include <sys/_intsup.h>
#include "WB_Command.h"
#include "stm32h7xx_hal_ospi.h"


static int8_t OSPI_W25Qxx_AutoPolling_MemReday(void);


/* Ensure Quad Enable (QE) in status register-2 is set for Quad I/O operations */
static int8_t OSPI_W25Qxx_EnableQuad(void)
{
    OSPI_RegularCmdTypeDef cmd;
    uint8_t sr2 = 0;
    uint8_t write_val = 0;

    /* Read Status Register-2 */
    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AddressMode         = HAL_OSPI_ADDRESS_NONE;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode            = HAL_OSPI_DATA_1_LINE;
    cmd.DataDtrMode         = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.NbData              = 1;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;
    cmd.Instruction         = WB_Flash_CMD_Read_Status_Register_2;

    if (HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_AUTO_POLLING_SEND_FAILED;
    }
    if (HAL_OSPI_Receive(&hospi1, &sr2, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_AUTO_POLLING_SEND_FAILED;
    }

    /* If QE already set, nothing to do */
    if (sr2 & WB_Flash_Status2_QE_Msk)
    {
        return EXT_FLASH_RET_OK;
    }

    /* Enable write and set QE bit in status register-2 */
    if (OSPI_W25Qxx_WriteEnable() != EXT_FLASH_RET_OK)
    {
        return EXT_FLASH_WRITE_ENABLE_FAILED;
    }

    write_val = sr2 | (uint8_t)WB_Flash_Status2_QE_Msk;

    cmd.Instruction = WB_Flash_CMD_Write_Status_Register_2;
    cmd.DataMode = HAL_OSPI_DATA_1_LINE;
    cmd.NbData = 1;

    if (HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_PAGE_WRITE_CMD_FAILED;
    }
    if (HAL_OSPI_Transmit(&hospi1, &write_val, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_PAGE_WRITE_DATA_FAILED;
    }

    return OSPI_W25Qxx_AutoPolling_MemReday();
}

int32_t OSPI_Get_FlashID(void)
{
    uint32_t id = 0;
    uint8_t data_buf[3] = {0};

    OSPI_RegularCmdTypeDef cmd;

    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AddressMode         = HAL_OSPI_ADDRESS_NONE;
    cmd.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode            = HAL_OSPI_DATA_1_LINE;
    cmd.DataDtrMode         = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.NbData              = 3;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;
    cmd.Instruction         = WB_Flash_CMD_Read_JEDEC_ID;

    HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);

    HAL_OSPI_Receive(&hospi1, data_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);

    id = (data_buf[0] << 16) | (data_buf[1] << 8) | data_buf[2];

    if(id == W25Qxx_FLASH_ID)
    {
        return (int32_t)id;
    }
    else 
    {
        return EXT_FLASH_GET_JEDEC_ID_FAILED;
    }
}

int8_t OSPI_ExtFlash_Mapped(void)
{
    OSPI_RegularCmdTypeDef cmd;
    OSPI_MemoryMappedTypeDef map_cfg;

    cmd.OperationType           = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId                 = HAL_OSPI_FLASH_ID_1;

    cmd.Instruction             = WB_Flash_CMD_Fast_Read_Quad_IO;
    cmd.InstructionMode         = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize         = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode      = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

    cmd.AddressMode             = HAL_OSPI_ADDRESS_4_LINES;
    cmd.AddressSize             = HAL_OSPI_ADDRESS_24_BITS;
    cmd.AddressDtrMode          = HAL_OSPI_ADDRESS_DTR_DISABLE;

    cmd.AlternateBytesMode      = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesDtrMode   = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;

    cmd.DataMode                = HAL_OSPI_DATA_4_LINES;
    cmd.DataDtrMode             = HAL_OSPI_DATA_DTR_DISABLE;

    cmd.DummyCycles             = 6;
    cmd.DQSMode                 = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode                = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_MAPPED_FAILED;
    }

    map_cfg.TimeOutActivation   = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
    map_cfg.TimeOutPeriod       = 0;
    
    if (HAL_OK == HAL_OSPI_MemoryMapped(&hospi1, &map_cfg)) 
    {
        return EXT_FLASH_RET_OK;
    }
    else 
    {
        return EXT_FLASH_MAPPED_FAILED;
    }
}

int8_t  OSPI_W25Qxx_WriteEnable(void)
{
    OSPI_RegularCmdTypeDef cmd;
    OSPI_AutoPollingTypeDef polling_cfg;

    cmd.OperationType           = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId                 = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode         = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize         = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode      = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.Address                 = 0;
    cmd.AddressMode             = HAL_OSPI_ADDRESS_NONE;
    cmd.AddressSize             = HAL_OSPI_ADDRESS_24_BITS;
    cmd.AlternateBytesDtrMode   = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;
    cmd.AlternateBytesMode      = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode                = HAL_OSPI_DATA_NONE;
    cmd.DataDtrMode             = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.DummyCycles             = 0;
    cmd.DQSMode                 = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode                = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
    cmd.Instruction             = WB_Flash_CMD_Write_Enable;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_WRITE_ENABLE_FAILED;
    }

    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AddressMode         = HAL_OSPI_ADDRESS_NONE;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;
    cmd.DataMode            = HAL_OSPI_DATA_1_LINE;
    cmd.NbData              = 1;

    cmd.Instruction         = WB_Flash_CMD_Read_Status_Register_1;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_WRITE_ENABLE_FAILED;
    }

    polling_cfg.Match           = 0x02;
    polling_cfg.MatchMode       = HAL_OSPI_MATCH_MODE_AND;
    polling_cfg.Interval        = 0x10;
    polling_cfg.AutomaticStop   = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
    polling_cfg.Mask            = WB_Flash_Status1_WEL_Msk;

    if (HAL_OSPI_AutoPolling(&hospi1, &polling_cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_WRITE_ENABLE_FAILED;
    }

    return EXT_FLASH_RET_OK;
}

/**
 * @brief  W25Qxx Sector Erase, 4K per sector
 * @param  address Sector address
 * @retval HAL status or other return value description
 */

int8_t  OSPI_W25Qxx_SectorErase(uint32_t address)
{
    int8_t ret = EXT_FLASH_RET_OK;
    OSPI_RegularCmdTypeDef cmd;

    ret = OSPI_W25Qxx_WriteEnable();
    if(ret != EXT_FLASH_RET_OK)
    {
        return ret;
    }

    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.Address             = address;
    cmd.AddressMode         = HAL_OSPI_ADDRESS_1_LINE;
    cmd.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode            = HAL_OSPI_DATA_NONE;
    cmd.DataDtrMode         = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction         = WB_Flash_CMD_Sector_Erase;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_ERASE_CMD_FAILED;
    }

    return OSPI_W25Qxx_AutoPolling_MemReday();
}

/**
 * @brief  int8_t  OSPI_W25Qxx_BlockErase
 * @param  erase_cmd can be Sector/4K(0x20); Half Block 32KB(0x52); Block 64KB(0xD8); Full Chip(0xC7)
 * @retval HAL status or other return value description
 */

int8_t  OSPI_W25Qxx_Erase(uint32_t address, uint32_t erase_cmd)
{
    int8_t ret = EXT_FLASH_RET_OK;
    OSPI_RegularCmdTypeDef cmd;

    ret = OSPI_W25Qxx_WriteEnable();
    if(ret != EXT_FLASH_RET_OK)
    {
        return ret;
    }

    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

    if(erase_cmd == WB_Flash_CMD_Chip_Erase)
    {
        cmd.AddressMode         = HAL_OSPI_ADDRESS_NONE;
        cmd.Address             = 0;
        cmd.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
    }
    else 
    {
        cmd.Address             = address;
        cmd.AddressMode         = HAL_OSPI_ADDRESS_1_LINE;
        cmd.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
    }
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode            = HAL_OSPI_DATA_NONE;
    cmd.DataDtrMode         = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction         = erase_cmd;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_ERASE_CMD_FAILED;
    }

    return OSPI_W25Qxx_AutoPolling_MemReday();
}

/**
 * @brief  OSPI_W25Qxx_WritePage
 * @param  dest_addr: destination address to write
 * @param  num_bytes: number of bytes to write, max 256 bytes per page
 * @param  p_data: pointer to data buffer to write
 * @retval HAL status or other return value description
 */

int8_t  OSPI_W25Qxx_WritePage(uint32_t dest_addr, uint32_t num_bytes, uint8_t *p_data)
{
    int8_t ret = EXT_FLASH_RET_OK;
    OSPI_RegularCmdTypeDef cmd;

    if(num_bytes > W25Qxx_PAGE_SIZE)
    {
        return EXT_FLASH_PAGE_WRITE_LEN_ERROR;
    }

    ret = OSPI_W25Qxx_WriteEnable();
    if(ret != EXT_FLASH_RET_OK)
    {
        return ret;
    }

    cmd.OperationType           = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId                 = HAL_OSPI_FLASH_ID_1;

    cmd.InstructionMode         = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize         = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode      = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    
    cmd.Address                 = dest_addr;
    cmd.AddressMode             = HAL_OSPI_ADDRESS_1_LINE;
    cmd.AddressSize             = HAL_OSPI_ADDRESS_24_BITS;
    cmd.AddressDtrMode          = HAL_OSPI_ADDRESS_DTR_DISABLE;

    cmd.AlternateBytesMode      = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesDtrMode   = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;
    
    cmd.DataMode                = HAL_OSPI_DATA_4_LINES;
    cmd.DataDtrMode             = HAL_OSPI_DATA_DTR_DISABLE;
    cmd.NbData                  = num_bytes;
    
    cmd.DummyCycles             = 0;
    cmd.DQSMode                 = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode                = HAL_OSPI_SIOO_INST_EVERY_CMD;
    
    cmd.Instruction             = WB_Flash_CMD_Quad_Input_Page_Program;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_PAGE_WRITE_CMD_FAILED;
    }

    if(HAL_OSPI_Transmit(&hospi1, p_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_PAGE_WRITE_DATA_FAILED;
    }

    return OSPI_W25Qxx_AutoPolling_MemReday();
}

/**
 * @brief  Write data to dest_addr with num_bytes length
 * @param  dest_addr: destination address to write
 * @param  num_bytes: number of bytes to write
 * @param  p_data: pointer to data buffer to write
 * @retval HAL status or other return value description
 */

int8_t  OSPI_W25Qxx_WriteBuffer(uint32_t dest_addr, uint32_t num_bytes, uint8_t *p_data)
{
    int8_t ret = EXT_FLASH_RET_OK;
    uint32_t end_addr = 0;
    uint32_t current_size = 0;
    uint32_t current_addr = 0;
    uint8_t *p_write_data = 0;

    // calculate current size need to write
    current_size = W25Qxx_PAGE_SIZE - (dest_addr % W25Qxx_PAGE_SIZE);
    if(current_size > num_bytes)
    {
        current_size = num_bytes;
    }

    current_addr = dest_addr;
    end_addr = dest_addr + num_bytes;           // end address
    p_write_data = p_data;

    do
    {
        ret = OSPI_W25Qxx_WritePage(current_addr, current_size, p_write_data);
        if(ret != EXT_FLASH_RET_OK)
        {
            return ret;
        }
        else 
        {
            current_addr += current_size;
            p_write_data += current_size;

            current_size = ((current_addr + W25Qxx_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Qxx_PAGE_SIZE;
        }
    }while(current_addr < end_addr);

    return ret;
}

int8_t  OSPI_W25Qxx_ReadBuffer(uint32_t src_addr, uint32_t num_bytes, uint8_t *p_data)
{
    // SFL_DEBUG("Call %s\r\n", __FUNCTION__);
    // SFL_DEBUG("src_addr: 0x%08X, num_bytes: %d\r\n", src_addr, num_bytes);
    
    OSPI_RegularCmdTypeDef  cmd;

   cmd.OperationType           = HAL_OSPI_OPTYPE_COMMON_CFG;         
   cmd.FlashId                 = HAL_OSPI_FLASH_ID_1;                

   cmd.Instruction             = WB_Flash_CMD_Fast_Read_Quad_IO;      
   cmd.InstructionMode         = HAL_OSPI_INSTRUCTION_1_LINE;         
   cmd.InstructionSize         = HAL_OSPI_INSTRUCTION_8_BITS;         
   cmd.InstructionDtrMode      = HAL_OSPI_INSTRUCTION_DTR_DISABLE;  

   cmd.Address                 = src_addr;                      
   cmd.AddressMode             = HAL_OSPI_ADDRESS_4_LINES;       
   cmd.AddressSize             = HAL_OSPI_ADDRESS_24_BITS;         
   cmd.AddressDtrMode          = HAL_OSPI_ADDRESS_DTR_DISABLE;          

   cmd.AlternateBytesMode      = HAL_OSPI_ALTERNATE_BYTES_NONE;
   cmd.AlternateBytesDtrMode   = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;

   cmd.DataMode                = HAL_OSPI_DATA_4_LINES;
   cmd.DataDtrMode             = HAL_OSPI_DATA_DTR_DISABLE;
   cmd.NbData                  = num_bytes;

   cmd.DummyCycles             = 6;
   cmd.DQSMode                 = HAL_OSPI_DQS_DISABLE;
   cmd.SIOOMode                = HAL_OSPI_SIOO_INST_EVERY_CMD;  


	if (HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return EXT_FLASH_PAGE_READ_CMD_FAILED;		
	}   

	if (HAL_OSPI_Receive(&hospi1, p_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return EXT_FLASH_PAGE_READ_DATA_FAILED;	
	}

	return OSPI_W25Qxx_AutoPolling_MemReday();
}


uint8_t write_buf[1024];


void OSPI_W25Qxx_Test(void)
{
    int8_t ret = 0;
    uint8_t sector_erase_test = 1;
    uint8_t block_erase_test = 0;
    uint8_t page_write_test = 0;
    uint8_t write_test = 0;
    uint32_t address = 0;

    uint32_t i = 0;
    
    for (i = 0; i < 1024; i++) 
    {
        write_buf[i] = i;
    }

    if(sector_erase_test)
    {
        ret = OSPI_W25Qxx_SectorErase(address);
    }

    if(block_erase_test)
    {
        ret = OSPI_W25Qxx_Erase(address, WB_Flash_CMD_Chip_Erase);

    }

    if(page_write_test)
    {
        ret = OSPI_W25Qxx_Erase(address, WB_Flash_CMD_Block_Erase);
        page_write_test--;
        if(ret == EXT_FLASH_RET_OK && page_write_test == 1)
        {
            ret = OSPI_W25Qxx_WritePage(address, 256, write_buf);
        }
    }

    if(write_test)
    {
        ret = OSPI_W25Qxx_Erase(address, WB_Flash_CMD_Chip_Erase);
        if(ret == EXT_FLASH_RET_OK)
        {
            ret = OSPI_W25Qxx_WriteBuffer(address, 1024, write_buf);
        }
    }

    /* Check */
    ret = OSPI_ExtFlash_Mapped();
}

static int8_t OSPI_W25Qxx_AutoPolling_MemReday(void)
{
    OSPI_RegularCmdTypeDef cmd;
    OSPI_AutoPollingTypeDef polling_cfg;

    cmd.OperationType       = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId             = HAL_OSPI_FLASH_ID_1;
    cmd.InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd.InstructionSize     = HAL_OSPI_INSTRUCTION_8_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.Address             = 0;
    cmd.AddressMode         = HAL_OSPI_ADDRESS_NONE;
    cmd.AddressSize         = HAL_OSPI_ADDRESS_24_BITS;
    cmd.InstructionDtrMode  = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    cmd.AlternateBytesMode  = HAL_OSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode            = HAL_OSPI_DATA_1_LINE;
    cmd.DataDtrMode         = HAL_OSPI_DATA_DTR_DISABLE;
    // cmd.NbData              = 0;
    cmd.NbData              = 1;
    cmd.DummyCycles         = 0;
    cmd.DQSMode             = HAL_OSPI_DQS_DISABLE;
    cmd.SIOOMode            = HAL_OSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction         = WB_Flash_CMD_Read_Status_Register_1;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return EXT_FLASH_AUTO_POLLING_SEND_FAILED;
    }

    polling_cfg.Match           = 0;
    polling_cfg.MatchMode       = HAL_OSPI_MATCH_MODE_AND;
    polling_cfg.Interval        = 0x10;
    polling_cfg.AutomaticStop   = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
    polling_cfg.Mask            = WB_Flash_Status1_BUSY_Msk;

    if (HAL_OSPI_AutoPolling(&hospi1, &polling_cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE*20) != HAL_OK)
    {
        return EXT_FLASH_AUTO_POLLING_TIMEOUT;
    }

    return EXT_FLASH_RET_OK;
}
