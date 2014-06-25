#if defined(OMP)
#include <omp.h>
#elif defined(WINDOWS)
#include <windows.h>
#elif defined(LINUX)
#include <unistd.h>
#elif defined(DARWIN)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

int system_is_big_endian(void)
{
    int const i = 1;
    return * (char *) &i == 0;
}

int system_core_count(void)
{
#if defined(OMP)
    return omp_get_num_procs();
#elif defined(WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    return sysinfo.dwNumberOfProcessors;
#elif defined(LINUX)
    return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(DARWIN)
    int numCPU;

    int mib[4];
    size_t len; 

    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;
    sysctl(mib, 2, &numCPU, &len, NULL, 0);

    if (numCPU < 1) 
    {
         mib[1] = HW_NCPU;
         sysctl( mib, 2, &numCPU, &len, NULL, 0);
    }

    if (numCPU < 1)
        numCPU = 1;

    return numCPU;
#else
    return 8;
#endif
}
