//#include "soc/i2c_struct.h"     // Gives you I2C0, I2C1 register structs
#include "soc/gpio_struct.h"    // Gives you GPIO register access
//#include "soc/soc.h"            // Base defines and types

#include "i2c_display.h"

/* define I2C register addresses */
#define I2C0						(0x3FF53000)
#define I2C_SLAVE_ADDR_REG  		(0x3FF53010)
#define I2C_CTR_REG					(0x3FF53004)	
#define I2C_SR_REG					(0x3FF53008)
#define I2C_SCL_LOW_PERIOD_REG		(0x3FF53000)
#define I2C_SCL_HIGH_PERIOD_REG		(0x3FF53038)
#define TRANSMIT_FIFO				(0x3FF5301C)
#define SLAVE_ADDR_7_BIT			(SCREEN_ADDR << 1)

#define GPIO_REG_START				(0x3FF44000)
#define GPIO_FUNC21_OUT_SEL_CFG_REG	(0x3FF44000 + 0x530 + (0x4 * 21))
#define GPIO_FUNC22_OUT_SEL_CFG_REG	(0x3FF44000 + 0x530 + (0x4 * 22))
#define GPIO_ENABLE_W1TS_REG		(0x3FF44024)
#define GPIO_PIN21_REG				(0x3FF44000 + 0x88 + (0x4 * 21))
#define GPIO_PIN22_REG				(0x3FF44000 + 0x88 + (0x4 * 22))

// below valued pulled from ./components/soc/esp32/register/soc/reg_base.h:#define DR_REG_IO_MUX_BASE 0x3FF49000
// offset pulled from: #define PERIPHS_IO_MUX_GPIO21_U (DR_REG_IO_MUX_BASE + 0xD4)
// keep grepping till you get the base value
#define IO_MUX_21_REG				(0x3FF49000 + 0xD4)

#define SDA_PIN			    21
#define SCL_PIN				22
#define FUN_IE				9

#define GPIO_FUNC21_OUT_SEL 	30
#define GPIO_FUNC22_OUT_SEL 	29

void i2c_init(void){
	/* Create register pointers */
	volatile uint32_t *i2c0					       = (volatile uint32_t *) I2C0;
	volatile uint32_t *i2c_slave_addr_reg 	       = (volatile uint32_t *) I2C_SLAVE_ADDR_REG;
	volatile uint32_t *i2c_ctr_reg 			       = (volatile uint32_t *) I2C_CTR_REG;
	volatile uint32_t *i2c_sr_reg 				   = (volatile uint32_t *) I2C_SR_REG;
	
	volatile uint32_t *gpio_func21_out_sel_cfg_reg = (volatile uint32_t *) GPIO_FUNC21_OUT_SEL_CFG_REG;
	volatile uint32_t *gpio_func22_out_sel_cfg_reg = (volatile uint32_t *) GPIO_FUNC22_OUT_SEL_CFG_REG;
	
	volatile uint32_t *gpio_enable_w1ts_reg		   = (volatile uint32_t *) GPIO_ENABLE_W1TS_REG;
	volatile uint32_t *io_mux_21_reg			   = (volatile uint32_t *) IO_MUX_21_REG;
	volatile uint32_t *gpio_pin21_reg			   = (volatile uint32_t *) GPIO_PIN21_REG;
	volatile uint32_t *gpio_pin22_reg			   = (volatile uint32_t *) GPIO_PIN22_REG;
	volatile uint32_t *i2c_scl_low_period_reg	   = (volatile uint32_t *) I2C_SCL_LOW_PERIOD_REG;
	volatile uint32_t *i2c_scl_high_period_reg	   = (volatile uint32_t *) I2C_SCL_HIGH_PERIOD_REG;
	
	/* enable I2C0 peripheral clock, will do later if necessary */
	
	/* ---- configure GPIO matrix for pin 21 (SDA) and pin 22 (SCL) ---- */
	
	/* Route signals to GPIO pins 21 and 22
	   - I2CEXT0_SCL_OUT_IDX: signal 29 
	   - I2CEXT0_SDA_OUT_IDX: signal 30
	*/
	*gpio_func21_out_sel_cfg_reg |= GPIO_FUNC21_OUT_SEL;
	*gpio_func22_out_sel_cfg_reg |= GPIO_FUNC22_OUT_SEL;
	
	/* Configure pin 21 for SDA input from slave and enable it. 
	   No need to do this for pin 22, as SCL goes one way - output
	   By linking pin 22 with the SCL signal, we've "technically" configured it as output
	   However, for SDA, we'll be reading from there too, so we need the GPIO hardware to allow the pin to be read
	   We only need to enable input on GPIO21
	*/
	
	*io_mux_21_reg |= FUN_IE;
	
	/* Set both pins to open drain */
	*gpio_pin21_reg |= (1U << 2);
	*gpio_pin22_reg |= (1U << 2);
	
	/* Now, set I2C slave device address */
	*i2c_slave_addr_reg = SLAVE_ADDR_7_BIT;
	
	/* Now, we configure the I2C control register.
	   We want to configure the I2C module in master mode
	   We also want to enable the I2C module and enable I2C FIFO mode 
	*/
	*i2c_ctr_reg |= (1U << 4);
	
	/* Now, let's move onto configuring the clock 
	   - We need to set the I2C bus speed. The I2C clock is controlled by adjusting low & high periods of SCL
	   - APB (Advanced Peripheral Bus) clock is 80 MHz by default
	   - To get a 100 kHz SCL signal (period = 10 microsec)
	     - 10 us x 80 MHz = 800 cycles total --> low 400 cycles, high 400 cycles.
	*/
	*i2c_scl_high_period_reg = 400;
	*i2c_scl_low_period_reg  = 400;
	
	
	

}
