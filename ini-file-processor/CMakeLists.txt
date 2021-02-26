#
#   Vita Development Suite Samples
#

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

# Build self

add_executable(ini-file-processor
  main.cpp
)

target_link_libraries(ini-file-processor
  SceIniFileProcessor_stub
  ScePafStdc_stub
  SceSysmodule_stub
)

# Build VPK

VDSuiteCreateSfo(ini-file-processor-param.sfo param.sfx)

VDSuitePackage(ini-file-processor.vpk
  FILES
    $<TARGET_FILE:ini-file-processor> eboot.bin
    ini-file-processor-param.sfo sce_sys/param.sfo
    default.ini default.ini
)
