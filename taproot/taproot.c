#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "taproot.h"


TapRoot_ThreadGlobal* TapRoot_InitThreadGlobal(TapRoot_Id id)
{
	TapRoot_ThreadGlobal* thgl = malloc(sizeof(TapRoot_ThreadGlobal));
	memset(thgl, 0, sizeof(TapRoot_ThreadGlobal));
	thgl->Self = id;
	TapRoot_ResizeBuffer(thgl, AddressableThreads, 8);
	TapRoot_ResizeBuffer(thgl, EventQueues, 8);

	thgl->AddressableThreadGlobals[0] = thgl;
	thgl->AddressableThreads[0] = id;
	thgl->ThreadQueues[0] = (TapRoot_EventQueue){id, malloc(sizeof(TapRoot_Event) * 100), PTHREAD_MUTEX_INITIALIZER, 0};
	thgl->AddressableThreadCount = 1;
	thgl->EventQueueCount = 1;
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

int TapRoot_PushEvent(TapRoot_Event event, TapRoot_Id destination, TapRoot_ThreadGlobal* global)
{
	int i = 0;
	while (global->AddressableThreads[i] != destination && i < global->AddressableThreadCount)
	{
		i++;
	}
	if (i == global->AddressableThreadCount)
	{
		return -1;
	}

	int j = 0;
	while (global->AddressableThreadGlobals[i]->ThreadQueues[j].ReceiverId != global->Self && j < global->AddressableThreadGlobals[i]->EventQueueCount)
	{
		j++;
	}
	if (j == global->AddressableThreadGlobals[i]->EventQueueCount)
	{
		return -2;
	}

	TapRoot_GetLock(&global->AddressableThreadGlobals[i]->ThreadQueues[j]);
	if (&global->AddressableThreadGlobals[i]->EventQueueCount == &global->AddressableThreadGlobals[i]->__EvQuAlloc)
	{
		TapRoot_ResizeBuffer(global->AddressableThreadGlobals[i], EventQueues, (global->AddressableThreadGlobals[i]->__EvQuAlloc) * 2);
	}
	global->AddressableThreadGlobals[i]->ThreadQueues[j].EventQueue[global->AddressableThreadGlobals[i]->ThreadQueues[j].EventCount++] = event;
	TapRoot_Unlock(&global->AddressableThreadGlobals[i]->ThreadQueues[j]);
	return 0;
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

void TapRoot_ClearQueue(TapRoot_EventQueue* queue)
{
	queue->__iter = 0;
	queue->EventCount = 0;
}

int32_t TapRoot_CreateWorker(TapRoot_ThreadGlobal* self, void* (*func)(void* threadGlobal))
{
	TapRoot_ThreadGlobal* thgl =  malloc(sizeof(TapRoot_ThreadGlobal));
	memcpy(thgl, self, sizeof(TapRoot_ThreadGlobal));

	if (((thgl->Self + 1 ) & 0x0000FFFF) == 0)
	{
		return -1;
	}
	thgl->Self++;

	TapRoot_InsertNewThread(self, thgl);

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

void TapRoot_DestroySelf(TapRoot_ThreadGlobal* self)
{
	for (int i = 0; i < self->AddressableThreadCount; ++i)
	{
		if (self->AddressableThreads[i] == TapRoot_Invalid) continue;

		pthread_mutex_lock(&self->AddressableThreadGlobals[i]->mutex);

		for (int j = 0; j < self->AddressableThreadGlobals[i]->AddressableThreadCount; ++j) {
			if (self->AddressableThreadGlobals[i]->AddressableThreads[j] == self->Self)
			{
				self->AddressableThreadGlobals[i]->AddressableThreads[j] = TapRoot_Invalid;
				self->AddressableThreadGlobals[i]->AddressableThreadGlobals[j] = NULL;
			}
		}

		for (int j = 0; j < self->AddressableThreadGlobals[i]->EventQueueCount; ++j) {
			if (self->AddressableThreadGlobals[i]->ThreadQueues[j].ReceiverId == self->Self)
			{
				self->AddressableThreadGlobals[i]->ThreadQueues[j] = (TapRoot_EventQueue){TapRoot_Invalid};
			}
		}
		pthread_mutex_unlock(&self->AddressableThreadGlobals[i]->mutex);

	}

	free(self->AddressableThreadGlobals);
	free(self->ThreadQueues);
	free(self->AddressableThreads);
}

void TapRoot_ResizeBuffer(TapRoot_ThreadGlobal* global, TapRoot_ResizableBuffer bufferType, uint64_t newSize)
{
	switch (bufferType) {

		case AddressableThreads: {
			TapRoot_ThreadGlobal **NewGlobPtr = realloc(global->AddressableThreadGlobals,
			                                            newSize * sizeof(TapRoot_ThreadGlobal *));
			if (NewGlobPtr == NULL) {
				//TODO: Handle
			}
			if (NewGlobPtr != global->AddressableThreadGlobals) {
				global->AddressableThreadGlobals = NewGlobPtr;
			}

			TapRoot_Id *NewListPtr = realloc(global->AddressableThreads, newSize * sizeof(TapRoot_Id));
			if (NewListPtr == NULL) {
				//TODO: Handle
			}
			if (NewListPtr != global->AddressableThreads) {
				global->AddressableThreads = NewListPtr;
			}

			global->__AddrThAlloc = newSize;
			break;
		}
		case EventQueues: {
			TapRoot_EventQueue* NewQueue = realloc(global->ThreadQueues, sizeof(TapRoot_EventQueue) * newSize);
			if (NewQueue == NULL)
			{
				//TODO: Handle
			}
			global->ThreadQueues = NewQueue;

			global->__EvQuAlloc = newSize;
			break;
		}
		default:
			__builtin_unreachable();
	}
}

void TapRoot_InsertNewThread(TapRoot_ThreadGlobal* target, TapRoot_ThreadGlobal* toInsert)
{
	pthread_mutex_lock(&target->mutex);

	if (target->AddressableThreadCount == target->__AddrThAlloc)
	{
		TapRoot_ResizeBuffer(target, AddressableThreads, target->__AddrThAlloc * 2);
	}

	if (target->EventQueueCount == target->__EvQuAlloc)
	{
		TapRoot_ResizeBuffer(target, EventQueues, target->__EvQuAlloc * 2);
	}

	target->AddressableThreadGlobals[target->AddressableThreadCount] = toInsert;
	target->AddressableThreads[target->AddressableThreadCount++] = toInsert->Self;

	target->ThreadQueues[target->EventQueueCount++] = (TapRoot_EventQueue){toInsert->Self, malloc(sizeof(TapRoot_Event) * 100), PTHREAD_MUTEX_INITIALIZER, 0};


	pthread_mutex_unlock(&target->mutex);
}

TapRoot_Event* TapRoot_NextInQueue(TapRoot_EventQueue* queue)
{
	if (queue->__iter > queue->EventCount)
	{
		return NULL;
	}

	return &queue->EventQueue[queue->__iter++];
}

bool TapRoot_QueueHasEvents(TapRoot_EventQueue* queue)
{
	return queue->EventCount > 0;
}

TapRoot_EventQueue* TapRoot_GetQueue(TapRoot_ThreadGlobal* global, TapRoot_Id id)
{
	for (int i = 0; i < global->EventQueueCount; ++i)
	{
		if (global->ThreadQueues[i].ReceiverId == id)
		{
			return &global->ThreadQueues[i];
		}
	}
}
