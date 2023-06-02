#include <stdio.h>
#include <unistd.h>
#include "../taproot/taproot.h"


void* test (void* glob)
{
	TapRoot_ThreadGlobal* global = glob;
	printf("My Id is: %llx\n", global->Self);
	TapRoot_PushEvent(TapRoot_IntoEvent(0, (void*)65536), 0xDEAD0000, global);
	printf("%llx: Pushed event containing data 65536\n", global->Self);
	while (!TapRoot_QueueHasEvents(&global->ThreadQueues[0]))
	{

	}
	TapRoot_GetLock(&global->ThreadQueues[0]);
	if (TapRoot_NextInQueue(&global->ThreadQueues[0])->EventData == (void*)32)
	{
		printf("%llx: received event with data 32\n", global->Self);
		TapRoot_ClearQueue(&global->ThreadQueues[0]);
		TapRoot_DestroySelf(global);
	}
	return 0;
}


int main()
{
	TapRoot_ThreadGlobal* global = TapRoot_InitThreadGlobal(0xDEAD0000);

	TapRoot_CreateThread(0xBEEF0000, *global, test);
	while (!TapRoot_QueueHasEvents(&global->ThreadQueues[1]))
	{

	}
	TapRoot_GetLock(&global->ThreadQueues[1]);
	if (TapRoot_NextInQueue(&global->ThreadQueues[1])->EventData == (void*)65536)
	{
		printf("%llx: received event with data 65536\n", global->Self);
		TapRoot_PushEvent(TapRoot_IntoEvent(0, (void*)32), 0xBEEF0000, global);
		printf("%llx: Sent event with data 32\n", global->Self);
	}
	TapRoot_ClearQueue(&global->ThreadQueues[1]);
	TapRoot_Unlock(&global->ThreadQueues[1]);

	while (global->AddressableThreads[1] != TapRoot_Invalid)
	{

	}

	return 0;
}
