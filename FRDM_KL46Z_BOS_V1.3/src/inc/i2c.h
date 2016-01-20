#ifndef __I2C_H__
#define __I2C_H__

#include <stdio.h>
#include <string.h>
#include "MKL46Z4.h"

#define I2C_READ   1
#define I2C_WRITE  0

void i2c_delay(void);
void i2c_start(I2C_Type *p);
void i2c_stop(I2C_Type *p);
void i2c_wait(I2C_Type *p);
int i2c_write(I2C_Type *p, uint8_t data);
void i2c_init(I2C_Type *p);

__inline void i2c_set_tx(I2C_Type *p)         { p->C1 |= I2C_C1_TX_MASK;    }
__inline void i2c_set_rx(I2C_Type *p)         { p->C1 &= ~I2C_C1_TX_MASK;   }
__inline void i2c_set_slave(I2C_Type *p)      { p->C1 &= ~I2C_C1_MST_MASK;  }
__inline void i2c_set_master(I2C_Type *p)     { p->C1 |=  I2C_C1_MST_MASK;  }
__inline void i2c_give_nack(I2C_Type *p)      { p->C1 |= I2C_C1_TXAK_MASK;  }
__inline void i2c_give_ack(I2C_Type *p)       { p->C1 &= ~I2C_C1_TXAK_MASK; }
__inline void i2c_repeated_start(I2C_Type *p) { p->C1 |= I2C_C1_RSTA_MASK;  }
__inline uint8_t i2c_read(I2C_Type *p)        { return p->D; }

#endif /* __I2C_H__ */
