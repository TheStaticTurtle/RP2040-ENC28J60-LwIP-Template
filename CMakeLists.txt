# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_ping C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(pico_ping 
    driver/enc28j60/enc28j60.c
    lwip/enc28j60-ethernetif.c
    lwip/nosys.c
    main.c
)


target_include_directories(pico_ping PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(pico_lwip INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/lwip
)


# Add any user requested libraries
target_link_libraries(pico_ping
    pico_stdlib 
    hardware_spi
    pico_lwip
    pico_lwip_http 
    pico_lwip_iperf  
)

pico_set_program_name(pico_ping "pico_ping")
pico_set_program_version(pico_ping "0.1")

pico_enable_stdio_usb(pico_ping 1)
pico_enable_stdio_uart(pico_ping 0)

pico_add_extra_outputs(pico_ping)

