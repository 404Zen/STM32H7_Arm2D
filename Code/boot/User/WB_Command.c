/*
	COPYRIGHT 2024 Winbond Electronics Corp.
	SPI FLASH Reference Code
	Version  V1.00

	The functions in this file describe the SPI input/output combinations for each command in the W25Q33_64_12_25_51_01_02RV-DTR.
	Each function is associated with the command as shown in Chapters 6.1 and 6.2 of the W25Q33_64_12_25_51_01_02RV-DTR datasheet.
	For example, the comment Command# "10" refers to Chapter 6.2."10" of the datasheet.

	The 'N/A' command is associated with the function that handles SPI control signals
	 or signal combinations for command testing.

*/

#include "WB_Command.h"
#include "main.h"

static unsigned char WB_SPINOR_SPIin(unsigned int data_input, unsigned int spi_mode);
static void WB_CS_Low();
static void WB_CS_High();
static void WB_Wait_Ready();
static void WB_Write_Enable();
static void WB_Write_Disable();

/*
Command#:			N/A
Function:           WB_SPINOR_SPIin(data_input, spi_mode)
Description:        MOSI input to Flash, MISO output from Flash.
Arguments:          data_input: MOSI input to Flash
                    spi_mode: single, dual, quad, octal,
                              single dtr, dual dtr, quad dtr,
                              octal dtr
Return:				MISO output from Flash or execution result (TRUE/FALSE)
Comment:			Please implement the low level driver
                    for SPI bus follow the controller
*/
static unsigned char WB_SPINOR_SPIin(unsigned int data_input, unsigned int spi_mode)
{
    unsigned char u8RxData = 0;
    unsigned int u32SpiParam = 0, u32SpiModel = 0;

    // Implement the SPI TX/RX function follow the hardware

    switch (spi_mode & WB_Flash_SPI_Mode_Msk)
    {
    case SIO:
        break;

    case DIO:
        if (!QSPI_IS_DUAL_ENABLED(QSPI_FLASH_PORT))
        {
            // Enable dual IO mode for first time
            if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
            {
                QSPI_ENABLE_DUAL_INPUT_MODE(QSPI_FLASH_PORT); // Enable SPI dual IO mode and set direction to input
            }
            else
            {
                QSPI_ENABLE_DUAL_OUTPUT_MODE(QSPI_FLASH_PORT); // Enable SPI dual IO mode and set direction to output
            }
        }
        else
        {
            // Dual IO mode has been enabled, determine whether to switch direction
            if (QSPI_IS_DIR_INPUT_MODE(QSPI_FLASH_PORT))
            {
                if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_OUTPUT)
                {
                    QSPI_ENABLE_DUAL_OUTPUT_MODE(QSPI_FLASH_PORT); // input -> output
                }
            }
            else
            {
                if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
                {
                    QSPI_ENABLE_DUAL_INPUT_MODE(QSPI_FLASH_PORT); // output -> input
                }
            }
        }
        break;

    case QIO:
        if (!QSPI_IS_QUAD_ENABLED(QSPI_FLASH_PORT))
        {
            // Enable quad IO mode for first time
            if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
            {
                QSPI_ENABLE_QUAD_INPUT_MODE(QSPI_FLASH_PORT); // Enable SPI quad IO mode and set direction to input
            }
            else
            {
                QSPI_ENABLE_QUAD_OUTPUT_MODE(QSPI_FLASH_PORT); // Enable SPI quad IO mode and set direction to output
            }
        }
        else
        {
            // Quad IO mode has been enabled, determine whether to switch direction
            if (QSPI_IS_DIR_INPUT_MODE(QSPI_FLASH_PORT))
            {
                if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_OUTPUT)
                {
                    QSPI_ENABLE_QUAD_OUTPUT_MODE(QSPI_FLASH_PORT); // input -> output
                }
            }
            else
            {
                if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
                {
                    QSPI_ENABLE_QUAD_INPUT_MODE(QSPI_FLASH_PORT); // output -> input
                }
            }
        }
        break;

    case OIO:
        break;

    case DTSIO:
        if (!QSPI_IS_DTR_ENABLED(QSPI_FLASH_PORT))
        {
            QSPI_ENABLE_DTR_MODE(QSPI_FLASH_PORT); // Enable DTR mode
        }
        break;

    case DTDIO:
        if (!QSPI_IS_DTR_ENABLED(QSPI_FLASH_PORT))
        {
            QSPI_ENABLE_DTR_MODE(QSPI_FLASH_PORT); // Enable DTR mode
        }

        if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
        {
            if (!QSPI_IS_DIR_INPUT_MODE(QSPI_FLASH_PORT))
            {
                QSPI_ENABLE_DUAL_INPUT_MODE(QSPI_FLASH_PORT); // Enable SPI dual IO mode and set direction to input
            }
        }
        else
        {
            if (!QSPI_IS_DIR_OUTPUT_MODE(QSPI_FLASH_PORT))
            {
                QSPI_ENABLE_DUAL_OUTPUT_MODE(QSPI_FLASH_PORT); // Enable SPI dual IO mode and set direction to output
            }
        }
        break;

    case DTQIO:
        if (!QSPI_IS_DTR_ENABLED(QSPI_FLASH_PORT))
        {
            QSPI_ENABLE_DTR_MODE(QSPI_FLASH_PORT); // Enable DTR mode
        }

        if ((spi_mode & WB_Flash_SPI_DIR_Msk) == SPI_DIR_INPUT)
        {
            if (!QSPI_IS_DIR_INPUT_MODE(QSPI_FLASH_PORT))
            {
                QSPI_ENABLE_QUAD_INPUT_MODE(QSPI_FLASH_PORT); // Enable SPI quad IO mode and set direction to input
            }
        }
        else
        {
            if (!QSPI_IS_DIR_OUTPUT_MODE(QSPI_FLASH_PORT))
            {
                QSPI_ENABLE_QUAD_OUTPUT_MODE(QSPI_FLASH_PORT); // Enable SPI quad IO mode and set direction to output
            }
        }
        break;

    case DTOIO:
        break;
    }

    // Processing SPI operating parameters & Dummy model
    u32SpiParam = spi_mode & WB_Flash_SPI_PARAM_Msk;
    u32SpiModel = spi_mode & WB_Flash_SPI_DUMMY_Model_Msk;

    if (u32SpiParam & SPI_PARAM_RETURN_TXRESULT_Msk)
    {
        // Continuous SPI TX case, write data to SPI TX register when TX FIFO is not full
        if (!QSPI_GET_TX_FIFO_FULL_FLAG(QSPI_FLASH_PORT))
        {
            QSPI_WRITE_TX(QSPI_FLASH_PORT, data_input);
            /*return TRUE;*/
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if (u32SpiModel == 0)
        {
            // Normal SPI TX case, write data to SPI TX
            QSPI_WRITE_TX(QSPI_FLASH_PORT, data_input);
        }
        else
        {
            // Special SPI TX case, adjust data width to match required dummy clock
            switch (u32SpiModel)
            {
            case SPI_DUMMY_MODEL_1_Msk:
                // set data width as 12 bits for 6 dummy clocks
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 12);
                QSPI_WRITE_TX(QSPI_FLASH_PORT, 0xFFF);
                while (QSPI_IS_BUSY(QSPI_FLASH_PORT))
                    ;
                QSPI_ClearRxFIFO(QSPI_FLASH_PORT);
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 8);
                break;

            case SPI_DUMMY_MODEL_2_Msk:
                // set data width as 16 bits for 4 dummy clocks
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 16);
                QSPI_WRITE_TX(QSPI_FLASH_PORT, 0xFFFF);
                while (QSPI_IS_BUSY(QSPI_FLASH_PORT))
                    ;
                QSPI_ClearRxFIFO(QSPI_FLASH_PORT);
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 8);
                break;

            case SPI_DUMMY_MODEL_3_Msk:
                // set data width as 28 bits for 7 dummy clocks (7*2*4 = 56 bits)
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 28);
                WB_SPINOR_SPIin(0x0FFFFFFF, SIO);
                WB_SPINOR_SPIin(0x0FFFFFFF, SIO);
                while (QSPI_IS_BUSY(QSPI_FLASH_PORT))
                    ;
                QSPI_ClearRxFIFO(QSPI_FLASH_PORT);
                QSPI_SET_DATA_WIDTH(QSPI_FLASH_PORT, 8);
                break;
            }
            return TRUE;
        }
    }

    if (u32SpiParam != 0)
    {
        if (u32SpiParam & SPI_PARAM_CHECK_BUSY_Msk)
        {
            while (QSPI_IS_BUSY(QSPI_FLASH_PORT))
                ; // Wait until SPI TX done

            if (u32SpiParam & SPI_PARAM_CLEAR_RXFIFO_Msk)
            {
                QSPI_ClearRxFIFO(QSPI_FLASH_PORT); // Clear RX FIFO buffer

                if (u32SpiParam & SPI_PARAM_DISABLE_FUNCTION_Msk)
                {
                    QSPI_DISABLE_DTR_MODE(QSPI_FLASH_PORT);
                    QSPI_DISABLE_DUAL_MODE(QSPI_FLASH_PORT);
                    QSPI_DISABLE_QUAD_MODE(QSPI_FLASH_PORT);
                }
                return TRUE;
            }
            else if (u32SpiParam & SPI_PARAM_READ_RXFIFO_Msk)
            {
                u8RxData = QSPI_READ_RX(QSPI_FLASH_PORT); // Read data from RX register

                if (u32SpiParam & SPI_PARAM_DISABLE_FUNCTION_Msk)
                {
                    QSPI_DISABLE_DTR_MODE(QSPI_FLASH_PORT);
                    QSPI_DISABLE_DUAL_MODE(QSPI_FLASH_PORT);
                    QSPI_DISABLE_QUAD_MODE(QSPI_FLASH_PORT);
                }
            }
            else
            {
                return TRUE;
            }
        }
        else
        {
            return TRUE; // for SPI_PARAM_RETURN_TXRESULT_Msk, continuous SPI TX case
        }
    }

    return u8RxData;
}

/*
Command#:			N/A
Function:           WB_CS_Low()
Description:        Enable the CS
Arguments:
Return:
Comment:			Please implement this function follow the controller
*/
static void WB_CS_Low()
{
    //	Implement the CS low follow the hardware
    // QSPI_SET_SS_LOW(QSPI_FLASH_PORT);
    return;
}

/*
Command#:			N/A
Function:           WB_CS_High()
Description:        Disable the CS
Arguments:
Return:
Comment:			Please implement this function follow the controller
*/
static void WB_CS_High()
{
    //	Implement the CS high follow the hardware
    // QSPI_SET_SS_HIGH(QSPI_FLASH_PORT);           // H7B0 hardware CS
    return;
}

/*
Command#:			N/A
Function:           WB_Wait_Ready()
Description:        Wait until Flash ready
Arguments:
Return:
Comment:			This command use to check ready after data write
                    Set the timeout value follow the controller
*/
static void WB_Wait_Ready()
{
    unsigned char read_data;
    unsigned int time_out = WB_Flash_PARAM_Timeout;
    while (time_out)
    {
        read_data = WB_Read_Status_Register_1();
        if ((read_data & WB_Flash_MASK_WIP) == 0)
        {
            break;
        }
        // time_out--;		// Please enable the timeout carefully
    }
    return;
}

/*
Command#:			1
Function:           WB_Write_Enable()
Description:        Set WEL for Flash data write
Arguments:
Return:
Comment:
*/
static void WB_Write_Enable()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Enable, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    WB_CS_High();
    return;
}

/*
Command#:			2
Function:           WB_Write_Enable_For_Volatile_SR()
Description:        Set WEL for Flash volatile data write
Arguments:
Return:
Comment:
*/
void WB_Write_Enable_For_Volatile_SR()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Enable_Volatile_SR, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    WB_CS_High();
    return;
}

/*
Command#:			3
Function:           WB_Write_Disable()
Description:        Reset WEL
Arguments:
Return:
Comment:
*/
static void WB_Write_Disable()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Disable, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);
    WB_CS_High();
    return;
}

/*
Command#:			4.1
Function:           WB_Read_Status_Register_1()
Description:        Read status register-1
Arguments:
Return:				Status register-1 value
Comment:
*/
unsigned char WB_Read_Status_Register_1()
{
    unsigned char read_data;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Status_Register_1, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    read_data = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return read_data;
}

/*
Command#:			4.2
Function:           WB_Read_Status_Register_2()
Description:        Read status register-2
Arguments:
Return:				Status register-2 value
Comment:
*/
unsigned char WB_Read_Status_Register_2()
{
    unsigned char read_data;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Status_Register_2, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    read_data = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return read_data;
}



/*
Command#:			4.3
Function:           WB_Read_Status_Register_3()
Description:        Read status register-3
Arguments:
Return:				Status register-3 value
Comment:
*/
unsigned char WB_Read_Status_Register_3()
{
    unsigned char read_data;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Status_Register_3, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    read_data = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return read_data;
}

/*
Command#:			5.1
Function:           WB_Write_Status_Register_1()
Description:        Write status register-1
Arguments:          sr_data_write: data write to the status register
Return:
Comment:
*/
void WB_Write_Status_Register_1(unsigned char sr_data_write)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Status_Register_1, SIO);
    WB_SPINOR_SPIin(sr_data_write, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			5.2
Function:           WB_Write_Status_Register_2()
Description:        Write status register-2
Arguments:          sr_data_write: data write to the status register
Return:
Comment:
*/
void WB_Write_Status_Register_2(unsigned char sr_data_write)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Status_Register_2, SIO);
    WB_SPINOR_SPIin(sr_data_write, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			5.3
Function:           WB_Write_Status_Register_3()
Description:        Write status register-3
Arguments:          sr_data_write: data write to the status register
Return:
Comment:
*/
void WB_Write_Status_Register_3(unsigned char sr_data_write)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Status_Register_3, SIO);
    WB_SPINOR_SPIin(sr_data_write, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:			6
Function:           WB_Read_Extend_Address_Register()
Description:        Read extend address register for address over 128Mbit
Arguments:
Return:				Status register-3 value
Comment:
*/
unsigned char WB_Read_Extend_Address_Register()
{
    unsigned char read_data;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Extended_Status_Register, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    read_data = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return read_data;
}
/*
Command#:			7
Function:           WB_Write_Extend_Address_Register()
Description:        Write extend address register for address over 128Mbit
Arguments:          exa_data_write: data write to the extend address register
Return:
Comment:
*/
void WB_Write_Extend_Address_Register(unsigned char exa_data_write)
{
    WB_Write_Enable_For_Volatile_SR();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Extended_Status_Register, SIO);
    WB_SPINOR_SPIin(exa_data_write, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			8
Function:           WB_Enter_4Byte_Address_Mode()
Description:        Enter 4-Byte address mode
Arguments:
Return:				Status register-3 ADS bit result. (TRUE/FALSE)
Comment:
*/
unsigned char WB_Enter_4Byte_Address_Mode()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enter_4_Byte_Mode, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();

#if (CHECK_4BYTE_ADDRESS_MODE == 1)
    if ((WB_Read_Status_Register_3() & 0x1) == 0)
    	return FALSE;
#endif
    return TRUE;
}

/*
Command#:			9
Function:           WB_Exit_4Byte_Address_Mode()
Description:        Exit 4-Byte address mode
Arguments:
Return:				Status register-3 ADS bit result. (TRUE/FALSE)
Comment:
*/
unsigned char WB_Exit_4Byte_Address_Mode()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Exit_4_Byte_Mode, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();

#if (CHECK_4BYTE_ADDRESS_MODE == 1)
    if ((WB_Read_Status_Register_3() & 0x1) == 0x1)
    	return FALSE;
#endif
    return TRUE;
}

/*
Command#:			10
Function:           WB_Read_Data()
Description:        Normal read function
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:			AKA normal read
*/
unsigned int WB_Read_Data(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Data, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			11
Function:           WB_Read_Data_4Byte_Address()
Description:        Normal read function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Read_Data_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Data_4Byte_Address, SIO);
    WB_SPINOR_SPIin(read_address >> 24, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			12
Function:           WB_Fast_Read()
Description:        Fast read function
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			13
Function:           WB_DTR_Fast_Read()
Description:        DTR fast read function with SPI
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_DTR_Fast_Read(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_DTR_Fast_Read, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16, DTSIO); // enable DTR mode
    WB_SPINOR_SPIin(read_address >> 8, DTSIO);
    WB_SPINOR_SPIin(read_address, DTSIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(WB_Flash_Dummy, DTSIO | SPI_DUMMY_MODEL_1_Msk); // 6 dummy clocks

    while (read_count)
    {
        if (read_count > 1)
        {
            *(read_buf + index) =
                WB_SPINOR_SPIin(WB_Flash_Dummy, DTSIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable DTR mode
            *(read_buf + index) =
                WB_SPINOR_SPIin(WB_Flash_Dummy, DTSIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			14
Function:           WB_Fast_Read_4Byte_Address()
Description:        Fast read function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_4Byte_Address, SIO);
    WB_SPINOR_SPIin(read_address >> 24, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}




/*
Command#:			15
Function:           WB_Fast_Read_Dual_Output()
Description:        Fast read dual output function
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:            AKA fast read
*/
unsigned int WB_Fast_Read_Dual_Output(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Dual_Output, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // enable SPI dual IO mode and set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			16
Function:           WB_Fast_Read_Dual_Output_4Byte_Addrerss()
Description:        Fast read dual output function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:            AKA fast read
*/
unsigned int WB_Fast_Read_Dual_Output_4Byte_Addrerss(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Dual_Output_4Byte_Address, SIO);
    WB_SPINOR_SPIin(read_address >> 24, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // enable SPI dual IO mode and set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			17
Function:           WB_Fast_Read_Quad_Output()
Description:        Fast read dual output function
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:            AKA fast read
*/
unsigned int WB_Fast_Read_Quad_Output(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned char status;
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    status = WB_Read_Status_Register_2();
    WB_Write_Status_Register_2(status | WB_Flash_Status2_QE_Msk);

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Quad_Output, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);

    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // enable SPI dual IO mode and set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable Quad IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			18
Function:           WB_Fast_Read_Quad_Output_4Byte_Address()
Description:        Fast read dual output function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:            AKA fast read
*/
unsigned int WB_Fast_Read_Quad_Output_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Quad_Output_4Byte_Address, SIO);
    WB_SPINOR_SPIin(read_address >> 24, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);

    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // enable SPI dual IO mode and set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable Quad IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}



/*
Command#:			19
Function:           WB_Fast_Read_Dual_IO()
Description:        Fast read dual output function with SPI
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read_Dual_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Dual_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16, DIO | SPI_DIR_OUTPUT); // enable dual IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 8, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DIO | SPI_DIR_OUTPUT);

    WB_SPINOR_SPIin(WB_Flash_Dummy,
                    (DIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk |
                                              SPI_PARAM_CLEAR_RXFIFO_Msk)); // TODO: read command bypass mode ... M7-0

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			20
Function:           WB_DTR_Fast_Read_Dual_IO()
Description:        DTR fast read dual output function with SPI
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_DTR_Fast_Read_Dual_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_DTR_Fast_Read_Dual_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16,
                    DTDIO | SPI_DIR_OUTPUT); // enable DTR mode & dual IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 8, DTDIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DTDIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy,
                    (DTDIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk); // TODO: read command bypass mode ... M7-0

    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTDIO | SPI_DIR_OUTPUT) | SPI_DUMMY_MODEL_2_Msk); // 4 dummy clocks

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTDIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode & DTR mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTDIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                           SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			21
Function:           WB_Fast_Read_Dual_IO_4Byte_Address()
Description:        Fast read dual output function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read_Dual_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Dual_IO_4Byte_Address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 24, DIO | SPI_DIR_OUTPUT); // enable dual IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 16, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address >> 8, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DIO | SPI_DIR_OUTPUT);

    WB_SPINOR_SPIin(WB_Flash_Dummy,
                    (DIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk |
                                              SPI_PARAM_CLEAR_RXFIFO_Msk)); // TODO: read command bypass mode ... M7-0

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}



/*
Command#:			22
Function:           WB_Fast_Read_Quad_IO()
Description:        Fast read quad output function with SPI
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read_Quad_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Quad_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16, QIO | SPI_DIR_OUTPUT); // enable dual IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 8, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT); // TODO: read command bypass mode ... M7-0

    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

//DTR 24 EDh

/*
Command#:			23
Function:           WB_DTR_Fast_Read_Quad_IO()
Description:        DTR fast read quad output function with SPI
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_DTR_Fast_Read_Quad_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_DTR_Fast_Read_Quad_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16,
                    DTQIO | SPI_DIR_OUTPUT); // enable DTR mode & quad IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 8, DTQIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DTQIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy,
                    (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk); // Read command bypass mode ... M7-0

    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk); // 7 dummy inputs
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTQIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI quad IO mode & DTR mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTQIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                           SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			24
Function:           WB_DTR_Fast_Read_Quad_IO_4Byte_Address()
Description:        DTR fast read quad output function with 4-Byte address
Arguments:          read_address: starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_DTR_Fast_Read_Quad_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_Enter_4Byte_Address_Mode();  // ADS

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_DTR_Fast_Read_Quad_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 24,
                        DTQIO | SPI_DIR_OUTPUT); // enable DTR mode & quad IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 16, DTQIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address >> 8, DTQIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DTQIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy,
                    (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk); // Read command bypass mode ... M7-0

    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk); // 7 dummy inputs
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DTQIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTQIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI quad IO mode & DTR mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTQIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                           SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();

    WB_Exit_4Byte_Address_Mode();

    return sum_of_data;
}

/*
Command#:			25
Function:           WB_Fast_Read_Quad_IO_4Byte_Address()
Description:        Fast read quad output function with 4-Byte address
Arguments:          read_address: 4-Byte starting address to read
                    read_count: number of the data to read
                    read_buf: pointer for the read data
Return:				check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Fast_Read_Quad_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf)
{
	unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Fast_Read_Quad_IO_4Byte_Address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 24, QIO | SPI_DIR_OUTPUT); // enable dual IO mode and set direction to output
    WB_SPINOR_SPIin(read_address >> 16, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address >> 8, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT); // TODO: read command bypass mode ... M7-0

    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}


/*
Command#:			26
Function:           WB_Set_Burst_With_Wrap()
Description:
Arguments:
Return:
Comment:
*/
void WB_Set_Burst_With_Wrap(unsigned int wrap_settin)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Set_Burst_With_Wrap, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    // Wrap Bit
    WB_SPINOR_SPIin(wrap_settin, (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk |
                                                           SPI_PARAM_DISABLE_FUNCTION_Msk));
    WB_CS_High();
    return;
}



/*
Command#:			27
Function:           WB_Page_Program()
Description:        Data page program
Arguments:          program_address: starting address to program
                    program_count: number of the data to program
                    program_buf: pointer for the program data
Return:
Comment:
*/
void WB_Page_Program(unsigned int program_address, unsigned int program_count, unsigned char *program_buf)
{
    unsigned char result;
    unsigned int index = 0;

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Page_Program, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), SIO | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO
            result =
                WB_SPINOR_SPIin(*(program_buf + index), SIO | (SPI_PARAM_RETURN_TXRESULT_Msk |
                                                               SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }

    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:           28
Function:           WB_Page_Program_4Byte_Address()
Description:        Data page program with 4-Byte address
Arguments:          program_address: 4-Byte starting address to program
                    program_count: number of the data to program
                    program_buf: pointer for the program data
Return:
Comment:
*/
void WB_Page_Program_4Byte_Address(unsigned int program_address, unsigned int program_count, unsigned char *program_buf)
{
    unsigned char result;
    unsigned int index = 0;

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Page_Program_4Byte_Address, SIO);
    WB_SPINOR_SPIin(program_address >> 24, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), SIO | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO
            result = WB_SPINOR_SPIin(*(program_buf + index), SIO | (SPI_PARAM_RETURN_TXRESULT_Msk |
                                  SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }

    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:			29
Function:           WB_Quad_input_Page_Program()
Description:        Data page program
Arguments:          program_address: starting address to program
                    program_count: number of the data to program
                    program_buf: pointer for the program data
Return:
Comment:
*/
void WB_Quad_Input_Page_Program(unsigned int program_address, unsigned int program_count, unsigned char *program_buf)
{
    unsigned char result;
    unsigned char status;
    unsigned int index = 0;

    status = WB_Read_Status_Register_2();
    WB_Write_Status_Register_2(status | WB_Flash_Status2_QE_Msk);

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Quad_Input_Page_Program, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), (QIO | SPI_DIR_OUTPUT) | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO & disable SPI quad IO mode
            result =
                WB_SPINOR_SPIin(*(program_buf + index),
                                (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_RETURN_TXRESULT_Msk | SPI_PARAM_CHECK_BUSY_Msk |
                                                          SPI_PARAM_CLEAR_RXFIFO_Msk | SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }

    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:			30
Function:           WB_Quad_Input_Page_Program_4Byte_Address()
Description:        Data page program with 4-Byte address
Arguments:          program_address: 4-Byte starting address to program
                    program_count: number of the data to program
                    program_buf: pointer for the program data
Return:
Comment:
*/
void WB_Quad_Input_Page_Program_4Byte_Address(unsigned int program_address, unsigned int program_count, unsigned char *program_buf)
{
    unsigned char result;
    unsigned int index = 0;

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Quad_Input_Page_Program_4Byte_Address, SIO);
    WB_SPINOR_SPIin(program_address >> 24, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), (QIO | SPI_DIR_OUTPUT) | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO & disable SPI quad IO mode
            result =
                WB_SPINOR_SPIin(*(program_buf + index),
                                (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_RETURN_TXRESULT_Msk | SPI_PARAM_CHECK_BUSY_Msk |
                                                          SPI_PARAM_CLEAR_RXFIFO_Msk | SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }

    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:			31
Function:           WB_Sector_Erase()
Description:        4 KB sector erase
Arguments:          erase_address: starting address to erase
Return:
Comment:
*/
void WB_Sector_Erase(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Sector_Erase, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:			32
Function:           WB_Sector_Erase_4Byte_Address()
Description:        4 KB sector erase with 4-Byte address
Arguments:          erase_address: 4-Byte starting address to erase
Comment:
*/
void WB_Sector_Erase_4Byte_Address(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Sector_Erase_4Byte_Address, SIO);
    WB_SPINOR_SPIin(erase_address >> 24, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}



/*
Command#:			33
Function:           WB_Block_Erase_32KB()
Description:        32 KB block erase
Arguments:          erase_address: starting address to erase
Return:
Comment:
*/
void WB_Block_Erase_32KB(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Block_Erase_32KB, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			34
Function:           WB_Block_Erase_64KB()
Description:        64 KB block erase
Arguments:          erase_address: starting address to erase
Return:
Comment:
*/
void WB_Block_Erase_64KB(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Block_Erase, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}


/*
Command#:           35
Function:           WB_Block_Erase_64KB_4Byte_Address()
Description:        64 KB block erase with 4-Byte address
Arguments:          erase_address: 4-Byte starting address to erase
Return:
Comment:
*/
void WB_Block_Erase_64KB_4Byte_Address(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Block_Erase_4Byte_Address, SIO);
    WB_SPINOR_SPIin(erase_address >> 24, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			36
Function:           WB_Chip_Erase()
Description:        All memory within the device erase
Arguments:
Return:
Comment:
*/
void WB_Chip_Erase()
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Chip_Erase, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    WB_Wait_Ready();
    return;
}

/*
Command#:			37
Function:           WB_Suspend()
Description:        Interrupt a Sector or Block Erase operation or a Page Program operation.
Arguments:
Return:
Comment:
*/
void WB_Suspend(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Suspend, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tSUS);
    return;
}

/*
Command#:			38
Function:           WB_resume()
Description:        Resume the Sector or Block Erase or the Page Program after an Erase/Program Suspend.
Arguments:
Return:
Comment:
*/
void WB_Resume(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Resume, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tSUS);
    return;
}

/*
Command#:			39
Function:           WB_Power_Down()
Description:        Enter power-down mode.
Arguments:
Return:
Comment:
*/
void WB_Power_Down(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Power_Down, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tDP);
    return;
}

/*
Command#:			40.1
Function:           WB_Release_Power_Down()
Description:        Release the device from the power-down state.
Arguments:
Return:
Comment:
*/
void WB_Release_Power_Down(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Release_Power_Down, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tRES1);
    return;
}

/*
Command#:			40.2
Function:           WB_Release_Power_Down_ID()
Description:
Arguments:
Return:
Comment:
*/
void WB_Release_Power_Down_ID(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_ID_PD, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tRES2);
    return;
}

/*
Command#:			41
Function:           WB_Read_MFTR_ID()
Description:        Read out Manufacturer / Device ID
Arguments:
Return:				return 2 byte ID
Comment:
*/
unsigned int WB_Read_MFTR_ID()
{
    unsigned int MFTR_ID = 0;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_MFTR_ID, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    MFTR_ID = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    MFTR_ID <<= 8;
    MFTR_ID += WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return MFTR_ID;
}

/*
Command#:			42
Function:           WB_Read_MFTR_ID_Dual_IO()
Description:        Read out Manufacturer / Device ID
Arguments:
Return:				return 2 byte ID
Comment:
*/
unsigned int WB_Read_MFTR_ID_Dual_IO()
{
    unsigned int MFTR_ID = 0;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_MFTR_ID_Dual_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    // A23-A0 and M7-0
    WB_SPINOR_SPIin(WB_Flash_Dummy, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, DIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (DIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    MFTR_ID =
        WB_SPINOR_SPIin(WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    MFTR_ID <<= 8;
    MFTR_ID +=
        WB_SPINOR_SPIin(WB_Flash_Dummy, (DIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                                 SPI_PARAM_DISABLE_FUNCTION_Msk));
    WB_CS_High();
    return MFTR_ID;
}

/*
Command#:			43
Function:           WB_Read_MFTR_ID_Quad_IO()
Description:        Read out Manufacturer / Device ID
Arguments:
Return:				return 2 byte ID
Comment:
*/
unsigned int WB_Read_MFTR_ID_Quad_IO()
{
    unsigned int MFTR_ID = 0;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_MFTR_ID_Quad_IO, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    // A23-A0 and M7-0
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(WB_Flash_Dummy, (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    MFTR_ID =
        WB_SPINOR_SPIin(WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    MFTR_ID <<= 8;
    MFTR_ID +=
        WB_SPINOR_SPIin(WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                                 SPI_PARAM_DISABLE_FUNCTION_Msk));
    WB_CS_High();
    return MFTR_ID;
}

/*
Command#:			44
Function:           WB_Read_Unique_ID()
Description:        Read out Unique ID
Arguments:
Return:				return 8 byte Unique ID
Comment:
*/
void WB_Read_Unique_ID(unsigned char *unique_id)
{
    unsigned int index;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Unique_ID, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    // 64-bit unique serial number
    for (index = 0; index < 8; index++)
    {
        *(unique_id + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    }
    WB_CS_High();
    return;
}

/*
Command#:			45
Function:           WB_Read_JEDEC_ID()
Description:        Read out JEDEC ID
Arguments:
Return:				return 3 byte JEDEC ID
Comment:
*/
unsigned int WB_Read_JEDEC_ID()
{
    unsigned int JEDEC_ID = 0;
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_JEDEC_ID, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    JEDEC_ID = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    JEDEC_ID <<= 8;
    JEDEC_ID += WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    JEDEC_ID <<= 8;
    JEDEC_ID += WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return JEDEC_ID;
}

/*
Command#:			46
Function:           WB_Read_SFDP_Register()
Description:
Arguments:          read_address:
                    read_buf:
Return:
Comment:
*/
unsigned char WB_Read_SFDP_Register(unsigned int read_address, unsigned char *read_buf)
{
    unsigned int index = 0;

    if ((read_address & WB_Flash_SFDP_Register_Address_Msk) != WB_Flash_SFDP_Register_Address)
    {
        return FALSE;
    }

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_SFDP_Register, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (1)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        index++;

        if (index > 0xFF)
            break;
    }

    WB_CS_High();
    return TRUE;
}

/*
Command#:			47
Function:           WB_Erase_Security_Register()
Description:
Arguments:          erase_address:
Return:
Comment:
*/
unsigned char WB_Erase_Security_Register(unsigned int erase_address)
{
    unsigned int address = 0;

    address = erase_address & WB_Flash_Security_Register_Address_Msk;
    if ((address != WB_Flash_Security_Register_Address_1) && (address != WB_Flash_Security_Register_Address_2) &&
        (address != WB_Flash_Security_Register_Address_3))
    {
        return FALSE;
    }

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Erase_Security_Register, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    WB_Wait_Ready();
    return TRUE;
}

/*
Command#:			48
Function:           WB_Program_Security_Register()
Description:
Arguments:          program_address:
                    program_buf:
Return:
Comment:
*/
unsigned char WB_Program_Security_Register(unsigned int program_address, unsigned char *program_buf)
{
    unsigned char result;
    unsigned int index = 0;
    unsigned int program_count = 256;
    unsigned int address = 0;

    address = program_address & WB_Flash_Security_Register_Address_Msk;
    if ((address != WB_Flash_Security_Register_Address_1) && (address != WB_Flash_Security_Register_Address_2) &&
        (address != WB_Flash_Security_Register_Address_3))
    {
        return FALSE;
    }

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Program_Security_Register, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), SIO | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO
            result =
                WB_SPINOR_SPIin(*(program_buf + index), SIO | (SPI_PARAM_RETURN_TXRESULT_Msk |
                                                               SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }

    WB_CS_High();
    WB_Wait_Ready();
    return TRUE;
}

/*
Command#:			49
Function:           WB_Read_Security_Register()
Description:
Arguments:          read_address:
                    read_buf:
Return:
Comment:
*/
unsigned char WB_Read_Security_Register(unsigned int read_address, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int address = 0;

    address = read_address & WB_Flash_Security_Register_Address_Msk;
    if ((address != WB_Flash_Security_Register_Address_1) && (address != WB_Flash_Security_Register_Address_2) &&
        (address != WB_Flash_Security_Register_Address_3))
    {
        return FALSE;
    }

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Securtiy_Register, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    while (1)
    {
        *(read_buf + index) =
            WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        index++;

        if (index > 0xFF)
            break;
    }

    WB_CS_High();
    return TRUE;
}

/*
Command#:			50
Function:           WB_Set_Read_Parameters()
Description:        Set the number of dummy clocks and wrap length configurations
                    for set of selected instructions.
Arguments:
Return:
Comment:
*/
void WB_Set_Read_Parameters(unsigned char read_settin)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Set_Read_Parameters, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    WB_SPINOR_SPIin(read_settin, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    return;
}


/*
Command#:			51
Function:           WB_Burst_Read_With_Wrap()
Description:        Perform the read operation with wrap around in QPI mode.
Arguments:          read_address: starting address to read
                    read_count  : number of the data to read
                    dummy_clock : can be 2, 4, 6, 8, 10, 12, 14, 16
                    read_buf    : pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_Burst_Read_With_Wrap(unsigned int read_address, unsigned int read_count, unsigned char dummy_clock,
                                         unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;
    unsigned int dummy_byte_count;
    unsigned int i;

    WB_CS_Low();

    WB_SPINOR_SPIin(WB_Flash_CMD_Burst_Read_With_Wrap, QIO | SPI_DIR_OUTPUT);

    WB_SPINOR_SPIin(read_address >> 16, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address >> 8, QIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, QIO | SPI_DIR_OUTPUT);

    // "Set Read Parameters" instruction (C0h) can set the number of dummy clocks.
    //  Dummy clocks can be 2, 4, 6, 8, 10, 12, 14, 16 and real dummy bits are 8, 16, 24, 32, 40, 48, 56, 64
    dummy_byte_count = dummy_clock * 4 / 8;
    for (i = 0; i < dummy_byte_count; i++)
    {
        WB_SPINOR_SPIin(WB_Flash_Dummy,
                        (QIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    }

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (QIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                         SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        // sum_of_data += *(read_buf+index);  // necessary?
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			52
Function:           WB_DTR_Burst_Read_With_Wrap()
Description:        Perform the read operation with wrap around with DTR in SPI mode.
Arguments:          read_address: starting address to read
                    read_count  : number of the data to read
                    dummy_clock : can be 8, 10, 12, 14, 16
                    read_buf    : pointer for the read data
Return:             check_sum: return the sum of the read out data
Comment:
*/
unsigned int WB_DTR_Burst_Read_With_Wrap_QPI(unsigned int read_address, unsigned int read_count,
                                             unsigned char dummy_clock, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;
    unsigned int dummy_byte_count;
    unsigned int i;

    WB_CS_Low();

    WB_SPINOR_SPIin(WB_Flash_CMD_DTR_Burst_Read_With_Wrap, (QIO | SPI_DIR_OUTPUT) | SPI_PARAM_CHECK_BUSY_Msk);

    WB_SPINOR_SPIin(read_address >> 16, DTSIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address >> 8, DTSIO | SPI_DIR_OUTPUT);
    WB_SPINOR_SPIin(read_address, DTSIO | SPI_DIR_OUTPUT);

    // "Set Read Parameters" instruction (C0h) can set the number of dummy clocks.
    //  Dummy clocks can be 8, 10, 12, 14, 16 and real dummy bits multiply by 8
    dummy_byte_count = dummy_clock;
    for (i = 0; i < dummy_byte_count; i++)
    {
        WB_SPINOR_SPIin(WB_Flash_Dummy,
                        (DTSIO | SPI_DIR_OUTPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    }

    while (read_count)
    {
        // set direction to input
        if (read_count > 1)
        {
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTSIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        }
        else
        {
            // last byte, disable SPI dual IO mode
            *(read_buf + index) = WB_SPINOR_SPIin(
                WB_Flash_Dummy, (DTSIO | SPI_DIR_INPUT) | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk |
                                                           SPI_PARAM_DISABLE_FUNCTION_Msk));
        }

        // sum_of_data += *(read_buf+index);  // necessary?
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			53
Function:           WB_Enter_QPI_Mode()
Description:        Switch the device from SPI mode to QPI mode.
Arguments:
Return:
Comment:
*/
void WB_Enter_QPI_Mode(void)
{
    unsigned char status;
    status = WB_Read_Status_Register_2();
    WB_Write_Status_Register_2(status | WB_Flash_Status2_QE_Msk);

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enter_QPI_Mode, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk);
    WB_CS_High();
    return;
}

/*
Command#:			54
Function:           WB_Exit_QPI_Mode()
Description:        Exit the QPI mode and return to SPI mode.
Arguments:
Return:
Comment:
*/
void WB_Exit_QPI_Mode(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Exit_QPI_Mode,
                    (QIO | SPI_DIR_OUTPUT) |
                        (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk | SPI_PARAM_DISABLE_FUNCTION_Msk));
    WB_CS_High();
    return;
}


/*
Command#:			55
Function:           WB_Individual_Block_Sector_Lock()
Description:        Individual block and sector lock
Arguments:			write_address:
Return:
Comment:			Send write enable before lock
					Use 4 byte address input in 4byte address mode
*/
void  WB_Individual_Block_Sector_Lock(unsigned int write_address)
{
    WB_Write_Enable();
	WB_CS_Low();
    WB_SPINOR_SPIin(Individual_Block_Sector_Lock, SIO);
    WB_SPINOR_SPIin(write_address >> 16, SIO);
    WB_SPINOR_SPIin(write_address >> 8, SIO);
    WB_SPINOR_SPIin(write_address, SIO);
    WB_CS_High();
    return;
}

/*
Command#:			56
Function:           WB_Individual_Block_Sector_Unlock()
Description:        Individual block and sector lock
Arguments:			write_address:
Return:
Comment:			Send write enable before lock
					Use 4 byte address input in 4byte address mode
*/
void  WB_Individual_Block_Sector_Unlock(unsigned int write_address)
{
    WB_Write_Enable();
	WB_CS_Low();
    WB_SPINOR_SPIin(Individual_Block_Sector_Unlock, SIO);
    WB_SPINOR_SPIin(write_address >> 16, SIO);
    WB_SPINOR_SPIin(write_address >> 8, SIO);
    WB_SPINOR_SPIin(write_address, SIO);
    WB_CS_High();
    return;
}


/*
Command#:			57
Function:           WB_Read_Block_Sector_Lock()
Description:		Read lock bit
Arguments:          read_address:
                    read_buf:
Return:
Comment:			Use 4 byte address input in 4byte address mode
*/
unsigned char WB_Read_Block_Sector_Lock(unsigned int read_address, unsigned char read_lock_bit)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(Read_Block_Sector_Lock, SIO);
    WB_SPINOR_SPIin(read_address >> 16, SIO);
    WB_SPINOR_SPIin(read_address >> 8, SIO);
    WB_SPINOR_SPIin(read_address, SIO);
	read_lock_bit = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
    WB_CS_High();
    return read_lock_bit;
}

/*
Command#:			58
Function:           WB_Global_Block_Sector_Lock()
Description:        Global block and sector lock
Arguments:
Return:
Comment:			Send write enable before lock
*/
void  WB_Global_Block_Sector_Lock()
{
    WB_Write_Enable();
	WB_CS_Low();
    WB_SPINOR_SPIin(Global_Block_Sector_Lock, SIO);
    WB_CS_High();
    return;
}

/*
Command#:			59
Function:           WB_Global_Block_Sector_Unlock()
Description:        Global block and sector lock
Arguments:
Return:
Comment:			Send write enable before unlock
*/
void  WB_Global_Block_Sector_Unlock()
{
    WB_Write_Enable();
	WB_CS_Low();
    WB_SPINOR_SPIin(Global_Block_Sector_Unlock, SIO);
    WB_CS_High();
    return;
}


/*
Command#:			60
Function:           WB_Software_Reset()
Description:        The device will return to its default power-on state.
Arguments:
Return:
Comment:
*/
void WB_Software_Reset(void)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enable_Reset, SIO | SPI_PARAM_CHECK_BUSY_Msk);
    WB_CS_High();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Reset_Device, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);
    WB_CS_High();
    Timer0_Delay_us(WB_Flash_tRST);
    return;
}

/*
Command#:			61
Function:           WB_Clear_Buffer()
Description:        Clear the 256-byte Page Buffer with command 81h
Arguments:          None
Return:             None
Comment:            Clears the entire 256-byte internal Page Buffer
*/
void WB_Clear_Buffer(unsigned int erase_address)
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Clear_Buffer, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);                    // A23-A16 (ignored for buffer)
    WB_SPINOR_SPIin(erase_address >> 8, SIO);                     // A15-A8 (ignored for buffer)
    WB_SPINOR_SPIin(erase_address, SIO);  
    WB_CS_High();
    return;
}

/*
Command#:			62
Function:           WB_Write_Buffer()
Description:        Write 1-256 bytes of data to Page Buffer using Write Buffer instruction (82h)
Arguments:          buffer_address: starting address in buffer (0x00-0xFF)
                    write_count: number of bytes to write (1-256)
                    write_buf: pointer to data buffer to write
Return:             None
Comment:            No Write Enable required. Address wraps around at 256-byte boundary.
                    To write full 256 bytes, last address byte (A7-A0) must be 0.
*/
void WB_Write_Buffer(unsigned int buffer_address, unsigned int write_count, unsigned char *write_buf)
{
    unsigned char result;
    unsigned int index = 0;

    // Try adding Write Enable for Write Buffer
    WB_Write_Enable();
    
    WB_CS_Low();
    
    // Send Write Buffer command (82h)
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_Buffer, SIO);
    WB_SPINOR_SPIin(buffer_address >> 16, SIO);                    // A23-A16 (ignored for buffer)
    WB_SPINOR_SPIin(buffer_address >> 8, SIO);                     // A15-A8 (ignored for buffer)
    WB_SPINOR_SPIin(buffer_address, SIO);                          // A7-A0 (actual buffer address)
    
    // Write data bytes - follow Status Register pattern
    for (index = 0; index < write_count; index++)
    {
        if (index == (write_count - 1))
        {
            // Last byte with control parameters (like Status Register)
            WB_SPINOR_SPIin(write_buf[index], SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
        }
        else
        {
            // Regular data bytes
            WB_SPINOR_SPIin(write_buf[index], SIO);
        }
    }
    
    WB_CS_High();
    WB_Wait_Ready();
    return;


}

/*
Command#:			63
Function:           WB_Read_Buffer()
Description:        Read data from Page Buffer (SRAM) using Read Buffer instruction (83h)
Arguments:          buffer_address: starting address in buffer (0x00-0xFF)
                    read_count: number of bytes to read
                    read_buf: pointer for the read data
Return:				sum_of_data: return the sum of the read out data
Comment:            Standard SPI mode only, supports sequential read with auto address increment
*/
unsigned int WB_Read_Buffer(unsigned int buffer_address, unsigned int read_count, unsigned char *read_buf)
{
    unsigned int index = 0;
    unsigned int sum_of_data = 0;

    WB_CS_Low();
    
    // Send Read Buffer command (83h)
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Buffer, SIO);
    
    // Send 24-bit address (A23-A0), buffer address is typically 8-bit but sent as 24-bit
    WB_SPINOR_SPIin(buffer_address >> 16, SIO);                    // A23-A16 (always 0 for buffer)
    WB_SPINOR_SPIin(buffer_address >> 8, SIO);                    // A15-A8 (always 0 for buffer)
    WB_SPINOR_SPIin(buffer_address, SIO| (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));          // A7-A0 (actual buffer address)
    
    // Send 8-bit dummy
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    // Read data bytes sequentially
    while (read_count)
    {
        *(read_buf + index) = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));
        sum_of_data += *(read_buf + index);
        index++;
        read_count--;
    }

    WB_CS_High();
    return sum_of_data;
}

/*
Command#:			64
Function:           WB_Program_Buffer()
Description:        Program Buffer instruction (8Ah) writes Page Buffer content to specified Flash memory page
                    Write Enable instruction must be executed first (WEL bit needs to be 1)
                    Instruction format: 8Ah + 24-bit address (A23-A0)
Arguments:          flash_address: Target page address in Flash memory (must be page-aligned, lower 8 bits should be 0x00)
Return:             None
Comment:            1. Must execute WB_Write_Enable() first
                    2. Target address should be page start address (256-byte aligned)
                    3. Execution time is tPP, BUSY bit is 1 during operation
                    4. WEL bit is automatically cleared to 0 after completion
                    5. If target page is block-protected, program instruction will not be executed
*/
void WB_Program_Buffer(unsigned int flash_address)
{
    // 必須先執行Write Enable，確保WEL位元為1
    WB_Write_Enable();
    
    WB_CS_Low();
    
    // 發送Program Buffer命令(8Ah)
    WB_SPINOR_SPIin(WB_Flash_CMD_Program_Buffer, SIO);
    
    // 發送24位元地址(A23-A0)，指定Flash記憶體的目標頁面
    WB_SPINOR_SPIin(flash_address >> 16, SIO);                     // A23-A16
    WB_SPINOR_SPIin(flash_address >> 8, SIO);                      // A15-A8  
    WB_SPINOR_SPIin(flash_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk)); // A7-A0
    
    WB_CS_High();
    
    // 等待程式緩衝指令完成(tPP時間)
    // 期間可通過讀取狀態暫存器檢查BUSY位元
    WB_Wait_Ready();
    
    return;
}


/*
Command#:			65
Function:           WB_Load_Buffer()
Description:        
Arguments:
Return:
Comment:
*/
void WB_Load_Buffer(unsigned int flash_address)
{
    
    WB_CS_Low();
    
    // 發送Load Buffer命令(8Bh)
    WB_SPINOR_SPIin(WB_Flash_CMD_Load_Buffer, SIO);
    
    // 發送24位元地址(A23-A0)，指定Flash記憶體的目標頁面
    WB_SPINOR_SPIin(flash_address >> 16, SIO);                     // A23-A16
    WB_SPINOR_SPIin(flash_address >> 8, SIO);                      // A15-A8  
    WB_SPINOR_SPIin(flash_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk)); // A7-A0
    
    WB_CS_High();
    
    // 等待Load Buffer指令完成
    // 期間可通過讀取狀態暫存器檢查BUSY位元
    WB_Wait_Ready();
    
    return;
}


/*
Command#:			66
Function:           WB_Calculate_Checksum()
Description:        Calculate checksum instruction (9Dh) calculates checksum for specified Flash memory range
                    Requires start address (SA) and end address (EA) in 24/32-bit format
                    WEL bit is cleared to 0 after checksum calculation cycle completes
Arguments:          start_address: 24-bit starting address (SA23-SA0)
                    end_address: 24-bit ending address (EA23-EA0)
Return:             None
Comment:            1. Input 9Dh instruction followed by 24-bit SA and EA addresses
                    2. Does not accept Erase/Program Suspend (75h) during execution
                    3. 32-bit BYTESUM register resets on POR, HW/SW RESET, or FFh instruction
                    4. WEL bit cleared after completion
*/
void WB_Calculate_Checksum(unsigned int start_address, unsigned int end_address)
{
    // 先執行Write Enable (校驗和計算需要WEL=1)
    WB_Write_Enable();  // 根據W25Q51RV規格，Calculate Checksum需要Write Enable
    
    WB_CS_Low();
    
    // 發送Calculate Checksum指令 (9Dh)
    WB_SPINOR_SPIin(WB_Flash_CMD_Calculate_Checksum, SIO);
    
    // 發送24位元起始位址 (SA23-SA0)
    WB_SPINOR_SPIin(start_address >> 16, SIO);     // SA23-SA16
    WB_SPINOR_SPIin(start_address >> 8, SIO);      // SA15-SA8  
    WB_SPINOR_SPIin(start_address, SIO ); // SA7-SA0

    // 發送24位元結束位址 (EA23-EA0) - 不是32位元！
    WB_SPINOR_SPIin(end_address >> 24, SIO);     // SA23-SA16
    WB_SPINOR_SPIin(end_address >> 16, SIO);     // SA23-SA16
    WB_SPINOR_SPIin(end_address >> 8, SIO);      // SA15-SA8  
    WB_SPINOR_SPIin(end_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk)); // SA7-SA0
    WB_CS_High();
    
    // 等待校驗和計算完成
    // 在此期間BUSY位元為1，完成後WEL位元會被清除為0
    WB_Wait_Ready();
    
    return;
}

/*
Command#:			67
Function:           WB_Read_Checksum()
Description:        Read calculated checksum result from 32-bit BYTESUM register
                    Must be called after WB_Calculate_Checksum() completes
Arguments:          None (checksum is read from internal register)
Return:             32-bit checksum value
Comment:            1. Reads 4-byte checksum result from BYTESUM register
                    2. Should be called only after Calculate Checksum (9Dh) completion
                    3. Returns the sum of all bytes in the specified address range
*/
unsigned int WB_Read_Checksum(unsigned int start_address)
{
    unsigned int checksum_result = 0;
    unsigned char d0_d7, d8_d15, d16_d23, d24_d31;
    
    // 確保之前的計算完成
    WB_Wait_Ready();
    

    WB_CS_Low();
    
    // 發送 Read Checksum 指令 (9Ch)
    WB_SPINOR_SPIin(WB_Flash_CMD_Read_Checksum, SIO);
    
    // 嘗試不發送地址，直接進行 dummy + 讀取
    // 根據一些 Winbond 規格，Read Checksum 可能不需要地址
    
    // 8 dummy cycles (嘗試標準格式)
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));

    // 讀取 32-bit checksum (嘗試不同的位元組順序)
    d0_d7   = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));   // Byte 0
    d8_d15  = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));   // Byte 1
    d16_d23 = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));   // Byte 2
    d24_d31 = WB_SPINOR_SPIin(WB_Flash_Dummy, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_READ_RXFIFO_Msk));   // Byte 3
    
    WB_CS_High();
    
    // 嘗試不同的位元組順序組合
    checksum_result = (d24_d31 << 24) | (d16_d23 << 16) | (d8_d15 << 8) | d0_d7;  // Big Endian
    unsigned int checksum_le = (d0_d7 << 24) | (d8_d15 << 16) | (d16_d23 << 8) | d24_d31;  // Little Endian
    

    
    return checksum_result;
}


/*
Command#:			68
Function:           WB_Recovery_for_Erase()
Description:        
Arguments:
Return:
Comment:
*/
void WB_Recovery_for_Erase()
{
    // 先執行Write Enable 
    WB_Write_Enable();  
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Recovery_for_Erase, SIO);  
    WB_CS_High();    
    WB_Wait_Ready();
    
    return;
}
/*
Command#:			69
Function:           WB_Enter_Factory_Mode()
Description:        
Arguments:
Return:
Comment:
*/
void WB_Enter_Factory_Mode()
{

    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enter_Factory_Mode_1, SIO);
	WB_CS_High();
	WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enter_Factory_Mode_2, SIO);
	WB_CS_High();
	WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enter_Factory_Mode_3, SIO);
    WB_CS_High();    
    WB_Wait_Ready();    
    return;
}
/*
--------- Appendix command for testing or application note ---------
*/

/*
Command#:			N/A
Function:           WB_Enable_One_Time_Program()
Description:        AN000003 Special OTP function for specific product.
                    Send this command first before write SRL = 1
Arguments:
Return:
Comment:
*/
void WB_Enable_One_Time_Program()
{
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Enable_SR_OTP, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);
    WB_CS_High();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Write_SR_OTP, SIO | SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk);
    WB_CS_High();
    return;
}

/*
Command#:			N/A
Function:           WB_Block_Erase_NoRBCheck()
Description:        64KB block erase without checking BUSY for suspend resume testing
Arguments:          erase_address: starting address to erase
Return:
Comment:
*/
void WB_Block_Erase_NoRBCheck(unsigned int erase_address)
{
    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Block_Erase, SIO);
    WB_SPINOR_SPIin(erase_address >> 16, SIO);
    WB_SPINOR_SPIin(erase_address >> 8, SIO);
    WB_SPINOR_SPIin(erase_address, SIO | (SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
    WB_CS_High();
    return;
}

/*
Command#:			N/A
Function:           WB_Page_Program_NoRBCheck()
Description:		without checking BUSY for suspend resume testing
Arguments:          program_address: starting address to program
                    program_count: number of the data to program
                    program_buf: pointer for the program data
Return:
Comment:
*/
void WB_Page_Program_NoRBCheck(unsigned int program_address, unsigned int program_count, unsigned char *program_buf)
{
    unsigned char result;
    unsigned int index = 0;

    WB_Write_Enable();
    WB_CS_Low();
    WB_SPINOR_SPIin(WB_Flash_CMD_Page_Program, SIO);
    WB_SPINOR_SPIin(program_address >> 16, SIO);
    WB_SPINOR_SPIin(program_address >> 8, SIO);
    WB_SPINOR_SPIin(program_address, SIO | SPI_PARAM_CHECK_BUSY_Msk);

    while (program_count)
    {
        if (program_count > 1)
        {
            result = WB_SPINOR_SPIin(*(program_buf + index), SIO | SPI_PARAM_RETURN_TXRESULT_Msk);
        }
        else
        {
            // last byte, clear SPI RX FIFO
            result =
                WB_SPINOR_SPIin(*(program_buf + index), SIO | (SPI_PARAM_RETURN_TXRESULT_Msk |
                                                               SPI_PARAM_CHECK_BUSY_Msk | SPI_PARAM_CLEAR_RXFIFO_Msk));
        }

        if (result == TRUE)
        {
            index++;
            program_count--;
        }
    }
    WB_CS_High();
    return;
}

