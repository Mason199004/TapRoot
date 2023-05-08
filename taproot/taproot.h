#pragma once

#include <stdint.h>
#include <pthread.h>

typedef uint64_t TapRoot_Id; //4 byte ascii identifier 4 byte int worker id

typedef struct
{
	uint32_t EventType;
	void* EventData;
} TapRoot_Event;

typedef struct
{
	TapRoot_Id ReceiverId;
	TapRoot_Event* EventQueue;
	pthread_mutex_t mutex;
	uint32_t EventCount;
} TapRoot_EventQueue;

typedef struct
{
	TapRoot_Id Self;
	TapRoot_Id* AddressableThreads;
	TapRoot_EventQueue* ThreadQueues;
	uint64_t AddressableThreadCount;
} TapRoot_ThreadGlobal;

TapRoot_ThreadGlobal* TapRoot_InitThreadGlobal(TapRoot_Id id);


TapRoot_Event TapRoot_IntoEvent(uint32_t type, void* data);

void TapRoot_PushEvent(TapRoot_Event event, TapRoot_Id destination, TapRoot_ThreadGlobal* global);

void TapRoot_GetLock(TapRoot_EventQueue* queue);

int TapRoot_MaybeGetLock(TapRoot_EventQueue* queue);

void TapRoot_Unlock(TapRoot_EventQueue* queue);

void TapRoot_ClearQueue(TapRoot_EventQueue* queue);

int32_t TapRoot_CreateWorker(TapRoot_ThreadGlobal* self, void(TapRoot_ThreadGlobal*, uint32_t, void*));

void TapRoot_DestroySelf(TapRoot_ThreadGlobal* self);


