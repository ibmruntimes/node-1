// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/base/sys-info.h"

#if V8_OS_POSIX
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#if !V8_OS_FUCHSIA
#include <sys/resource.h>
#endif
#endif

#if V8_OS_BSD
#include <sys/sysctl.h>
#endif

#include <limits>

#include "src/base/logging.h"
#include "src/base/macros.h"
#if V8_OS_WIN
#include "src/base/win32-headers.h"
#endif
#if V8_OS_ZOS
#include "zos.h"
#endif

namespace v8 {
namespace base {

// static
int SysInfo::NumberOfProcessors() {
#if V8_OS_OPENBSD
  int mib[2] = {CTL_HW, HW_NCPU};
  int ncpu = 0;
  size_t len = sizeof(ncpu);
  if (sysctl(mib, arraysize(mib), &ncpu, &len, nullptr, 0) != 0) {
    return 1;
  }
  return ncpu;
#elif V8_OS_ZOS
  ZOSCVT* __ptr32 cvt = ((ZOSPSA*)0)->cvt;
  ZOSRMCT* __ptr32 rmct = cvt->rmct;
  ZOSCCT* __ptr32 cct = rmct->cct;
  return static_cast<int>(cct->cpuCount);
#elif V8_OS_POSIX
  long result = sysconf(_SC_NPROCESSORS_ONLN);  // NOLINT(runtime/int)
  if (result == -1) {
    return 1;
  }
  return static_cast<int>(result);
#elif V8_OS_WIN
  SYSTEM_INFO system_info = {};
  ::GetNativeSystemInfo(&system_info);
  return static_cast<int>(system_info.dwNumberOfProcessors);
#endif
}


// static
int64_t SysInfo::AmountOfPhysicalMemory() {
#if V8_OS_MACOSX
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  int64_t memsize = 0;
  size_t len = sizeof(memsize);
  if (sysctl(mib, arraysize(mib), &memsize, &len, nullptr, 0) != 0) {
    return 0;
  }
  return memsize;
#elif V8_OS_FREEBSD
  int pages, page_size;
  size_t size = sizeof(pages);
  sysctlbyname("vm.stats.vm.v_page_count", &pages, &size, nullptr, 0);
  sysctlbyname("vm.stats.vm.v_page_size", &page_size, &size, nullptr, 0);
  if (pages == -1 || page_size == -1) {
    return 0;
  }
  return static_cast<int64_t>(pages) * page_size;
#elif V8_OS_CYGWIN || V8_OS_WIN
  MEMORYSTATUSEX memory_info;
  memory_info.dwLength = sizeof(memory_info);
  if (!GlobalMemoryStatusEx(&memory_info)) {
    return 0;
  }
  int64_t result = static_cast<int64_t>(memory_info.ullTotalPhys);
  if (result < 0) result = std::numeric_limits<int64_t>::max();
  return result;
#elif V8_OS_QNX
  struct stat stat_buf;
  if (stat("/proc", &stat_buf) != 0) {
    return 0;
  }
  return static_cast<int64_t>(stat_buf.st_size);
#elif V8_OS_AIX
  int64_t result = sysconf(_SC_AIX_REALMEM);
  return static_cast<int64_t>(result) * 1024L;
#elif V8_OS_ZOS
  ZOSCVT * __ptr32 cvt = ((ZOSPSA*)0)->cvt;
  ZOSRCE * __ptr32 rce = cvt->rce;
  intptr_t pages = rce->pool;
  intptr_t page_size = sysconf(_SC_PAGESIZE);
  return static_cast<uint64_t>(pages) * page_size;
#elif V8_OS_POSIX
  long pages = sysconf(_SC_PHYS_PAGES);    // NOLINT(runtime/int)
  long page_size = sysconf(_SC_PAGESIZE);  // NOLINT(runtime/int)
  if (pages == -1 || page_size == -1) {
    return 0;
  }
  return static_cast<int64_t>(pages) * page_size;
#endif
}


// static
int64_t SysInfo::AmountOfVirtualMemory() {
#if V8_OS_WIN || V8_OS_FUCHSIA
  return 0;
#elif V8_OS_POSIX
  struct rlimit rlim;
  int result = getrlimit(RLIMIT_DATA, &rlim);
  if (result != 0) {
    return 0;
  }
  return (rlim.rlim_cur == RLIM_INFINITY) ? 0 : rlim.rlim_cur;
#endif
}

}  // namespace base
}  // namespace v8
