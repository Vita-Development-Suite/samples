#
#   Vita Development Suite Samples
#

if(NOT DEFINED ENV{SCE_GIMCONV})
  find_program(SCE_GIMCONV
    NAMES GimConv.exe GimConv
  )
else()
  set(SCE_GIMCONV $ENV{SCE_GIMCONV})
endif()

# Build self

add_executable(right-plugin
  main.c
)

# Build VPK

if(SCE_GIMCONV STREQUAL SCE_GIMCONV-NOTFOUND)
  set(RIGHT_GIM ${CMAKE_CURRENT_SOURCE_DIR}/right.gim)
  message(WARNING "GimConv not found.")
else()
  set(RIGHT_GIM ${CMAKE_CURRENT_BINARY_DIR}/right.gim)
  add_custom_command(OUTPUT ${RIGHT_GIM}
    COMMAND ${SCE_GIMCONV} ${CMAKE_CURRENT_SOURCE_DIR}/right.png
      --output_filename ${RIGHT_GIM}
      --image_format index8
      --pixel_order normal
      --format_endian little
    DEPENDS right.png
  )
endif()

VDSuiteCreateSfo(right-plugin-param.sfo param.yml)

VDSuitePackage(right-plugin.vpk
  FILES
    $<TARGET_FILE:right-plugin> eboot.bin
    right-plugin-param.sfo sce_sys/param.sfo
    ${RIGHT_GIM} sce_sys/about/right.gim
    right.txt sce_sys/about/right.txt
)
