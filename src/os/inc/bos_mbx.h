#ifndef __BOS_MBX_H__
#define __BOS_MBX_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "bos.h"

#define MBX_INI   0x00000000 /* initial state    */
#define MBX_POS   0x00000001 /* posted status    */
#define MBX_GET   0x00000002 /* already gotten   */

typedef struct
{
    uint8_t   *data;         /* data buffer      */
    uint32_t  len;           /* data length      */
    uint32_t  sender_id;     /* sender task id   */
    uint32_t  receiver_id;   /* receiver task id */
    uint32_t  flag;          /* mailbox status   */
    struct list_head list;   /* linked list      */
}bos_mbx_t;

#endif /* __BOS_MBX_H__ */
