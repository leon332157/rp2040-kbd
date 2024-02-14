cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD seeed_xiao_rp2040)
include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(target_name "usb-kbd")
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(${target_name}
  ${target_name}.c
)
set(PICO_PIO_USB_DIR "${CMAKE_CURRENT_LIST_DIR}/Pico-PIO-USB/")
add_subdirectory(${PICO_PIO_USB_DIR} pico_pio_usb)

pico_enable_stdio_usb(${target_name} 1)
#pico_add_extra_outputs(${target_name})
target_link_libraries(${target_name} pico_stdlib pico_multicore pico_pio_usb)
target_link_options(${target_name} PUBLIC -Xlinker --print-memory-usage)
target_compile_options(${target_name} PUBLIC -Wall -Wextra)