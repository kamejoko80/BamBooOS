#include "i2c.h"
#include "mma8451.h"

uint8_t mma8451_read(uint8_t addr)
{
	  i2c_delay();
    i2c_start(I2C0);
    i2c_write(I2C0, MMA8451_I2C_ADDRESS | I2C_WRITE);
    i2c_write(I2C0, addr);
    i2c_repeated_start(I2C0);
    i2c_write(I2C0, MMA8451_I2C_ADDRESS | I2C_READ);
    i2c_set_rx(I2C0);
    i2c_give_nack(I2C0);
    i2c_read(I2C0);
    i2c_wait(I2C0);
    i2c_stop(I2C0);
    return i2c_read(I2C0);
}

void mma8451_write(uint8_t addr, uint8_t data)
{
	  i2c_delay();
    i2c_start(I2C0);
    i2c_write(I2C0, MMA8451_I2C_ADDRESS | I2C_WRITE);
    i2c_write(I2C0, addr);
    i2c_write(I2C0, data);
    i2c_stop(I2C0);
}


void mma8451_init(void)
{
    uint8_t deviceid;
	
	  /* read sensor ID */
	  deviceid = mma8451_read(MMA8451_REG_WHOAMI); 

	  /* check available sensor */
		if (deviceid != 0x1A)
		{
			printf("mma8451: Device not found\r\n");
		}	
	
		/* reset the sensor */
		mma8451_write(MMA8451_REG_CTRL_REG2, 0x40);
		
		/* wait for reset complete */
    while (mma8451_read(MMA8451_REG_CTRL_REG2) & 0x40); 
	
		/* enable 4G range */
		mma8451_write(MMA8451_REG_XYZ_DATA_CFG, MMA8451_RANGE_4_G);
		/* High res */
		mma8451_write(MMA8451_REG_CTRL_REG2, 0x02);
		/* Low noise */
		mma8451_write(MMA8451_REG_CTRL_REG4, 0x01);
		/* DRDY on INT1 */
		mma8451_write(MMA8451_REG_CTRL_REG4, 0x01);
		mma8451_write(MMA8451_REG_CTRL_REG5, 0x01);

		/* Turn on orientation config */
		mma8451_write(MMA8451_REG_PL_CFG, 0x40);

		/* Activate */
		mma8451_write(MMA8451_REG_CTRL_REG1, 0x01); // active, max rate			
}

// Read a signed 14-bit value from (reg, reg+1)
int16_t _read_reg14(uint8_t reg)
{
    return (int16_t)((mma8451_read(reg) << 8) | mma8451_read(reg + 1)) >> 2;
}

// Read acceleration values for each axis
int16_t mma8451_x(void) {return _read_reg14(0x01);}
int16_t mma8451_y(void) {return _read_reg14(0x03);}
int16_t mma8451_z(void) {return _read_reg14(0x05);}
