#ifndef RECEIVER_CLOCK_H
#define RECEIVER_CLOCK_H
#include <stdint.h>
#include <esp_timer.h>

// Milliseconds since boot (wraps after 49 days; compare with subtraction)
inline uint32_t nowMs() {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

#endif
