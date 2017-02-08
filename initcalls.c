/*
 * initcalls.c
 *
 *  Created on: 06.01.2017
 *      Author: sefo
 */
#include <init.h>

void do_initcalls(void)
{
	extern initcall_t __initcalls_end[];
	extern initcall_t __initcalls_start[];

	int no_of_calls = __initcalls_end - __initcalls_start;
	initcall_t* calls = __initcalls_start;

	for (int i = 0; i < no_of_calls; ++i) {
		initcall_t call = calls[i];
		call();
	}
}
