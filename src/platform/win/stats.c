#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#include <stdio.h>

int process_stats_collect(struct process_stats *out)
{
    if (!out) return -1;

    memset(out, 0, sizeof(*out));

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             &pmc,
                             sizeof(pmc))) {

        out->max_rss_mb =
            pmc.PeakWorkingSetSize / (1024.0 * 1024.0);

        out->current_rss_mb =
            pmc.WorkingSetSize / (1024.0 * 1024.0);
                             }

    FILETIME create, exit, kernel, user;

    if (GetProcessTimes(GetCurrentProcess(),
                        &create, &exit,
                        &kernel, &user)) {

        ULARGE_INTEGER k, u;
        k.LowPart  = kernel.dwLowDateTime;
        k.HighPart = kernel.dwHighDateTime;

        u.LowPart  = user.dwLowDateTime;
        u.HighPart = user.dwHighDateTime;

        /* FILETIME is 100-nanosecond units */
        out->system_cpu_sec = k.QuadPart / 10000000.0;
        out->user_cpu_sec   = u.QuadPart / 10000000.0;
                        }

    return 0;
}

void process_stats_print(void)
{
    struct process_stats stats;
    if (process_stats_collect(&stats) != 0) {
        printf("Failed to collect process stats\n");
        return;
    }

    printf("\n==== Process Stats ====\n");
    printf("Max RSS        : %.2f MB\n", stats.max_rss_mb);
    printf("Current RSS    : %.2f MB\n", stats.current_rss_mb);
    printf("User CPU       : %.3f s\n", stats.user_cpu_sec);
    printf("System CPU     : %.3f s\n", stats.system_cpu_sec);
    printf("========================\n\n");
}

#endif
