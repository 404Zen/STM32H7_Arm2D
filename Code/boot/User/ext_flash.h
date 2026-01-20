#ifndef __EXT_FLASH__
#define __EXT_FLASH__


#include <stdint.h>



#define W25Qxx_FLASH_ID                 0xEF4017            // Windbond W25Q64JV JEDEC ID
#define W25Qxx_FLASH_ID_SWAP            0x1740EF            // ...

#define W25Qxx_CMD_JedecID              0x9F                // JEDEC ID 
#define W25Qxx_CMD_FastReadQuad_IO      0xEB                // Fast Read Quad I/O


uint32_t OSPI_Get_FlashID(void);
uint32_t OSPI_ExtFlash_Mapped(void);

#endif