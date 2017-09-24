#pragma once

#include "bb_thread.h"

// | MemoryInfo | user allocated memory |
struct bbMemoryInfo {
  char* label;
  uint32_t size;

  bbMemoryInfo* prev, *next;
};

static bbMemoryInfo* memlist_head = 0;
static bbMutex mem_mutex;

static void bb_memInit() {
  mem_mutex = bb_createMutex();
}

static void bb_freeMem() {
  bb_deleteMutex(&mem_mutex);

}

static void* bb_malloc(uint32_t size, char* label) {
  bb_assert(size != 0);
  bb_assert(label != 0);

  bbMemoryInfo* m = (bbMemoryInfo*)malloc(size + sizeof(bbMemoryInfo));
  m->size = size;
  m->label = label;
  m->prev = 0;
  m->next = 0;

  bb_lock(&mem_mutex);

  if (memlist_head == 0)
    memlist_head = m;
  else {
    memlist_head->prev = m;
    m->next = memlist_head;
    memlist_head = m;
  }
    
  bb_unlock(&mem_mutex);
  return m + 1;
}

static void bb_free(void* p) {
  bbMemoryInfo* m = ((bbMemoryInfo*)p) - 1;

  bb_lock(&mem_mutex);

  if (m->prev == 0) {
    memlist_head = m->next;
  } else {
    m->prev->next = m->next;
  }

  if (m->next != 0) {
    m->next->prev = m->prev;
  }

  free(m);

  bb_unlock(&mem_mutex);
}
