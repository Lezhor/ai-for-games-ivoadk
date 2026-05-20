#include "utils/time_utils.h"
#include <sys/time.h>
#include <stddef.h>

uint64_t time_get_now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
