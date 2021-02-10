/*
	Vita Development Suite Samples
*/

/* library.cpp: Defines the functions, variables and classes for the PRX */

/*E Signal to the header file that this compilation is for the PRX */
/*J このコンパイルが特定のPRX向けであることを示すヘッダーファイルへのシグナル*/
#define LIBRARY_IMPL  (1)
#include <stdio.h>
#include <kernel.h>
#include <moduleinfo.h>
#include "library.h"

SCE_MODULE_INFO(library, SCE_MODULE_ATTR_NONE, 1, 2)

/*E These functions have special meaning and are optional. */
/*J これらの関数は特別な意味を持つオプションの関数です。*/
extern "C"
{
	/*E This function is called automatically when sceKernelLoadStartModule is called */
	/*J この関数は、sceKernelLoadStartModuleが呼び出されると自動的に呼び出されます。*/
	int module_start(SceSize args, const void *argp)
	{
		(void)args; (void)argp;
		specialNumber = 8.0;
		printf("module_start called in sub-module\n");
		return SCE_KERNEL_START_SUCCESS;
	}

	/*E This function is called automatically when sceKernelStopUnloadModule is called */
	/*J この関数はsceKernelStopUnloadModuleが呼び出されると自動的に呼び出されます。*/
	int module_stop(SceSize args, const void *argp)
	{
		(void)args; (void)argp;
		printf("module_stop called in sub-module\n");
		return SCE_KERNEL_STOP_SUCCESS;
	}

	/*E This function is called automatically if the module has not been unloaded when the process exits. */
	/*E In this sample, this function shouldn't be called as the module is unloaded */
	/*J この関数は、プロセスの終了時にモジュールがアンロードされていないと自動的に呼び出されます。*/
	/*J このサンプルにおいては、モジュールがアンロードされてない場合はこの関数を呼び出さないでください。*/
	void module_exit()
	{
		printf("module_exit called in sub-module\n");
	}
}

/*E Implementation of exported functions */
/*J エクスポートした関数の実装*/

PRX_INTERFACE double addNumbers(double a, double b)
{
	return a + b;
}

PRX_INTERFACE double ExportedClass::divideNumbers(double a, double b)
{
	return a / b;
}

PRX_INTERFACE double ExportedClass::subtractNumbers(double a, double b)
{
	return a - b;
}

PRX_INTERFACE double SimpleClass::negateNumber(double a)
{
	return -a;
}

void SimpleClass::notExported()
{

}
