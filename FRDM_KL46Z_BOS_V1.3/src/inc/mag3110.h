#ifndef __MAG3110_H__
#define __MAG3110_H__

#include <stdio.h>
#include <string.h>
#include "MKL46Z4.h"
#include "i2c.h"

#define MAG3110_I2C_ADDR  (0x0E << 1) // Magnetometer MAG3110

void mag3110_init(void);
int16_t mag3110_x(void);
int16_t mag3110_y(void);
int16_t mag3110_z(void);

#endif /* __MAG3110_H__ */
