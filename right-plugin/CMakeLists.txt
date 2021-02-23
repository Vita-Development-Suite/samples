#
#   Vita Development Suite Samples
#

set(SCE_GIMCONV GimConv CACHE STRING "GimConv")

# Build self

add_executable(right-plugin
  main.c
)

# Build VPK

add_custom_command(OUTPUT right.gim
  COMMAND ${SCE_GIMCONV} ${CMAKE_CURRENT_SOURCE_DIR}/right.png
    --output_filename ${CMAKE_CURRENT_BINARY_DIR}/right.gim
    --image_format index8
    --pixel_order normal
    --format_endian little
  DEPENDS right.png
)

VDSuiteCreateSfo(right-plugin-param.sfo param.yml)

VDSuitePackage(right-plugin.vpk
  FILES
    $<TARGET_FILE:right-plugin> eboot.bin
    right-plugin-param.sfo sce_sys/param.sfo
    right.gim sce_sys/about/right.gim
    right.txt sce_sys/about/right.txt
)
