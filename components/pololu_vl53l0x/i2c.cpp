#include "i2c.h"
#include <driver/i2c.h>

_i2c i2c;

void _i2c::init(int scl, int sda) {
    i2c_config_t conf;
    const int I2C_MASTER_SCL_IO = scl;       /*!< GPIO number used for I2C master clock */
    const int I2C_MASTER_SDA_IO = sda;       /*!< GPIO number used for I2C master data  */
    // const int I2C_MASTER_NUM = I2C_NUM_0;    /*!< I2C master i2c port number */
    const int I2C_MASTER_FREQ_HZ = 100000;   /*!< I2C master clock frequency */
    const int I2C_MASTER_TX_BUF_DISABLE = 0; /*!< I2C master doesn't need buffer */
    const int I2C_MASTER_RX_BUF_DISABLE = 0; /*!< I2C master doesn't need buffer */
    
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;

    // Cấu hình I2C với port cụ thể
    i2c_param_config(I2C_NUM_0, &conf);
    // Cài đặt driver
    i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void _i2c::write(uint8_t addr, uint8_t* data, size_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);  // ACK cho byte địa chỉ
    i2c_master_write(cmd, data, size, true);  // ACK cho dữ liệu
    i2c_master_stop(cmd);

    // Kiểm tra trạng thái sau khi gửi lệnh
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        printf("I2C Write Error: %s\n", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);
}

void _i2c::write(uint8_t addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);  // ACK cho byte địa chỉ
    i2c_master_write_byte(cmd, data, true);  // ACK cho byte dữ liệu
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        printf("I2C Write Error: %s\n", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);
}

void _i2c::read(uint8_t addr, uint8_t* data, size_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);  // ACK cho byte địa chỉ
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);  // Đọc dữ liệu, NACK ở byte cuối
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        printf("I2C Read Error: %s\n", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);
}
