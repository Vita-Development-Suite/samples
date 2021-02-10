/*
	Vita Development Suite Samples
*/

/* app.cpp: Use functions, variables and classes implemented in a PRX */
#include <stdio.h>
#include <stdlib.h>
#include <sceerror.h>
#include <kernel.h>
#include <libdbg.h>
#include <moduleinfo.h>
#include "library.h"

SCE_MODULE_INFO(api_prx_basic, SCE_MODULE_ATTR_NONE, 1, 2)

int main()
{
	/*E app0 is set to the correct directory by the working path property in VSI properties */
	/*J app0はVSIプロパティ内の作業パスプロパティによって正しいディレクトリに設定されます。*/
	static const char *s_libraryPath = "app0:prx.suprx";
	double prxNumber, prxResult;
	int startResult, stopResult, unloadResult;
	SceUID moduleId;

	/*E Load the PRX into memory. This must be done before using any imported variables, functions or classes from the PRX. */
	/*E Calls module_start in the PRX with the arguments passed in sceKernelLoadStartModule. */
	/*J PRXをメモリにロードする。このロード処理はPRXからインポート済みの変数、関数、クラスを使用する前に行う必要があります。*/
	/*J sceKernelLoadStartModule に渡される引数を使って、PRX内でmodule_startを呼び出す*/
	moduleId = sceKernelLoadStartModule(s_libraryPath, 0, 0, 0, NULL, &startResult);
    SCE_DBG_ASSERT_MSG(moduleId > SCE_OK, "sceKernelLoadStartModule() returns invalid moduleId: %d \n", moduleId);

    SCE_DBG_LOG_INFO("## [api_prx_basic]: INIT SUCCEEDED ##\n");

	/*E Use a variable defined in the PRX */
	/*J PRXにおいて定義されている変数を使用*/
	prxNumber = specialNumber;
	/*E Use a function defined in the PRX */
	/*J PRXにおいて定義されている関数を使用 */
	prxResult = addNumbers(prxNumber, 2.0);
	/*E Use an exported class defined in the PRX */
	/*J PRXにおいて定義されているエクスポートクラスを使用*/
	ExportedClass prxClass;
	prxResult = prxClass.divideNumbers(prxResult, prxClass.memberVariable);
	prxResult = prxClass.subtractNumbers(14.0, prxResult);
	/*E Use a class defined in the PRX with a single exported static member */
	/*J 単一のエクスポートスタティックメンバーを使って、PRXにおいて定義されているクラスを使用*/
	prxResult = SimpleClass::negateNumber(prxResult);

	/*E Print "The result was: -10.0" */
	/*J "The result was: -10.0"をプリント*/
	SCE_DBG_LOG_INFO("The result was: %f\n", prxResult);

	/*E Unload the PRX */
	/*E Calls module_stop in the PRX with the arguments passed in sceKernelStopUnloadModule. */
	/*J PRXをアンロード*/
	/*J sceKernelStopUnloadModuleに渡される引数を使って、PRX内でmodule_stopを呼び出す*/
	unloadResult = sceKernelStopUnloadModule(moduleId, 0, 0, 0, NULL, &stopResult);
	SCE_DBG_ASSERT_MSG(unloadResult == SCE_OK, "sceKernelStopUnloadModule() returned error code: %d \n", unloadResult);

    SCE_DBG_LOG_INFO("## [api_prx_basic]: FINISHED ##\n");

	return SCE_OK;
}
