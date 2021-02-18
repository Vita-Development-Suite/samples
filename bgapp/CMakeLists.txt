#
#   Vita Development Suite Samples
#

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

# Build main application

add_executable(bgapp
  main.c
)

target_link_libraries(bgapp
  SceBgAppUtil_stub
  SceSysmodule_stub
)

VDSuiteSignElf(bgapp.self bgapp
  BOOT_PARAM bootparam.yml
)

# Build background application

add_executable(bgapp-bg
  main-bg.c
)

target_link_libraries(bgapp-bg
  SceAppMgr_stub
  SceNotificationUtil_stub
  SceNotificationUtilBgApp_stub
  SceNotificationUtilProgress_stub
  SceSysmodule_stub
)

VDSuiteSignElf(bgapp-bg.self bgapp-bg
  BOOT_PARAM bootparam-bg.yml
)

# Build VPK

VDSuiteCreateSfo(bgapp-param.sfo param.sfx)
VDSuiteCreateSfo(bgapp-bg-param.sfo param-bg.sfx)

VDSuitePackage(bgapp.vpk
  FILES
    bgapp.self eboot.bin
    bgapp-bg.self eboot2.bin
    bgapp-param.sfo sce_sys/param.sfo
    bgapp-bg-param.sfo sce_sys/param2.sfo
)
