#ifndef __BOS_TIMER_H__
#define __BOS_TIMER_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "bos.h"

typedef struct
{
	uint32_t         start_time;     /* start time (ticks) */
  uint32_t         interval;       /* interval (ticks)   */
	bool             one_shot;       /* oneshot or cyclic timer */
	uint32_t         evt;            /* event to set when expire */
	void           (*cb_func)(void); /* call back function */
  bool             start;          /* indicate status of timer  */
  uint32_t         init_time;      /* tick value at start       */  	
	bool             trigger;        /* set after trigger         */
  uint32_t         counter; 	
	struct list_head list;           /* linked list */ 	
}	bos_timer_t;

void BOS_TimerInit(bos_timer_t *timer);
void BOS_CyclicTimerStart(bos_timer_t *timer, 
	                        uint32_t start_time,
                          uint32_t interval,
                          uint32_t evt,
                          void (*cb_func)(void));

void BOS_OneShotTimerStart(bos_timer_t *timer, 
	                        uint32_t start_time,
                          uint32_t interval,
                          uint32_t evt,
                          void (*cb_func)(void));
void BOS_TimerStop(bos_timer_t *timer);

#endif /* __TIMER_H__ */


