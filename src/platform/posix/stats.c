#include "puremvc/platform.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

#if defined(__linux__)
#include <stdlib.h>
#endif

int process_stats_collect(struct process_stats *out) {
    if (!out) return -1;

    memset(out, 0, sizeof(*out));

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0)
        return -1;

#if defined(__APPLE__)
    /* macOS: ru_maxrss is bytes */
    out->max_rss_mb = usage.ru_maxrss / (1024.0 * 1024.0);
#elif defined(__linux__)
    /* Linux: ru_maxrss is KB */
    out->max_rss_mb = usage.ru_maxrss / 1024.0;
#endif

    out->user_cpu_sec =
        usage.ru_utime.tv_sec +
        usage.ru_utime.tv_usec / 1000000.0;

    out->system_cpu_sec =
        usage.ru_stime.tv_sec +
        usage.ru_stime.tv_usec / 1000000.0;

    out->minor_page_faults = usage.ru_minflt;
    out->major_page_faults = usage.ru_majflt;
    out->voluntary_ctx_switches = usage.ru_nvcsw;
    out->involuntary_ctx_switches = usage.ru_nivcsw;

#if defined(__linux__)
    /* Current RSS from /proc */
    FILE *fp = fopen("/proc/self/statm", "r");
    if (fp) {
        long size = 0, resident = 0;
        if (fscanf(fp, "%ld %ld", &size, &resident) == 2) {
            long page_size = sysconf(_SC_PAGESIZE);
            out->current_rss_mb =
                (resident * page_size) / (1024.0 * 1024.0);
        }
        fclose(fp);
    }
#endif

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
    if (stats.current_rss_mb > 0.0)
        printf("Current RSS    : %.2f MB\n", stats.current_rss_mb);

    printf("User CPU       : %.3f s\n", stats.user_cpu_sec);
    printf("System CPU     : %.3f s\n", stats.system_cpu_sec);

    printf("Minor faults   : %ld\n", stats.minor_page_faults);
    printf("Major faults   : %ld\n", stats.major_page_faults);

    printf("Voluntary CS   : %ld\n", stats.voluntary_ctx_switches);
    printf("Involuntary CS : %ld\n", stats.involuntary_ctx_switches);
    printf("========================\n\n");
}