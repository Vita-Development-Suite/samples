/*
	Vita Development Suite Samples
*/

#include <bgapputil.h>
#include <libsysmodule.h>

int main(void) {
	sceSysmoduleLoadModule(SCE_SYSMODULE_BG_APP_UTIL);
	sceBgAppUtilStartBgApp(0);
	return 0;
}
