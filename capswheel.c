/*
 * capswheel.c
 *
 * by buzzert <buzzert@buzzert.net> 30 Jun 2019
 *
 * Run with intercept-tools:
 * intercept -g $MOUSE | capswheel | uinput -d $MOUSE
 *
 * where $MOUSE is the trackpoint/mouse path in /dev/input
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SCROLL_MODIFIER_KEY KEY_CAPSLOCK
#define SCROLL_ACCUMULATION_AMOUNT 20

static bool __modifier_held = false;

typedef enum {
    DIRECTION_X,
    DIRECTION_Y
} scroll_direction_t;

static inline void
send_event(uint16_t type, uint16_t code, int32_t value)
{
    struct input_event event = {
        .type = type,
        .code = code,
        .value = value
    };

    fwrite(&event, sizeof(event), 1, stdout);
}

void send_wheel_event(int amount, scroll_direction_t direction)
{
    static uint32_t accumulator;

    accumulator += abs(amount);
    if (accumulator < SCROLL_ACCUMULATION_AMOUNT) return;

    // Reset accumulator
    accumulator = 0;

    uint16_t code = REL_WHEEL;
    uint16_t hires_code = REL_WHEEL_HI_RES;
    if (direction == DIRECTION_X) {
        code = REL_HWHEEL;
        hires_code = REL_HWHEEL_HI_RES;
    }

    // Send mousewheel event
    int normalized = (amount < 0) ? 1 : -1;
    send_event(EV_REL, code, normalized);

    // Send hires wheel event
    send_event(EV_REL, hires_code, amount);

    // Send SYN event separator
    send_event(EV_SYN, 0, 0);
}

void* keyboard_watch_loop(void *userdata)
{
    const char *keyboard_path = (const char *)userdata;
    FILE *kbfd = fopen(keyboard_path, "r");
    if (!kbfd) {
        fprintf(stderr, "Unable to open keyboard: %s\n", strerror(errno));
        exit(1);
    }

    struct input_event event;
    while ( fread(&event, sizeof(event), 1, kbfd) == 1 ) {
        if (event.type != EV_KEY) continue;
        if (event.code != SCROLL_MODIFIER_KEY) continue;

        __modifier_held = event.value;
    }

    return NULL;
}

void read_loop(void)
{
    struct input_event event;
    while ( fread(&event, sizeof(event), 1, stdin) == 1 ) {
        if (!__modifier_held) {
            fwrite(&event, sizeof(event), 1, stdout);
            continue;
        }

        // Watch for mouse events
        if (event.type != EV_REL) continue;
        if (event.code != REL_Y && event.code != REL_X) continue;

        scroll_direction_t direction = DIRECTION_Y;
        if (event.code == REL_X) {
            direction = DIRECTION_X;
        }

        int32_t value = event.value;
        send_wheel_event(value, direction);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [keyboard_input_device]\n", argv[0]);
        exit(1);
    }

    struct stat statbuf;
    char *keyboard_path = argv[1];
    if (stat(keyboard_path, &statbuf) != 0) {
        fprintf(stderr, "Unable to read keyboard input at path: %s\n", keyboard_path);
        exit(1);
    }

    // No buffering on stdin/stdout
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // Create keyboard listening thread
    pthread_t keyboard_thread;
    pthread_create(&keyboard_thread, NULL, keyboard_watch_loop, keyboard_path);

    // Start watching for mouse events
    read_loop();

    return 0;
}
