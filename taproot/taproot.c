#include <stdlib.h>
#include <string.h>
#include "taproot.h"


TapRoot_ThreadGlobal* TapRoot_InitThreadGlobal(TapRoot_Id id)
{
	TapRoot_ThreadGlobal* thgl = malloc(sizeof(TapRoot_ThreadGlobal));
	memset(thgl, 0, sizeof(TapRoot_ThreadGlobal));
	thgl->Self = id;
	TapRoot_ResizeBuffer(thgl, AddressableThreads, 8);
	TapRoot_ResizeBuffer(thgl, EventQueues, 8);
	return thgl;
}

int TapRoot_CreateThread(TapRoot_Id NewThreadId, TapRoot_ThreadGlobal inherit, void* (*func)(void* threadGlobal) )
{
	TapRoot_ThreadGlobal* thgl = malloc(sizeof(TapRoot_ThreadGlobal));
	*thgl = inherit;
	thgl->Self = NewThreadId;

	for (int i = 0; i < thgl->AddressableThreadCount; ++i)
	{
		if (thgl->AddressableThreads[i] == TapRoot_Invalid) continue;

		TapRoot_InsertNewThread(thgl->AddressableThreadGlobals[i], thgl);
	}

	pthread_t p;
	int thread = pthread_create(&p, NULL, func, thgl);
	if (thread) //not good
	{
		TapRoot_DestroySelf(thgl); /* not efficient error handling but can prevent issues caused by the target thread spawning immediately
		                                  and not being addressable or receivable by other threads */
		return -1;
	}

	return 0;
}

TapRoot_Event TapRoot_IntoEvent(uint32_t type, void* data)
{
	TapRoot_Event event;
	event.EventType = type;
	event.EventData = data;
	return event;
}

void TapRoot_PushEvent(TapRoot_Event event, TapRoot_Id destination, TapRoot_ThreadGlobal* global)
{
	
}

void TapRoot_GetLock(TapRoot_EventQueue* queue)
{
	pthread_mutex_lock(&queue->mutex);
}

int TapRoot_MaybeGetLock(TapRoot_EventQueue* queue)
{
	return pthread_mutex_trylock(&queue->mutex);
}

void TapRoot_Unlock(TapRoot_EventQueue* queue)
{
	pthread_mutex_unlock(&queue->mutex);
}

void TapRoot_ClearQueue(TapRoot_EventQueue* queue);

int32_t TapRoot_CreateWorker(TapRoot_ThreadGlobal* self, void* (*func)(void* threadGlobal));

void TapRoot_DestroySelf(TapRoot_ThreadGlobal* self);

void TapRoot_ResizeBuffer(TapRoot_ThreadGlobal* global, TapRoot_ResizableBuffer bufferType, uint64_t newSize);

void TapRoot_InsertNewThread(TapRoot_ThreadGlobal* target, TapRoot_ThreadGlobal* toInsert);
