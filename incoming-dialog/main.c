/*
	Vita Development Suite Samples
*/

#include <string.h>
#include <wchar.h>
#include <alloca.h>

#include <incoming_dialog.h>
#include <kernel.h>
#include <libsysmodule.h>
#include <libdbg.h>
#include <appmgr.h>

int main(void) {
	int ret;
	SceIncomingDialogParam *param;

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_INCOMING_DIALOG);
	if (ret < 0) {
		SCE_DBG_LOG_ERROR("Failed to load SceIncomingDialog 0x%08X", ret);
		goto done;
	}

	ret = sceIncomingDialogInit(0);
	if (ret < 0) {
		SCE_DBG_LOG_ERROR("Failed to initialize SceIncomingDialog 0x%08X", ret);
		goto done;
	}

	param = alloca(offsetof(SceIncomingDialogParam, appParam) + 0x80);
	sceIncomingDialogParamInit(param);
	strcpy(param->titleId, "VDSS20003");
	param->timeout = 15;
	wcsncpy(param->acceptText, L"OK", 0x1F);
	wcsncpy(param->rejectText, L"Close", 0x1F);
	wcsncpy(param->dialogText, L"Open target application?", 0x3F);
	wcsncpy(param->notificationText, L"Incoming dialog closed", 0x3F);
	strcpy(param->appParam, "Application parameter from incoming dialog");

	ret = sceIncomingDialogOpen(param);
	if (ret < 0) {
		SCE_DBG_LOG_ERROR("Failed to open dialog 0x%08X", ret);
		goto done;
	}

	while (sceIncomingDialogGetStatus() != SCE_INCOMING_DIALOG_STATUS_RUNNING) {
		sceKernelDelayThread(100 * 1000);
	}

	sceKernelDelayThread(2000 * 1000);
	sceIncomingDialogSwitchToDialog();

	while (sceIncomingDialogGetStatus() == SCE_INCOMING_DIALOG_STATUS_RUNNING) {
		sceKernelDelayThread(100 * 1000);
	}

	ret = sceIncomingDialogGetStatus();
	SCE_DBG_LOG_INFO("Dialog closed with status %d", ret);

done:
	return 0;
}
