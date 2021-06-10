#ifndef LP_SCHEDULER_H
#define LP_SCHEDULER_H

#include "pippicore.h"

/* forward declaration */
typedef struct event_t {
    size_t id;
    buffer_t * buf;
    size_t pos;
    size_t onset;
    void * next;
} event_t;

typedef struct scheduler_t {
    lpfloat_t * current_frame;
    int channels;
    size_t now;
    size_t event_count;
    event_t * waiting_queue_head;
    event_t * playing_stack_head;
    event_t * garbage_stack_head;
} scheduler_t;

typedef struct scheduler_factory_t {
    scheduler_t * (*create)(int);
    void (*tick)(scheduler_t *);
    int (*count_waiting)(scheduler_t *);
    int (*count_playing)(scheduler_t *);
    int (*count_done)(scheduler_t *);
    void (*schedule_event)(scheduler_t *, buffer_t *, size_t);
    void (*destroy)(scheduler_t *);
} scheduler_factory_t;

extern const scheduler_factory_t Scheduler;

#endif
