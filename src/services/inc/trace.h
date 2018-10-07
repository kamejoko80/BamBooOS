#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdio.h>
#include <stdarg.h>
#include "bos.h"

void SysTrace_Init(void);
uint32_t SysTrace_GetTime(char *TimeStr);
void SysTrace_Message(char *Format, ...);
void SysTrace_PrintOut(void);
void SysTrace_ManageMsgQueueEvent(void);

#endif /* __MESSAGE_H__ */
