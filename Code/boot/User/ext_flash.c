
#include "ext_flash.h"
#include "main.h"
#include "octospi.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_ospi.h"
#include <stdint.h>

uint32_t OSPI_Get_FlashID(void)
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
    cmd.Instruction         = W25Qxx_CMD_JedecID;

    HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);

    HAL_OSPI_Receive(&hospi1, data_buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);

    // id = *(uint32_t*)data_buf;
    id = (data_buf[0] << 16) | (data_buf[1] << 8) | data_buf[2];

    if(id == W25Qxx_FLASH_ID)
    {
        return id;
    }
    else 
    {
        return 0;
    }
}

uint32_t OSPI_ExtFlash_Mapped(void)
{
    OSPI_RegularCmdTypeDef cmd;
    OSPI_MemoryMappedTypeDef map_cfg;

    cmd.OperationType           = HAL_OSPI_OPTYPE_COMMON_CFG;
    cmd.FlashId                 = HAL_OSPI_FLASH_ID_1;

    cmd.Instruction             = W25Qxx_CMD_FastReadQuad_IO;
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
        return 1;
    }

    map_cfg.TimeOutActivation   = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
    map_cfg.TimeOutPeriod       = 0;
    
    if (HAL_OK == HAL_OSPI_MemoryMapped(&hospi1, &map_cfg)) 
    {
        return 0;
    }
    else 
    {
        return 1;
    }
}