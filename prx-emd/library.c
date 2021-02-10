/*
	Vita Development Suite Samples
*/

/* library.cpp: Defines the functions, variables and classes for the PRX */

#include <stdio.h>
#include <kernel.h>
#include <moduleinfo.h>
#include "library.h"

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

/*E Definition of exported variables */
/*J エクスポートした変数の定義*/

double specialNumber;

/*E Implementation of exported functions */
/*J エクスポートした関数の実装*/

double addNumbers(double a, double b)
{
	return a + b;
}
