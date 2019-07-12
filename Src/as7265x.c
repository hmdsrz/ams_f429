/**
 * AS7265x triad spectroscopic sensor I2C library.
 * 
 * See https://ams.com/as7265x for sensor datasheet.

 * Most recent code from:
 * https://github.com/jdesbonnet/as7265x 
 */

#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>

//#include "i2c.h"
#include "as7265x.h"

#include "stm32f4xx_hal.h"
#include "main.h"

#define AS7265X_I2C_ADDR 0x49

I2C_HandleTypeDef hi2c2;

/**
 * Read a I2C (real) register from AS7265x
 */
uint8_t i2cm_read(I2C_HandleTypeDef *hi2c, uint8_t addr)
    {
	uint8_t result;
	HAL_I2C_Mem_Read(hi2c, (uint16_t)(0x49 << 1) ,addr, I2C_MEMADD_SIZE_8BIT, &result , sizeof(result) , 100);
	return result;
    }


/**
 * Write a I2C (real) register to AS7265x.
 */
void i2cm_write(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t value)
    {

	HAL_I2C_Mem_Write(hi2c, (uint16_t)(0x49 << 1) , addr, I2C_MEMADD_SIZE_8BIT, &value, sizeof(value) , 100); //AS7265X_I2C_ADDR
    }

/**
 * Write to AS7265x virtual register. Based on code in the AS7265x datasheet.
 */
void as7265x_vreg_write(I2C_HandleTypeDef *hi2c, uint8_t virtualReg, uint8_t d)
{
	volatile uint8_t status;
	while (1)
	{
		// Read slave I²C status to see if the write buffer is ready.
		status = i2cm_read(hi2c,I2C_AS72XX_SLAVE_STATUS_REG);
		if ((status & I2C_AS72XX_SLAVE_TX_VALID) == 0) 
			// No inbound TX pending at slave. Okay to write now.
			break ;
//		HAL_Delay(5);
	}
	// Send the virtual register address (enabling bit 7 to indicate a write).
	i2cm_write(hi2c, I2C_AS72XX_SLAVE_WRITE_REG, (virtualReg | 0x80)) ;
	while (1)
	{
		// Read the slave I²C status to see if the write buffer is ready.
		status = i2cm_read(hi2c, I2C_AS72XX_SLAVE_STATUS_REG) ;
		if ((status & I2C_AS72XX_SLAVE_TX_VALID) == 0)
		// No inbound TX pending at slave. Okay to write data now.
		break ;
//		HAL_Delay(5);

	}
	// Send the data to complete the operation.
	i2cm_write(hi2c,I2C_AS72XX_SLAVE_WRITE_REG, d) ;
}

/**
 * Read from AS7265x virtual register. Based on code in the AS7265x datasheet. 
 */
uint8_t as7265x_vreg_read(I2C_HandleTypeDef *hi2c, uint8_t virtualReg)
{
	volatile uint8_t status, d;

//	status = i2cm_read(&hi2c2, I2C_AS72XX_SLAVE_STATUS_REG);
//	if ( (status & I2C_AS72XX_SLAVE_RX_VALID) != 0)  {
//		// data to be read
//		d = i2cm_read(&hi2c2, I2C_AS72XX_SLAVE_READ_REG);
//	}

	// Wait for WRITE flag to clear
	while (1)
	{
		// Read slave I²C status to see if the read buffer is ready.
		status = i2cm_read(hi2c, I2C_AS72XX_SLAVE_STATUS_REG) ;
		if ((status & I2C_AS72XX_SLAVE_TX_VALID) == 0)
		// No inbound TX pending at slave. Okay to write now.
		break;
//		HAL_Delay(5);

	}


	// Send the virtual register address (disabling bit 7 to indicate a read).
	i2cm_write(hi2c, I2C_AS72XX_SLAVE_WRITE_REG, virtualReg);


	while (1)
	{
		// Read the slave I²C status to see if our read data is available.
		status = i2cm_read(hi2c, I2C_AS72XX_SLAVE_STATUS_REG);
		if ((status & I2C_AS72XX_SLAVE_RX_VALID)!= 0)
		// Read data is ready.
		break;
//		HAL_Delay(5);

	}

	// Read the data to complete the operation.
	d = i2cm_read(hi2c, I2C_AS72XX_SLAVE_READ_REG) ;
	return d;
}

/**
 * Test DATA_RDY flag of configuration virtual register (add
 *
 * @return 0 if not set, non-zero if set.
 */
int as7265x_is_data_available (I2C_HandleTypeDef *hi2c)
{
	int status = as7265x_vreg_read(hi2c, AS7265X_CONFIG);
	return (status & (1<<1) );
}

/**
 * Select device
 *
 * @param device 0=master; 1=first slave; 2=second slave
 */
void as7265x_device_select(I2C_HandleTypeDef *hi2c, uint8_t device) {
	as7265x_vreg_write(hi2c, AS7265X_DEV_SELECT_CONTROL, device);
}


/**
 * Set amplifier gain

 * @param gain 0= 1x (default), 1=3.7x, 2=16x, 3=64x
 */
void as7265x_set_gain (I2C_HandleTypeDef *hi2c, int gain) 
{

	int value = as7265x_vreg_read(hi2c, AS7265X_CONFIG);
	value &= 0b11001111; // clear gain bits
	value |= (gain&0b11) << 4;
	as7265x_vreg_write(hi2c, AS7265X_CONFIG,value);
}

/**
 * Set ADC integration time. 
 *
 * @param time from 1..255. 2.8ms units.
 */
void as7265x_set_integration_time(I2C_HandleTypeDef *hi2c, uint8_t time)
{
	as7265x_vreg_write(hi2c, AS7265X_INTERGRATION_TIME, time);
}

/**
 * Set bulb current.
 *
 * @param device 0, 1, or 2
 * @param current 
 */
void as7265x_set_bulb_current(I2C_HandleTypeDef *hi2c, uint8_t device, uint8_t current)
{
	as7265x_device_select(hi2c,device);

	current &= 0b11;

	uint8_t value = as7265x_vreg_read(hi2c,AS7265X_LED_CONFIG);
	value &= 0b11001111; //Clear ICL_DRV bits
	value |= (current << 4); //Set ICL_DRV bits with user's choice
	as7265x_vreg_write(hi2c,AS7265X_LED_CONFIG, value);
}

/**
 * Bulb enable / disable
 */
void as7265x_bulb_enable (I2C_HandleTypeDef *hi2c, uint8_t device) 
{
	as7265x_device_select(hi2c, device);

	uint8_t value = as7265x_vreg_read(hi2c, AS7265X_LED_CONFIG);
	// bit 3: bulb en/disable
	value |= (1 << 3);
	as7265x_vreg_write(hi2c, AS7265X_LED_CONFIG, value);
}

void as7265x_bulb_disable (I2C_HandleTypeDef *hi2c, uint8_t device)
{
        as7265x_device_select(hi2c, device);
	uint8_t value = as7265x_vreg_read(hi2c, AS7265X_LED_CONFIG);
	// bit 3: bulb en/disable
	value &= ~(1 << 3);
	as7265x_vreg_write(hi2c, AS7265X_LED_CONFIG, value);
}

void as7265x_indicator_enable (I2C_HandleTypeDef *hi2c) 
{
	as7265x_device_select(hi2c, 0);
	uint8_t value = as7265x_vreg_read(hi2c, AS7265X_LED_CONFIG);
	value |= (1<<0);
	as7265x_vreg_write(hi2c, AS7265X_LED_CONFIG, value);
}

void as7265x_indicator_disable (I2C_HandleTypeDef *hi2c) 
{
        as7265x_device_select(hi2c, 0);
        uint8_t value = as7265x_vreg_read(hi2c, AS7265X_LED_CONFIG);
        value &= ~(1<<0);
        as7265x_vreg_write(hi2c, AS7265X_LED_CONFIG, value);
}

/**
 * Set measurement mode.
 * 
 * @param mode 2 = all 6 channels continuous; 3 = one shot all channels
 */
void as7265x_set_measurement_mode(I2C_HandleTypeDef *hi2c, uint8_t mode) 
{
	uint8_t value = as7265x_vreg_read(hi2c, AS7265X_CONFIG);
	value &= 0b11110011;
	value |= (mode&0b11) << 2;
	as7265x_vreg_write(hi2c, AS7265X_CONFIG, value);
}

/**
 * Read calibrated value (IEEE 754 float)
 */
float as7265x_get_calibrated_value (I2C_HandleTypeDef *hi2c, uint8_t device, uint8_t base_addr)
{
	int i;
	uint8_t value;
	uint32_t shift_reg = 0;

	as7265x_device_select(hi2c, device);

	for (i = base_addr; i < base_addr+4; i++) {
		shift_reg <<= 8;
		value = as7265x_vreg_read(hi2c, i);
		shift_reg |= value;
	}
	// convert content of shift_reg to floating point
	float ret;
	memcpy (&ret, &shift_reg, sizeof(float));
	return ret;
}

/**
 * Read raw value (16 bit unsigned integer)
 */
int as7265x_get_raw_value (I2C_HandleTypeDef *hi2c, uint8_t device, uint8_t base_addr)
{
	as7265x_device_select(hi2c, device);
        uint32_t value = (as7265x_vreg_read(hi2c, base_addr)<<8);
	value |= as7265x_vreg_read(hi2c,base_addr+1);
	return value;
}


/**
 * Read all 18 channels. Channels AS72651 (vis): channels 0-5, AS72652 (vis+IR): channels 6-11,
 * AS72653 (vis+UV): channels 12-17.
 */
void as7265x_get_all_calibrated_values (I2C_HandleTypeDef *hi2c, as7265x_channels_t *channels)
{

	uint8_t base_addr;
	int channel_index = 0;
	uint8_t device;
	float v;

	for (device = 0; device < 3; device++) {
		for (base_addr = 0x14; base_addr < 0x2c; base_addr += 4) {	
			v = as7265x_get_calibrated_value (hi2c, device, base_addr);
			channels->channel[channel_index] = v;
			channel_index++;
		}
	}

}
/**
 * Read all 18 channels raw ADC. Channels AS72651 (vis): channels 0-5, AS72652 (vis+IR): channels 6-11,
 * AS72653 (vis+UV): channels 12-17
 */

void as7265x_get_all_raw_values (I2C_HandleTypeDef *hi2c, as7265x_raw_channels_t *channels) 
{
	int base_addr;
	int device;
	int channel_index = 0;
	for (device = 0; device < 3; device++) {
		for (base_addr = 0x8; base_addr < 0x14; base_addr += 2) {
			channels->channel[channel_index] = (uint16_t)as7265x_get_raw_value(hi2c, device, base_addr);
			channel_index++;
		}
	}

}


/**
 * Order channels in ascending wavelength.
 */
void as7265x_order_calibrated_channels(I2C_HandleTypeDef *hi2c, as7265x_channels_t *channels) 
{
	float buf[18];
	int i;
	for (i = 0; i < 18; i++) {
		buf[i] = channels->channel[as7265x_channel_order_table[i]];
	}
	for (i = 0; i < 18; i++) {
		channels->channel[i] = buf[i];
	}
}

void as7265x_order_raw_channels(I2C_HandleTypeDef *hi2c, as7265x_raw_channels_t *channels) 
{
	uint16_t buf[18];
	int i;
	for (i = 0; i < 18; i++) {
		buf[i] = channels->channel[as7265x_channel_order_table[i]];
	}
	for (i = 0; i < 18; i++) {
		channels->channel[i] = buf[i];
	}
}


void as7265x_measure(I2C_HandleTypeDef *hi2c)
{
	int i;
	as7265x_set_measurement_mode(hi2c, AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
	for (i = 0; i < 100; i++) {
		if ( as7265x_is_data_available(hi2c) )
		{
			break;
		}
	}


	as7265x_get_calibrated_value (hi2c, AS7265X_R_G_A_CAL, AS72653_UV);

}

void as7265x_soft_reset (I2C_HandleTypeDef *hi2c) 
{
	as7265x_vreg_write(hi2c, AS7265X_CONFIG, (1<<7));
}


as7265x_wavelengths_t as7265x_get_unordered_channel_wavelengths (void)
{
	int i = 0;
	as7265x_wavelengths_t ret;
	for (i = 0; i < 18; i++) {
		ret.channel[i] = as7265x_unordered_channel_wavelength[i];
	}
	// TODO: is this allowed in C?
	return ret;
}

as7265x_wavelengths_t as7265x_get_ordered_channel_wavelengths (void)
{
	int i = 0;
	as7265x_wavelengths_t ret;
	for (i = 0; i < 18; i++) {
		ret.channel[i] = as7265x_unordered_channel_wavelength[i];
	}
	return ret;
}

