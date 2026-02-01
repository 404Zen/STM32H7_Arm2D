# Copilot 使用说明（为本仓库定制）

以下说明帮助 AI 编程代理快速理解本仓库的结构、关键约定、构建与调试工作流，以及常见模式和示例代码位置。

- **仓库类型**：STM32H7 嵌入式固件工程，基于 STM32CubeMX 生成的 HAL 驱动与手写用户代码。

- **主要目录**:
  - `cmake/stm32cubemx/`：CubeMX 生成的构建片段和自动生成的代码集成点（通常不应随意替换关键生成逻辑）。
  - `Core/`：应用核心代码，`Core/Src/main.c` 包含启动流程、MPU 配置与外设初始化，观察 `/* USER CODE BEGIN */` 块的使用习惯。
  - `Drivers/`：CMSIS 与 HAL 驱动（由 ST 提供）。
  - `User/`：项目自定义外设驱动与工具（例如 `User/ext_flash.c`, `User/w25qxx.c`, `User/WB_Command.c`）。
  - 根目录含 `CMakeLists.txt`, `CMakePresets.json` 和链接脚本 `STM32H7B0XX_FLASH.ld`（内存布局）。

- **架构与边界**:
  - CubeMX/HAL 负责外设配置和低层 API（位于 `Drivers/` 与 `cmake/stm32cubemx/`），高层逻辑与设备驱动位于 `Core/` 与 `User/`。
  - 外部 SPI/OctoSPI 闪存驱动在 `User/ext_flash.c`，可作为与外部闪存交互的主要示例（查找 `OSPI_` 和 `WB_Flash_CMD_*` 符号）。

- **构建与常用命令**（可在 Windows PowerShell 下运行）：
```powershell
# 使用 CMakePresets（推荐）
cmake --preset Debug
cmake --build --preset Debug

# 或显式：
cmake -S . -B build/Debug -G "Ninja" --preset Debug
ninja -C build/Debug
```
CMake 使用仓库内的工具链文件（`cmake/gcc-arm-none-eabi.cmake` 或 `cmake/starm-clang.cmake`），交叉工具链必须在 PATH 中（例如 `arm-none-eabi-gcc`）。

- **常见开发流程与调试**:
  - 在 `Core/Src/main.c` 中注意 `USER CODE` 区块：自动生成器会保留这些块内的改动。
  - 构建产物位于 `build/Debug`，生成的 `.map` 文件名称基于项目名（见 linker flags），用于内存分析。
  - 仿真/烧录通常使用厂商工具（ST-Link/OpenOCD），仓库不包含专用烧录脚本——不要假设存在通用闪存命令。

- **项目约定与风格**:
  - 错误码/返回值通常为 `int8_t` 或宏定义（参考 `User/ext_flash.c` 与对应头文件 `User/ext_flash.h`）。
  - 外设驱动函数以 `OSPI_` 前缀命名，HAL 命令结构体（如 `OSPI_RegularCmdTypeDef`）被大量复用。
  - 注释和被注释掉的备选实现（`#if 0`/注释段）表示历史或调试代码，AI 修改时应保持谨慎并尽量在 `User/` 下添加新代码而非改动生成代码。

- **集成点与变量**:
  - 构建系统通过 `cmake/stm32cubemx` 子目录将 CubeMX 输出整合进目标，改动该目录可能影响生成/再生流程。
  - 链接脚本 `STM32H7B0XX_FLASH.ld` 定义闪存/SRAM 分区，任何改变都会影响内存映射与运行时行为。

- **修改建议与限制**:
  - 若需修改外设初始化或固件入口，优先在 `Core/Src/main.c` 的 `USER CODE` 区块或 `User/` 下新增模块。
  - 不要删除或重写 `Drivers/` 下的 ST HAL文件；若需要修补，优先通过封装/适配层在 `User/` 中实现。

- **示例查找**（用于实现或参考）：
  - 外部闪存读写示例： `User/ext_flash.c`（查找 `OSPI_W25Qxx_WritePage`、`OSPI_Get_FlashID`）。
  - 系统时钟与 MPU 配置： `Core/Src/main.c`（搜索 `SystemClock_Config`、`MPU_Config`）。
  - CMake 工具链和 preset： `cmake/gcc-arm-none-eabi.cmake`, `CMakePresets.json`。

如果内容有遗漏或需要我把某些示例展开成更详细的开发/调试步骤（例如如何在本机用 ST-Link 烧写、如何配置调试器），请告诉我想要补充的部分。
