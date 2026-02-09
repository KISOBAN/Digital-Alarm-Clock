cmake_minimum_required(VERSION 3.13)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(/home/kiso/Documents/raspberrypi/pico-sdk/pico_sdk_init.cmake)

project(blinkertest C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(dummy
    dummy.c dummy.pio
)

pico_generate_pio_header(dummy ${CMAKE_CURRENT_LIST_DIR}/dummy.pio)


# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(dummy pico_stdlib hardware_pio pico_multicore)

 # enable usb output, disable uart output

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(dummy)
