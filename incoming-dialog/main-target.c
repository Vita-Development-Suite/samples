/*
	Vita Development Suite Samples
*/

#include <string.h>

#include <libdbg.h>
#include <appmgr.h>

int main(void) {
	char appParam[0x80];
	memset(appParam, 0, sizeof(appParam));
	sceAppMgrGetAppParam(appParam);
	SCE_DBG_LOG_INFO("App param: %s", appParam);
	return 0;
}
