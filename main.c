#include <stdio.h>
#include <unistd.h>
#include "taproot/taproot.h"


void* test (void* glob)
{
	TapRoot_ThreadGlobal* global = glob;
	printf("My Id is: %lx\n", global->Self);
	TapRoot_PushEvent(TapRoot_IntoEvent(0, (void*)65536), 0xDEAD0000, global);
	printf("%lx: Pushed event containing data 65536\n", global->Self);
	while (global->ThreadQueues[0].EventCount == 0)
	{

	}
	if (global->ThreadQueues[0].EventQueue[0].EventData == (void*)32)
	{
		printf("%lx: received event with data 32\n", global->Self);
		TapRoot_ClearQueue(&global->ThreadQueues[0]);
		TapRoot_DestroySelf(global);
	}
	return 0;
}


int main()
{
	TapRoot_ThreadGlobal* global = TapRoot_InitThreadGlobal(0xDEAD0000);

	TapRoot_CreateThread(0xBEEF0000, *global, test);
	while (global->ThreadQueues[1].EventCount == 0)
	{

	}

	if (global->ThreadQueues[1].EventQueue[0].EventData == (void*)65536)
	{
		printf("%lx: received event with data 65536\n", global->Self);
		TapRoot_PushEvent(TapRoot_IntoEvent(0, (void*)32), 0xBEEF0000, global);
		printf("%lx: Sent event with data 32\n", global->Self);
	}
	while (global->AddressableThreads[1] != TapRoot_Invalid)
	{

	}

	return 0;
}
