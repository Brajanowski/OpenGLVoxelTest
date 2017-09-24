#ifndef BB_FILE_H_
#define BB_FILE_H_

#include <stdio.h>
#include <unistd.h>

static bool bb_isFileExists(const char* filename) {
  if (!access(filename, F_OK)) {
    return true;
  }
  return false;
}


#endif
