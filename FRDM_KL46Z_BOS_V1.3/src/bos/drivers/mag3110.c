#include "i2c.h"
#include "mag3110.h"

void mag3110_write(uint8_t addr, uint8_t data)
{
    i2c_delay();
    i2c_start(I2C0);
    i2c_write(I2C0, MAG3110_I2C_ADDR | I2C_WRITE);
    i2c_write(I2C0, addr);
    i2c_write(I2C0, data);
    i2c_stop(I2C0);
}

uint8_t mag3110_read(uint8_t addr)
{
    i2c_delay();
    i2c_start(I2C0);
    i2c_write(I2C0, MAG3110_I2C_ADDR | I2C_WRITE);
    i2c_write(I2C0, addr);
    i2c_repeated_start(I2C0);
    i2c_write(I2C0, MAG3110_I2C_ADDR | I2C_READ);
    i2c_set_rx(I2C0);
    i2c_give_nack(I2C0);
    i2c_read(I2C0);
    i2c_wait(I2C0);
    i2c_stop(I2C0);
    return i2c_read(I2C0);
}

void mag3110_init(void)
{

    uint8_t deviceid;

    /* read sensor ID */
    deviceid = mag3110_read(0x07); 

    /* check available sensor */
    if (deviceid != 0xC4)
    {
        printf("mag3110: Device not found\r\n");
    }

    //CTRL_REG1
    //DR2|DR1|DR0|OS1|OS0|FastRead|Trigger|ActiveMode|
    // 0 | 1 | 1 | 1 | 1 |    0   |   0   |    1     |
    mag3110_write(0x10, 0x79);
    i2c_delay();
    i2c_delay();
  
    //CTRL_REG2: 
    //AutoMagRst|---|Raw|Mag_Rst|---|---|---|---|
    //    1     | 0 | 0 |   0   | 0 | 0 | 0 | 0 |
    mag3110_write(0x11, 0x80);
}

// Read a signed 14-bit value from (reg, reg+1)
int16_t mag3110_read_reg16(uint8_t reg)
{
    return (int16_t)((mag3110_read(reg) << 8) | mag3110_read(reg + 1));
}

// Read acceleration values for each axis
int16_t mag3110_x(void) {return mag3110_read_reg16(0x01);}
int16_t mag3110_y(void) {return mag3110_read_reg16(0x03);}
int16_t mag3110_z(void) {return mag3110_read_reg16(0x05);}
