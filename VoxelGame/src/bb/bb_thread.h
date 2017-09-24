#pragma once

#include "bb.h"

// TODO: make it multiplatform

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

struct bbThread {
#ifdef _WIN32
  HANDLE thread_handler;
  uint32_t thread_id;
#endif

  void(*function)(void*);
  void* param;
};

#ifdef _WIN32
static uint32_t __stdcall __bb_entryPoint(void* data) {
  bb_assert(data != 0);
  bbThread* thread = (bbThread*)data;
  bb_assert(thread->function != 0);
  thread->function(thread->param);

  _endthreadex(0);
  return 0;
}
#endif

static bbThread* bb_createThread(void(*function)(void*), void* param = 0) {
  bbThread* thread = (bbThread*)malloc(sizeof(bbThread));
  thread->function = function;
  thread->param = param;
#ifdef _WIN32
  thread->thread_id = 0;
  thread->thread_handler = 0;
  thread->thread_handler = (HANDLE)_beginthreadex(NULL, 0, &__bb_entryPoint, thread, 0, &thread->thread_id);
  bb_assert(thread->thread_handler != 0);
#endif
  return thread;
}

static void bb_closeThread(bbThread* thread) {
  bb_assert(thread != 0);
#ifdef _WIN32
  if (thread->thread_handler)
    CloseHandle(thread->thread_handler);
#endif

  free(thread);
}

static void bb_terminateThread(bbThread* thread) {
  bb_assert(thread != 0);

#ifdef _WIN32
  if (thread->thread_handler)
    TerminateThread(thread->thread_handler, 0);
#endif
}

static void bb_waitForThread(bbThread* thread) {
  bb_assert(thread != 0);

#ifdef _WIN32
  WaitForSingleObject(thread->thread_handler, INFINITE);
#endif
}

struct bbMutex {
#ifdef _WIN32
  HANDLE m;
#endif
};

static bbMutex bb_createMutex() {
  bbMutex mutex;
#ifdef _WIN32
  mutex.m = CreateMutex(NULL, FALSE, NULL);
#endif
  return mutex;
}

static void bb_deleteMutex(bbMutex* mutex) {
  bb_assert(mutex != 0);
#ifdef _WIN32
  CloseHandle(mutex->m);
#endif
}

static void bb_lock(bbMutex* mutex) {
  bb_assert(mutex != 0);
#ifdef _WIN32
  WaitForSingleObject(mutex->m, INFINITE);
#endif
}

static bool bb_tryLock(bbMutex* mutex) {
  bb_assert(mutex != 0);
#ifdef _WIN32
  return WaitForSingleObject(mutex->m, 0) == WAIT_OBJECT_0;
#endif
}

static void bb_unlock(bbMutex* mutex) {
  bb_assert(mutex != 0);
#ifdef _WIN32
  ReleaseMutex(mutex->m);
#endif
}
