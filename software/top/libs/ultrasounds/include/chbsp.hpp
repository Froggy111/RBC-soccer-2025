#pragma once

extern "C" {

#include <invn/soniclib/soniclib.h>
#include <invn/soniclib/chirp_bsp.h>

// All function declarations here...
void chbsp_reset_assert(void);
void chbsp_reset_release(void);
void chbsp_program_enable(ch_dev_t *dev_ptr);
void chbsp_program_disable(ch_dev_t *dev_ptr);
void chbsp_set_int1_dir_out(ch_dev_t *dev_ptr);
void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr);
void chbsp_int1_clear(ch_dev_t *dev_ptr);
void chbsp_group_int1_clear(ch_group_t *grp_ptr);
int chbsp_i2c_init(void);
void chbsp_delay_us(uint32_t us);
void chbsp_delay_ms(uint32_t ms);
uint32_t chbsp_timestamp_ms(void);
int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes);
int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes);
int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes);
int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes);
void chbsp_spi_cs_on(ch_dev_t *dev_ptr);
void chbsp_spi_cs_off(ch_dev_t *dev_ptr);
int chbsp_spi_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes);
void chbsp_i2c_reset(ch_dev_t *dev_ptr);
uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num, ch_i2c_info_t *info_ptr);
}

class MCP23S17;

// Global DMUX instances declaration
extern MCP23S17 *dmux1;
extern MCP23S17 *dmux2;