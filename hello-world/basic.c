/*
	Vita Development Suite Samples
*/

#include <stdio.h>
#include <kernel.h>

/*J ユーザメインスレッドパラメータ */
/*E User main thread parameters */
const char		sce_user_main_thread_name[]		= "basic_main_thr";
int				sce_user_main_thread_priority	= SCE_KERNEL_DEFAULT_PRIORITY_USER;
unsigned int	sce_user_main_thread_stack_size	= SCE_KERNEL_STACK_SIZE_DEFAULT_USER_MAIN;

int main(void)
{
	printf("## hello_world: INIT SUCCEEDED ##\n");

	printf("Hello, PSP2 World!\n");

	printf("## hello_world: FINISHED ##\n");

	return 0;
}
