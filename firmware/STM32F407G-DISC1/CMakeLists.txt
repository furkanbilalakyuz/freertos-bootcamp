# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.20)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Set target definition variables required by the toolchain file
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target.cmake)

# Set the toolchain file for cross-compilation
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

# Project definition
project(STM32F407G-DISC1)

# Enable CMake support for ASM and C/CPP languages
enable_language(C ASM)

set(PROJECT_NAME STM32F407G-DISC1)

# Create an executable object type
add_executable(${PROJECT_NAME})

# Include additional CMake files for flags, components, and files
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/components.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/files.cmake)

set(HAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver)

# Add sources to executable/library
target_sources(${PROJECT_NAME} PRIVATE
  # User defined sources
  "${CMAKE_CURRENT_SOURCE_DIR}/Src/system_stm32f4xx.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_it.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32f4xx_hal_msp.c"
  # STM32F4 HAL (stm32f4xx_hal_conf.h'de etkin olan moduller)
  "${HAL_DIR}/Src/stm32f4xx_hal.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_cortex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_rcc.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_rcc_ex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_flash.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_flash_ex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_flash_ramfunc.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_pwr.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_pwr_ex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_gpio.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_exti.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_dma.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_dma_ex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_tim.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_tim_ex.c"
  "${HAL_DIR}/Src/stm32f4xx_hal_uart.c"
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
  # User defined include directories
  "${HAL_DIR}/Inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include"
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
  # User defined compiler options
)

# Add additional defined symbols
target_compile_definitions(${PROJECT_NAME} PRIVATE
  # Add additional defined symbols here
  USE_HAL_DRIVER
  STM32F407xx
  HSE_VALUE=8000000U
)

# Linker options
target_link_options(${PROJECT_NAME} PRIVATE 
  # User defined linker options
)

# Add additional linker search directories
target_link_directories(${PROJECT_NAME} PRIVATE
  # Add additional linker search directories here
)

# Add additional linker libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  # Add additional linker libraries here
)
