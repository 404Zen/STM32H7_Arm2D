/*
	COPYRIGHT 2024 Winbond Electronics Corp.
	SPI FLASH Reference Code
	Version  V1.00
	
	Definitions of Flash devices and commands parameters.
*/

#define WB_Flash_PARAM_Timeout 1 // Set the value carefully, default disable timeout in WB_Wait_Ready()

#define WB_Flash_Dummy 0x0

// AC Characteristics
#define WB_Flash_tDP 3   // (AC Characteristics 3 us)
#define WB_Flash_tRES1 3 // (AC Characteristics 3 us)
#define WB_Flash_tRES2 2 // (AC Characteristics 1.8 us)
#define WB_Flash_tRST 30 // (AC Characteristics 30 us)
#define WB_Flash_tSUS 20 // A timer use to wait the next Instruction after Suspend (AC Characteristics 20 us)

// SPI NOR Command Set (Table 1)
#define WB_Flash_CMD_Write_Enable 0x06
#define WB_Flash_CMD_Write_Enable_Volatile_SR 0x50
#define WB_Flash_CMD_Write_Disable 0x04
#define WB_Flash_CMD_Read_Status_Register_1 0x05
#define WB_Flash_CMD_Read_Status_Register_2 0x35
#define WB_Flash_CMD_Read_Status_Register_3 0x15
#define WB_Flash_CMD_Write_Status_Register_1 0x01
#define WB_Flash_CMD_Write_Status_Register_2 0x31
#define WB_Flash_CMD_Write_Status_Register_3 0x11
#define WB_Flash_CMD_Read_Data 0x03
#define WB_Flash_CMD_Fast_Read 0x0B
#define WB_Flash_CMD_Page_Program 0x02
#define WB_Flash_CMD_Block_Erase 0xD8
#define WB_Flash_CMD_Block_Erase_4Byte_Address 0xDC
#define WB_Flash_CMD_Read_MFTR_ID 0x90
#define WB_Flash_CMD_Read_Unique_ID 0x4B
#define WB_Flash_CMD_Read_JEDEC_ID 0x9F
#define WB_Flash_CMD_Power_Down 0xB9
#define WB_Flash_CMD_Release_Power_Down 0xAB
#define WB_Flash_CMD_Read_ID_PD 0xAB
#define WB_Flash_CMD_Enable_Reset 0x66
#define WB_Flash_CMD_Reset_Device 0x99
#define WB_Flash_CMD_Suspend 0x75
#define WB_Flash_CMD_Resume 0x7A
#define WB_Flash_CMD_Sector_Erase 0x20
#define WB_Flash_CMD_Block_Erase_32KB 0x52
#define WB_Flash_CMD_Chip_Erase 0xC7 // 0x60
#define WB_Flash_CMD_Read_ECC_Status 0x25
#define WB_Flash_CMD_Read_SFDP_Register 0x5A
#define WB_Flash_CMD_Erase_Security_Register 0x44
#define WB_Flash_CMD_Program_Security_Register 0x42
#define WB_Flash_CMD_Read_Securtiy_Register 0x48
#define WB_Flash_CMD_Calculate_Checksum 0x9D
#define WB_Flash_CMD_Read_Checksum 0x9C
#define WB_Flash_CMD_Recovery_for_Erase 0xA8


//New Function for Buffer 
#define WB_Flash_CMD_Clear_Buffer 0x81
#define WB_Flash_CMD_Write_Buffer 0x82
#define WB_Flash_CMD_Read_Buffer 0x83
#define WB_Flash_CMD_Program_Buffer 0x8A
#define WB_Flash_CMD_Load_Buffer 0x8B
#define WB_Flash_CMD_Enter_Factory_Mode_1 0x51
#define WB_Flash_CMD_Enter_Factory_Mode_2 0xA5
#define WB_Flash_CMD_Enter_Factory_Mode_3 0x41


/*
	P[6:4] dummy clocks configuration
	P[1:0] wrap length configuration for QPI mode only
	Dual/Quad SPI Commnad Set (Table 2)
*/
#define WB_Flash_CMD_Set_Read_Parameters 0xC0 	
#define WB_Flash_CMD_Fast_Read_Dual_Output 0x3B
#define WB_Flash_CMD_Fast_Read_Dual_IO 0xBB
#define WB_Flash_CMD_Read_MFTR_ID_Dual_IO 0x92
#define WB_Flash_CMD_Quad_Input_Page_Program 0x32
#define WB_Flash_CMD_Fast_Read_Quad_Output 0x6B
#define WB_Flash_CMD_Read_MFTR_ID_Quad_IO 0x94
#define WB_Flash_CMD_Set_Burst_With_Wrap 0x77
#define WB_Flash_CMD_Fast_Read_Quad_IO 0xEB

// QPI Command Set (Table 3)
#define WB_Flash_CMD_Burst_Read_With_Wrap 0x0C

// DTR with SPI Command Set (Table 7)
#define WB_Flash_CMD_DTR_Fast_Read 0x0D
#define WB_Flash_CMD_DTR_Fast_Read_Dual_IO 0xBD
#define WB_Flash_CMD_DTR_Fast_Read_Quad_IO 0xED

// DTR with QPI Command Set (Table 9)
#define WB_Flash_CMD_DTR_Burst_Read_With_Wrap 0x0E

// Eextra Instructions
#define WB_Flash_CMD_Enter_QPI_Mode 0x38
#define WB_Flash_CMD_Exit_QPI_Mode 0xFF
#define WB_Flash_CMD_Enable_SR_OTP 0xAA
#define WB_Flash_CMD_Write_SR_OTP 0x55

// SPI NOR Mask
#define WB_Flash_MASK_WIP 0x01

// Set Burst with Wrap (77h)
#define WB_Flash_Burst_Mode_Pos (4)
#define WB_Flash_Burst_Mode_Msk (0x1ul << WB_Flash_Burst_Mode_Pos)

#define WB_Flash_Burst_Length_Pos (5)
#define WB_Flash_Burst_Length_Msk (0x3ul << WB_Flash_Burst_Length_Pos)

#define WB_Flash_Burst_8_Byte (0x0ul << WB_Flash_Burst_Length_Pos)
#define WB_Flash_Burst_16_Byte (0x1ul << WB_Flash_Burst_Length_Pos)
#define WB_Flash_Burst_32_Byte (0x2ul << WB_Flash_Burst_Length_Pos)
#define WB_Flash_Burst_64_Byte (0x3ul << WB_Flash_Burst_Length_Pos)

// Set Read Parameters (C0h)
#define WB_Flash_Read_Wrap_Length_Pos (0) // only applicable in QPI mode
#define WB_Flash_Read_Wrap_Length_Msk (0x3ul << WB_Flash_Read_Wrap_Length_Pos)

#define WB_Flash_Read_Dummy_Clock_Pos (4)
#define WB_Flash_Read_Dummy_Cloc_Msk (0x7ul << WB_Flash_Read_Dummy_Clock_Pos)

#define WB_Flash_Wrap_8_Byte (0x0ul << WB_Flash_Read_Wrap_Length_Pos) // default from power up
#define WB_Flash_Wrap_16_Byte (0x1ul << WB_Flash_Read_Wrap_Length_Pos)
#define WB_Flash_Wrap_32_Byte (0x2ul << WB_Flash_Read_Wrap_Length_Pos)
#define WB_Flash_Wrap_64_Byte (0x3ul << WB_Flash_Read_Wrap_Length_Pos)

// Status Register-1
#define WB_Flash_Status1_BUSY_Pos (0)
#define WB_Flash_Status1_BUSY_Msk (0x1ul << WB_Flash_Status1_BUSY_Pos) // S0

#define WB_Flash_Status1_WEL_Pos  (1)
#define WB_Flash_Status1_WEL_Msk  (0x1ul << WB_Flash_Status1_WEL_Pos)   // S1

// Status Register-2
#define WB_Flash_Status2_QE_Pos (1)
#define WB_Flash_Status2_QE_Msk (0x1ul << WB_Flash_Status2_QE_Pos) // S9

// Serial Flash Discoverable Parameter Register
#define WB_Flash_SFDP_Register_Address 0x000000 // A23-A8 = 0, A7-A0 = starting byte address

#define WB_Flash_SFDP_Register_Address_Pos (8)
#define WB_Flash_SFDP_Register_Address_Msk (0xFFFFul << WB_Flash_SFDP_Register_Address_Pos)

// Security Register
#define WB_Flash_Security_Register_Address_1                                                                           \
    0x001000 // A23-A16 = 00h, A15-A12 = 1h, A11-A8 = 0h, A7-A0 = starting byte address
#define WB_Flash_Security_Register_Address_2                                                                           \
    0x002000 // A23-A16 = 00h, A15-A12 = 2h, A11-A8 = 0h, A7-A0 = starting byte address
#define WB_Flash_Security_Register_Address_3                                                                           \
    0x003000 // A23-A16 = 00h, A15-A12 = 3h, A11-A8 = 0h, A7-A0 = starting byte address

#define WB_Flash_Security_Register_Address_Pos (8)
#define WB_Flash_Security_Register_Address_Msk (0xFFFFul << WB_Flash_Security_Register_Address_Pos)

#define WB_Flash_CMD_Read_Extended_Status_Register 0xC8
#define WB_Flash_CMD_Write_Extended_Status_Register 0xC5
#define WB_Flash_CMD_Enter_4_Byte_Mode 0xB7
#define WB_Flash_CMD_Exit_4_Byte_Mode 0xE9
#define WB_Flash_CMD_Read_Data_4Byte_Address 0x13
#define WB_Flash_CMD_Fast_Read_4Byte_Address 0x0C
#define WB_Flash_CMD_Fast_Read_Dual_Output_4Byte_Address 0x3C
#define WB_Flash_CMD_Fast_Read_Quad_Output_4Byte_Address 0x6C
#define WB_Flash_CMD_Fast_Read_Dual_IO_4Byte_Address 0xBC
#define WB_Flash_CMD_Fast_Read_Quad_IO_4Byte_Address 0xEC
#define WB_Flash_CMD_Page_Program_4Byte_Address 0x12
#define WB_Flash_CMD_Quad_Input_Page_Program_4Byte_Address 0x34
#define WB_Flash_CMD_Sector_Erase_4Byte_Address 0x21

// Individual Protect Command Set
#define Individual_Block_Sector_Lock 0x36
#define Individual_Block_Sector_Unlock 0x39
#define Read_Block_Sector_Lock 0x3D
#define Global_Block_Sector_Lock 0x7E
#define Global_Block_Sector_Unlock 0x98


// DTR with SPI Command Set (Table 7)
#define WB_Flash_CMD_DTR_Fast_Read 0x0D
#define WB_Flash_CMD_DTR_Fast_Read_Dual_IO 0xBD
#define WB_Flash_CMD_DTR_Fast_Read_Quad_IO 0xED
/*
    SPI NOR Parameter (3 Bytes)
    - Operation Mode Byte:
      [2:0]    0 = SPI
               1 = Dual SPI
               2 = Quad SPI
               3 = Octal SPI
               4 = SPI       & DTR
               5 = Dual SPI  & DTR
               6 = Quad SPI  & DTR
               7 = Octal SPI & DTR
      [5:3]    Reserved
      [7:6]    0x0 = N/A
               0x1 = set SPI Dual/Quad IO direction to input
               0x2 = set SPI Dual/Quad IO direction to output

    - Extend Parameter Byte:
      [15:8]   0x1  = check SPI BUSY
               0x2  = clear RX FIFO
               0x4  = return TX result
               0x8  = disable SPI function
               0x10 = read RX FIFO
               All others are reserved.

    - Dummy Model Byte for DTR instruction:
      [23:16]  0x1 = 6 dummy clock
               0x2 = 4 dummy clock
               0x4 = 7 dummy clock
               All others are reserved.
*/
#define WB_Flash_SPI_Mode_Msk 0x00000007
#define WB_Flash_SPI_DIR_Msk 0x000000C0
#define WB_Flash_SPI_PARAM_Msk 0x0000FF00
#define WB_Flash_SPI_DUMMY_Model_Msk 0x00FF0000

#define SIO 0
#define DIO 1
#define QIO 2
#define OIO 3
#define DTSIO 4
#define DTDIO 5
#define DTQIO 6
#define DTOIO 7

#define SPI_DIR_INPUT 0x40
#define SPI_DIR_OUTPUT 0x80

#define SPI_PARAM_CHECK_BUSY_Pos (8)
#define SPI_PARAM_CHECK_BUSY_Msk (0x1ul << SPI_PARAM_CHECK_BUSY_Pos)

#define SPI_PARAM_CLEAR_RXFIFO_Pos (9)
#define SPI_PARAM_CLEAR_RXFIFO_Msk (0x1ul << SPI_PARAM_CLEAR_RXFIFO_Pos)

#define SPI_PARAM_RETURN_TXRESULT_Pos (10)
#define SPI_PARAM_RETURN_TXRESULT_Msk (0x1ul << SPI_PARAM_RETURN_TXRESULT_Pos)

#define SPI_PARAM_DISABLE_FUNCTION_Pos (11)
#define SPI_PARAM_DISABLE_FUNCTION_Msk (0x1ul << SPI_PARAM_DISABLE_FUNCTION_Pos)

#define SPI_PARAM_READ_RXFIFO_Pos (12)
#define SPI_PARAM_READ_RXFIFO_Msk (0x1ul << SPI_PARAM_READ_RXFIFO_Pos)

#define SPI_DUMMY_MODEL_1_Pos (16)
#define SPI_DUMMY_MODEL_1_Msk (0x1ul << SPI_DUMMY_MODEL_1_Pos)

#define SPI_DUMMY_MODEL_2_Pos (17)
#define SPI_DUMMY_MODEL_2_Msk (0x1ul << SPI_DUMMY_MODEL_2_Pos)

#define SPI_DUMMY_MODEL_3_Pos (18)
#define SPI_DUMMY_MODEL_3_Msk (0x1ul << SPI_DUMMY_MODEL_3_Pos)

void WB_Power_Down(void);
void WB_Release_Power_Down(void);
void WB_Software_Reset(void);
unsigned char WB_Read_SFDP_Register(unsigned int read_address, unsigned char *read_buf);
unsigned char WB_Read_Security_Register(unsigned int param1, unsigned char *param2);
unsigned char WB_Program_Security_Register(unsigned int param1, unsigned char *param2);
unsigned char WB_Erase_Security_Register(unsigned int param);
unsigned char WB_Read_ECC_Status_Register();
unsigned char WB_Read_Status_Register_1();
unsigned char WB_Read_Status_Register_2();
unsigned char WB_Read_Status_Register_3();
void WB_Write_Status_Register_1(unsigned char sr_data_write);
void WB_Write_Status_Register_2(unsigned char sr_data_write);
void WB_Write_Status_Register_3(unsigned char sr_data_write);
void WB_Chip_Erase();
void WB_Block_Erase_64KB(unsigned int erase_address);
void WB_Block_Erase_64KB_4Byte_Address(unsigned int erase_address);
void WB_Block_Erase_32KB(unsigned int erase_address);
void WB_Sector_Erase(unsigned int erase_address);
void WB_Sector_Erase_4Byte_Address(unsigned int erase_address);
void WB_Page_Program(unsigned int program_address, unsigned int program_count, unsigned char *program_buf);
void WB_Page_Program_4Byte_Address(unsigned int program_address, unsigned int program_count, unsigned char *program_buf);
void WB_Quad_Input_Page_Program(unsigned int program_address, unsigned int program_count, unsigned char *program_buf);
void WB_Quad_Input_Page_Program_4Byte_Address(unsigned int program_address, unsigned int program_count, unsigned char *program_buf);
void WB_Read_Unique_ID(unsigned char *unique_id);
unsigned int WB_Read_JEDEC_ID();
unsigned int WB_Read_MFTR_ID();
unsigned int WB_Read_MFTR_ID_Dual_IO();
unsigned int WB_Read_MFTR_ID_Quad_IO();
unsigned int WB_Read_Data(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Read_Data_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Dual_Output(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Dual_Output_4Byte_Addrerss(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Quad_Output(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Quad_Output_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Dual_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Dual_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Quad_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_Fast_Read_Quad_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read_Dual_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read_Quad_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);

unsigned int WB_Burst_Read_With_Wrap(unsigned int read_address, unsigned int read_count, unsigned char dummy_clock,
                                         unsigned char *read_buf);
unsigned int WB_DTR_Burst_Read_With_Wrap_QPI(unsigned int read_address, unsigned int read_count,
                                             unsigned char dummy_clock, unsigned char *read_buf);
void WB_Enter_QPI_Mode(void);
void WB_Exit_QPI_Mode(void);
unsigned char WB_Read_Status_Register_2_QPI();

void WB_Enable_One_Time_Program();
void WB_Set_Burst_With_Wrap(unsigned int wrap_settin);
void WB_Set_Read_Parameters(unsigned char read_settin);
void WB_Release_Power_Down_ID(void);
void WB_Suspend(void);
void WB_Resume(void);
void WB_Write_Enable_For_Volatile_SR();
void WB_Write_Volatile_Status_Register_1(unsigned char sr_data_write);
void WB_Write_Volatile_Status_Register_2(unsigned char sr_data_write);
void WB_Write_Volatile_Status_Register_3(unsigned char sr_data_write);

void WB_Block_Erase_NoRBCheck(unsigned int erase_address);
void WB_Page_Program_NoRBCheck(unsigned int program_address, unsigned int program_count, unsigned char *program_buf);

//

unsigned char WB_Enter_4Byte_Address_Mode();
unsigned char WB_Exit_4Byte_Address_Mode();

void  WB_Individual_Block_Sector_Lock(unsigned int write_address);
void  WB_Individual_Block_Sector_Unlock(unsigned int write_address);
unsigned char WB_Read_Block_Sector_Lock(unsigned int read_address, unsigned char read_lock_bit);
void  WB_Global_Block_Sector_Lock();
void  WB_Global_Block_Sector_Unlock();

//DTR

unsigned int WB_DTR_Fast_Read(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read_Dual_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read_Quad_IO(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);
unsigned int WB_DTR_Fast_Read_Quad_IO_4Byte_Address(unsigned int read_address, unsigned int read_count, unsigned char *read_buf);


//buffer function
void WB_Clear_Buffer(unsigned int erase_address);
void WB_Write_Buffer(unsigned int buffer_address, unsigned int write_count, unsigned char *write_data);
unsigned int WB_Read_Buffer(unsigned int buffer_address, unsigned int read_count, unsigned char *read_buf);
void WB_Program_Buffer(unsigned int flash_address);

//checksum function
void WB_Calculate_Checksum(unsigned int start_address, unsigned int end_address);
unsigned int WB_Read_Checksum(unsigned int start_address);
unsigned int WB_Read_Checksum_Debug(void);
