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

add_executable(notification-util
  main.c
)

target_link_libraries(notification-util
  SceBgAppUtil_stub
  SceSysmodule_stub
)

VDSuiteSignElf(notification-util.self $<TARGET_FILE:notification-util>
  BOOT_PARAM bootparam.yml
)

# Build background application

add_executable(notification-util-bg
  main-bg.c
)

target_link_libraries(notification-util-bg
  SceNotificationUtil_stub
  SceNotificationUtilBgApp_stub
  SceNotificationUtilProgress_stub
  SceSysmodule_stub
)

VDSuiteSignElf(notification-util-bg.self $<TARGET_FILE:notification-util-bg>
  BOOT_PARAM bootparam-bg.yml
)

# Build VPK

VDSuiteCreateSfo(notification-util-param.sfo param.sfx)
VDSuiteCreateSfo(notification-util-bg-param.sfo param-bg.sfx)

VDSuitePackage(notification-util.vpk
  FILES
    notification-util.self eboot.bin
    notification-util-bg.self eboot2.bin
    notification-util-param.sfo sce_sys/param.sfo
    notification-util-bg-param.sfo sce_sys/param2.sfo
)
