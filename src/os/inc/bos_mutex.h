#ifndef __BOS_MUTEX_H__
#define __BOS_MUTEX_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "bos_list.h"

#define MAX_SUSPEND_TASK (16)

#define MUTEX_CREATE(mutex) bos_mutex_t mutex = {\
                                            .locked_by_task_id = 0x00,\
                                            .suspend_task_id = {0},\
                                            .locked = false,\
                                        };

typedef struct
{
    uint8_t locked_by_task_id;                  /* task id that taken the lock                   */
    uint8_t suspend_task_id[MAX_SUSPEND_TASK];  /* list task id which are suspended by the mutex */
    bool  locked;                               /* locked status flag                            */
    struct list_head list;                      /* linked list                                   */
}bos_mutex_t;


void BOS_MutexLock(bos_mutex_t *mutex);
void BOS_MutexUnLock(bos_mutex_t *mutex);

#endif /* __BOS_MUTEX_H__ */


