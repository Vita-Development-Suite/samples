/*
	Vita Development Suite Samples
*/

#include <string.h>
#include <wchar.h>

#include <appmgr.h>
#include <kernel/threadmgr.h>
#include <libsysmodule.h>
#include <notification_util.h>

#define SET_TEXT(param, text_) wcsncpy(param.text, text_, SCE_NOTIFICATION_UTIL_TEXT_MAX)
#define SET_SUB_TEXT(param, text_) wcsncpy(param.subText, text_, SCE_NOTIFICATION_UTIL_TEXT_MAX)
#define SET_CANCEL_TEXT(param, text_) wcsncpy(param.cancelText, text_, SCE_NOTIFICATION_UTIL_TEXT_MAX)

#define PROGRESS_EVT_CANCEL 0x00000001

void cancelCallback(void *userData) {
	sceKernelSetEventFlag((SceUID)userData, PROGRESS_EVT_CANCEL);
}

int main(void) {
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceNotificationUtilBgAppInitialize();

	// Send regular notification
	SceNotificationUtilSendParam sendParam = {0};
	SET_TEXT(sendParam, L"Hello from background application");
	sceNotificationUtilSendNotification(&sendParam);

	sceKernelDelayThread(2 * 1000 * 1000);

	// Create progress notification event flag
	SceUID evfId = sceKernelCreateEventFlag(
		"ProgressEvf",
		SCE_KERNEL_EVF_ATTR_TH_FIFO | SCE_KERNEL_EVF_ATTR_MULTI,
		0x00000000,
		NULL);

	// Start progress notification
	SceNotificationUtilProgressInitParam beginParam = {0};
	SET_TEXT(beginParam, L"Progress start");
	SET_SUB_TEXT(beginParam, L"Progress start sub text");
	beginParam.cancelCallback = cancelCallback;
	beginParam.userData = (void*)evfId;
	SET_CANCEL_TEXT(beginParam, L"Progress cancel");
	sceNotificationUtilProgressBegin(&beginParam);

	// Continue progress notification
	int progressSteps = 20;
	int progressDelay = 1000 * 1000;
	int cancelled = 0;

	for (int i = 0; i < progressSteps; i++) {
		// Check for progress cancellation
		if (sceKernelPollEventFlag(evfId, PROGRESS_EVT_CANCEL, SCE_KERNEL_EVF_WAITMODE_AND, NULL) == SCE_OK) {
			cancelled = 1;
			break;
		}
		SceNotificationUtilProgressUpdateParam updateParam = {0};
		SET_TEXT(updateParam, L"Progress update");
		SET_SUB_TEXT(updateParam, L"Progress update sub text");
		updateParam.progress = (float)i / (float)progressSteps * 100.0;
		sceNotificationUtilProgressUpdate(&updateParam);
		sceKernelDelayThread(progressDelay);
	}

	// Finish progress notification
	if (cancelled) {
		SceNotificationUtilSendParam finishParam = {0};
		SET_TEXT(finishParam, L"Progress cancelled");
		sceNotificationUtilSendNotification(&finishParam);
	} else {
		SceNotificationUtilProgressFinishParam finishParam = {0};
		SET_TEXT(finishParam, L"Progress finish");
		SET_SUB_TEXT(finishParam, L"Progress finish sub text");
		sceNotificationUtilProgressFinish(&finishParam);
	}

	return sceAppMgrDestroyAppByAppId(-2);
}
