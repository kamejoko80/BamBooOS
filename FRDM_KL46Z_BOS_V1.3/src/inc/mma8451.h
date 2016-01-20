#ifndef __MMA8451_H__
#define __MMA8451_H__

#include <stdio.h>
#include <string.h>
#include "MKL46Z4.h"
#include "i2c.h"

#define MMA8451_I2C_ADDRESS (0x1d << 1)
#define CTRL_REG1           (0x2a)

#define MMA8451_REG_OUT_X_MSB     0x01
#define MMA8451_REG_SYSMOD        0x0B
#define MMA8451_REG_WHOAMI        0x0D
#define MMA8451_REG_XYZ_DATA_CFG  0x0E
#define MMA8451_REG_PL_STATUS     0x10
#define MMA8451_REG_PL_CFG        0x11
#define MMA8451_REG_CTRL_REG1     0x2A
#define MMA8451_REG_CTRL_REG2     0x2B
#define MMA8451_REG_CTRL_REG4     0x2D
#define MMA8451_REG_CTRL_REG5     0x2E

#define MMA8451_PL_PUF            0
#define MMA8451_PL_PUB            1
#define MMA8451_PL_PDF            2
#define MMA8451_PL_PDB            3
#define MMA8451_PL_LRF            4  
#define MMA8451_PL_LRB            5  
#define MMA8451_PL_LLF            6  
#define MMA8451_PL_LLB            7  

#define MMA8451_RANGE_8_G         0x02  // +/- 8g
#define MMA8451_RANGE_4_G         0x01  // +/- 4g
#define MMA8451_RANGE_2_G         0x00  // +/- 2g (default value)

/* Used with register 0x2A (MMA8451_REG_CTRL_REG1) to set bandwidth */

#define MMA8451_DATARATE_800_HZ   0x00 //  400Hz 
#define MMA8451_DATARATE_400_HZ   0x01 //  200Hz
#define MMA8451_DATARATE_200_HZ   0x02 //  100Hz
#define MMA8451_DATARATE_100_HZ   0x03 //   50Hz
#define MMA8451_DATARATE_50_HZ    0x04 //   25Hz
#define MMA8451_DATARATE_12_5_HZ  0x05 // 6.25Hz
#define MMA8451_DATARATE_6_25HZ   0x06 // 3.13Hz
#define MMA8451_DATARATE_1_56_HZ  0x07 // 1.56Hz

void mma8451_init(void);
int16_t mma8451_x(void); 
int16_t mma8451_y(void);
int16_t mma8451_z(void);

#endif /* __MMA8451_H__ */
