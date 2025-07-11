#include "driver/i2c.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "font8x8_basic.h"

static const char *TAG = "I2C_DISPLAY";

void i2c_master_init(){
	i2c_config_t config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA_PIN,
			.scl_io_num = SCL_PIN,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = I2C_FREQ
	};

	/* Apply our configurations and install the driver */
	i2c_param_config(I2C_PORT, &config);
	i2c_driver_install(I2C_PORT, config.mode, 0, 0, 0);
}

void send_command(uint8_t cmd){
	i2c_cmd_handle_t command = i2c_cmd_link_create();

	/* Add a start condition to the I2C command list - ie pull SDA low while SCL remains high */
	i2c_master_start(command);

	/* slave address and write command - remember the first byte in I2C is 7-bit slave addr + R/W bit
	 * I2C_MASTER_WRITE is a 0, while I2C_MASTER_READ is a 1
	 * */
	i2c_master_write_byte(command, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);

	/* Send control byte 0x80, indicating one byte is to be sent after, which will be a command */
	i2c_master_write_byte(command, DISPLAY_ONE_CMD, true);

	/* send the command, which is 0xAE - turns display off */
	i2c_master_write_byte(command, cmd, true);

	/* stop transmission */
	i2c_master_stop(command);

	i2c_master_cmd_begin(I2C_PORT, command, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(command);
}

void send_command_burst(const uint8_t *commands, size_t len){
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);

	for(size_t i = 0; i < len; i++){
		/* control byte: indicate that one command is coming - in the original send_command, we send this as well */
		i2c_master_write_byte(cmd, DISPLAY_ONE_CMD, true);

		/* write the actual command
		 * remember *(commands + i), that's how we can treat the uint8_t pointer as an "array"
		*/
		i2c_master_write_byte(cmd, commands[i], true);
	}
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void display_init_burst() {
    const uint8_t init_sequence[] = {
        DISPLAY_OFF,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0x7F,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA7,
        DISPLAY_ON
    };

    send_command_burst(init_sequence, sizeof(init_sequence));
}



void display_init(){
	send_command(DISPLAY_OFF);

	/* set oscillator frequency & clock divide ratio. 0x80 for default oscillator. */
	send_command(0xD5);
	send_command(0x80);

	/* 0x3F = 63, so we're enabling 64 mux lines (0 to 63). This must match display's vertical resolution */
	send_command(0xA8);
	send_command(0x3F);

	/* how many rows to vertically shift the image. 0 unless we want a vertical offset */
	send_command(0xD3);
	send_command(0x00);

	/* Defines the first row of GDDRAM to display. 0x40 = start at line 0 */
	send_command(0x40);

	/* 0x8D enables access to internal voltage booster. 0x14 turns charge pump on */
	send_command(0x8D);
	send_command(0x14);

	/* 0x20 switches memory mode, 0x00 = horizontal addressing */
	send_command(0x20);
	send_command(0x00);

	/* Flips screen left to right (column address 0 maps to SEG127). Needed if display is flipped */
	send_command(0xA1);

	/* Flips screen vertically (row 0 maps to COM[n-1]). Used with 0xA1 to fully rotate screen 180 deg if needed */
	send_command(0xC8);

	/* 0x12 is for displays with 64 COM lines - this must match the internal wiring of your panel */
	send_command(0xDA);
	send_command(0x12);

	/* 0x7F = mid-level contrast. We can tune this (0x00 - 0xFF) for brightness */
	send_command(0x81);
	send_command(0x7F);

	/* Controls how long pixels are pre-charged before being turned on. 0xF! is recommended for internal charge pump */
	send_command(0xD9);
	send_command(0xF1);

	/* Adjust voltage level to reduce flicker/ghosting. 0x40 = ~0.77 vcc (a good default value) */
	send_command(0xDB);
	send_command(0x40);

	/* Resume display from RAM content - 0xA4 = display what's in GDDRAM */
	send_command(0xA4);

	/* 0xA6 = white on black (normal). 0xA7 = black on white (inverted). */
	send_command(0xA6);

	send_command(DISPLAY_ON);
}

void send_data(uint8_t data){
	i2c_cmd_handle_t command = i2c_cmd_link_create();
	i2c_master_start(command);

	i2c_master_write_byte(command, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);
	/* Control byte to indicate the next byte is data: 0x40 */
	i2c_master_write_byte(command, 0x40, true);
	i2c_master_write_byte(command, data, true);

	i2c_master_stop(command);

	i2c_master_cmd_begin(I2C_PORT, command, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(command);
}

void clear_display(){
	for(uint8_t page = 0; page < 8; page++){
		send_command(0xB0 + page);

		/* Set lower column address = 0 */
		send_command(0x00);
		/* set upper column address = 0 */
		send_command(0x10);

		/* final column address: 0b 0000 0000
		 * Remember the SSD1306 interprets the actual value of the byte as a command.
		*/

		for(uint8_t col = 0; col < 128; col++){
			/* Turn off all 8 vertical pixels in this column */
			send_data(0x00);
			/* Yield every 16 bytes */
			if (col % 16 == 0) vTaskDelay(1);
		}
	}
}

void display_write_char(char c, uint8_t col, uint8_t page){
	/* reset watchdog manually */
	esp_task_wdt_reset();
	vTaskDelay(pdMS_TO_TICKS(1000));

	/* check for out of bounds */
	if (c < 0 || c > 127) {
		return;
	}

	/* Set page */
	send_command(0xB0 + page);

	/* Set column */
	/*
	 * This sets the lower nibble (bits 0–3) of the column address.
	 * The SSD1306 requires the lower 4 bits to be set using command 0x00–0x0F.
	 *
	 * Example: if col = 45 (0x2D, binary 0010 1101)
	 *   col & 0x0F = 0x0D (binary 1101)
	 *   0x00 + 0x0D = 0x0D
	 * → Sends command 0x0D to set lower nibble to 13
	 */
	send_command(0x00 + (col & 0x0F));

	/*
	 * This sets the upper nibble (bits 4–7) of the column address.
	 * The SSD1306 requires the upper 4 bits to be set using command 0x10–0x1F.
	 *
	 * Example: col = 45 (0x2D, binary 0010 1101)
	 *   col >> 4 = 0x02 (binary 0010)
	 *   0x10 + 0x02 = 0x12
	 * → Sends command 0x12 to set upper nibble to 2
	 *
	 * Actual column = (upper << 4) | lower = (2 << 4) | 13 = 32 + 13 = 45
	 */
	send_command(0x10 + ((col >> 4) & 0x0F));

	for (int i = 0; i < 8; i++) {
	    uint8_t byte = font8x8_basic_tr[(uint8_t)c][i];
	    ESP_LOGI("CHAR", "Byte %d: 0x%02X", i, ~byte);
	    send_data(byte);
	    /* This is A:
		 *
		 * { 0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00, 0x00 }
		 *
		 */
	    vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void display_write_string(const char* str, uint8_t col, uint8_t page){
    uint8_t func_col = col;
    uint8_t func_page = page;

    while (*str) {
        if (func_col + 8 > 128) {  // Avoid wrapping mid-character
            func_col = 0;
            func_page++;
            if (func_page >= 8) break;
        }

        display_write_char(*str, func_col, func_page);
        ESP_LOGI("I2C_DISPLAY", "char: %c at col: %u page: %u", *str, func_col, func_page);

        func_col += 8;  // move to next char slot
        str++;
    }
}


void display_burst_write_string(const char* string, uint8_t col, uint8_t page){
	while(*string && page < 8){
		/* Check for column overflow */
		if(col + 8 > 128){
			col = 0;
			page++;
			if(page >= 8) break;
		}
		/* Set page and column - same as display_write_char() */
		send_command(0xB0 + page);
		send_command(0x00 + (col & 0x0F));
		send_command(0x10 + ((col >> 4) & 0x0F));

		/* Prep persistent I2C command handle */
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		/* add start condition to transaction */
		i2c_master_start(cmd);

		/* Send 7-bit slave addr + 1 bit write mode */
		i2c_master_write_byte(cmd, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);

		/* send control byte indicating the bytes following are all data */
		i2c_master_write_byte(cmd, 0x40, true);

		/* Push characters until we hit the end or fill a page */
		while(*string && col + 8 <= 128){
			uint8_t *character = font8x8_basic_tr[(uint8_t) *string];

			i2c_master_write(cmd, character, 8, true);
			col += 8;
			string++;
		}

		/* Stop I2C */
        i2c_master_stop(cmd);

        /* begin i2c from the command handle, then delete the command link after it's done */
        i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

	}

}

esp_err_t i2c_read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len){
	/* parameters check */
	if (len == 0 || data == NULL) {
	        return ESP_ERR_INVALID_ARG;
	}

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	/* Write address of the device */
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true); /* send first byte: address + r/w bit */
	i2c_master_write_byte(cmd, reg_addr, true);

	/* Repeated start - keep control of the bus. Address bytes are not valid unless preceded by a start or repeated start */
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

	/* verify number of bytes requested is at least 1 */
	if(len > 1){
		i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);

	i2c_master_stop(cmd);

	esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return err;
}

esp_err_t i2c_write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t value){
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true); /* send first byte: address + r/w bit */
	i2c_master_write_byte(cmd, reg_addr, true);

	/* Write the value to the specified register */
	i2c_master_write_byte(cmd, value, true);

	esp_err_t err = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return err;

}

void draw_wifi_icon(uint8_t col, uint8_t page) {
	ESP_LOGI(TAG, "Drawing Icon at col: %u, page: %u", col, page);
    const uint8_t wifi_bitmap[2][2] = {
        {0b00011000, 0b00100100},  // Page 0: columns 0 and 1
        {0b01000010, 0b10000001}   // Page 1: columns 0 and 1
    };

    for (int p = 0; p < 2; p++) {
        send_command(0xB0 + page + p);
        send_command(0x00 + ((col + 0) & 0x0F));
        send_command(0x10 + (((col + 0) >> 4) & 0x0F));

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x40, true); // data

        i2c_master_write_byte(cmd, wifi_bitmap[p][0], true);
        i2c_master_write_byte(cmd, wifi_bitmap[p][1], true);

        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }
}

void display_wifi_symbol() {
    // WiFi symbol will be drawn in yellow bar area (spans pages 0 and 1)
    // Starting at column 120 (128-8) to leave 8 pixels from right edge
    uint8_t start_col = 120;

    // Your custom bitmap: 16 rows by 8 columns
    // Converting to SSD1306 format: 8 bytes for page 0, 8 bytes for page 1

    // Top 8 rows (page 0) - extracting bits 8-15 from each column
    const uint8_t wifi_page0[] = {
        0x9E,  // Column 0: 10011110
		0x8E,  // Column 1: 10001110
		0xC7,  // Column 2: 11000111
		0xC7,  // Column 3: 11000111
		0xC7,  // Column 4: 11000111
		0xC7,  // Column 5: 11000111
		0x8E,  // Column 6: 10001110
		0x9E   // Column 7: 10011110
    };

    const uint8_t wifi_page1[] = {
        0x1F,  // Column 0: 00011111
		0x0B,  // Column 1: 00001011
		0x41,  // Column 2: 01000001
		0xE1,  // Column 3: 11100001
		0xE1,  // Column 4: 11100001
		0x41,  // Column 5: 01000001
		0x09,  // Column 6: 00001001
		0x1F   // Column 7: 00011111
    };

    // Draw page 0 (top half)
    send_command(0xB0 + 0);  // Set page 0
    send_command(0x00 + (start_col & 0x0F));  // Set lower column nibble
    send_command(0x10 + ((start_col >> 4) & 0x0F));  // Set upper column nibble

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x40, true);
    i2c_master_write(cmd, wifi_page0, sizeof(wifi_page0), true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // Draw page 1 (bottom half)
    send_command(0xB0 + 1);  // Set page 1
    send_command(0x00 + (start_col & 0x0F));  // Set lower column nibble
    send_command(0x10 + ((start_col >> 4) & 0x0F));  // Set upper column nibble

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x40, true);
    i2c_master_write(cmd, wifi_page1, sizeof(wifi_page1), true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    ESP_LOGI(TAG, "Custom WiFi symbol displayed at column %d, pages 0-1", start_col);
}









