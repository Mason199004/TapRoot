#pragma once

#include <stdint.h>
#include <stdbool.h>
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
	uint32_t __iter;
} TapRoot_EventQueue;

typedef struct TapRoot_ThreadGlobal
{
       TapRoot_Id Self;
       TapRoot_Id* AddressableThreads;
struct TapRoot_ThreadGlobal** AddressableThreadGlobals;
       TapRoot_EventQueue* ThreadQueues;
	   pthread_mutex_t mutex;
       uint64_t AddressableThreadCount;
       uint64_t EventQueueCount;
	   uint64_t __AddrThAlloc;
	   uint64_t __EvQuAlloc;
} TapRoot_ThreadGlobal;

#define TapRoot_Invalid UINT64_MAX

TapRoot_ThreadGlobal* TapRoot_InitThreadGlobal(TapRoot_Id id);

int TapRoot_CreateThread(TapRoot_Id NewThreadId, TapRoot_ThreadGlobal inherit, void* (*func)(void* threadGlobal) );

TapRoot_Event TapRoot_IntoEvent(uint32_t type, void* data);

int TapRoot_PushEvent(TapRoot_Event event, TapRoot_Id destination, TapRoot_ThreadGlobal* global);

void TapRoot_GetLock(TapRoot_EventQueue* queue);

int TapRoot_MaybeGetLock(TapRoot_EventQueue* queue);

void TapRoot_Unlock(TapRoot_EventQueue* queue);

void TapRoot_ClearQueue(TapRoot_EventQueue* queue);

int32_t TapRoot_CreateWorker(TapRoot_ThreadGlobal* self, void* (*func)(void* threadGlobal));

void TapRoot_DestroySelf(TapRoot_ThreadGlobal* self);

typedef enum
{
	AddressableThreads,
	EventQueues
} TapRoot_ResizableBuffer;

void TapRoot_ResizeBuffer(TapRoot_ThreadGlobal* global, TapRoot_ResizableBuffer bufferType, uint64_t newSize);

void TapRoot_InsertNewThread(TapRoot_ThreadGlobal* target, TapRoot_ThreadGlobal* toInsert);

TapRoot_Event* TapRoot_NextInQueue(TapRoot_EventQueue* queue); //read then inc

bool TapRoot_QueueHasEvents(TapRoot_EventQueue* queue);

TapRoot_EventQueue* GetQueue(TapRoot_ThreadGlobal* global, TapRoot_Id id);
