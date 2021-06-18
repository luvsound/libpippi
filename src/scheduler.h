#ifndef LP_SCHEDULER_H
#define LP_SCHEDULER_H

#include "pippicore.h"

typedef struct lpevent_t {
    size_t id;
    lpbuffer_t * buf;
    size_t pos;
    size_t onset;
    void * next;
} lpevent_t;

typedef struct lpscheduler_t {
    lpfloat_t * current_frame;
    int channels;
    size_t now;
    size_t event_count;
    lpevent_t * waiting_queue_head;
    lpevent_t * playing_stack_head;
    lpevent_t * garbage_stack_head;
} lpscheduler_t;

typedef struct lpscheduler_factory_t {
    lpscheduler_t * (*create)(int);
    void (*tick)(lpscheduler_t *);
    int (*count_waiting)(lpscheduler_t *);
    int (*count_playing)(lpscheduler_t *);
    int (*count_done)(lpscheduler_t *);
    void (*schedule_event)(lpscheduler_t *, lpbuffer_t *, size_t);
    void (*destroy)(lpscheduler_t *);
} lpscheduler_factory_t;

extern const lpscheduler_factory_t LPScheduler;

#endif
