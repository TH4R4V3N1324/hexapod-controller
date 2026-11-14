#include "I2CLink.h"

#include <driver/i2c_slave.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/i2c_ll.h>
#include <stdio.h>
#include <string.h>

#include "Clock.h"
#include "Config.h"
#include "Link.h"

// The Servo 2040 is the I2C master. Each 10 ms tick it reads a ControlPacket (one transaction), and
// later writes a HexPacket (another). The ESP32 can't stretch the clock as a slave, so a read gets
// whatever is already in the TX FIFO: like Arduino's Wire, we keep exactly one ControlPacket loaded,
// and replace it with the newest after every read. That one is sent on the next tick.

namespace i2c {
namespace {

constexpr i2c_port_num_t PORT = I2C_NUM_0;
constexpr uint8_t MARK = 0xA5;   // fills the receive buffer, so an untouched buffer means "that was a read"
constexpr uint32_t RELOAD_MS = 100;

i2c_slave_dev_handle_t slave = nullptr;
TaskHandle_t task = nullptr;
uint8_t rxBuffer[sizeof(HexPacket)];

// ISR: a transaction finished (read or write); the task works out which
bool IRAM_ATTR onDone(i2c_slave_dev_handle_t, const i2c_slave_rx_done_event_data_t*, void*) {
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(task, &woken);
    return woken == pdTRUE;
}

void arm() {
    memset(rxBuffer, MARK, sizeof(rxBuffer));
    i2c_slave_receive(slave, rxBuffer, sizeof(rxBuffer));
}

// Replace whatever is in the TX FIFO with the newest ControlPacket
void load() {
    const ControlPacket packet = link::control();
    i2c_ll_txfifo_rst(I2C_LL_GET_HW(PORT));
    i2c_slave_transmit(slave, reinterpret_cast<const uint8_t*>(&packet), sizeof(packet), 0);
}

bool wasWrite() {
    for (uint8_t b : rxBuffer) {
        if (b != MARK) return true;
    }
    return false;
}

void i2cTask(void*) {
    arm();
    load();
    uint32_t lastLoadMs = nowMs();
    for (;;) {
        const bool done = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(RELOAD_MS)) > 0;
        if (done) {
            if (wasWrite()) {
                // A Servo 2040 on another protocol version is ignored (its status, not its reads)
                if (rxBuffer[0] != HEXAPOD_PROTOCOL_VERSION) {
                    arm();
                    continue;
                }
                HexPacket status;
                memcpy(&status, rxBuffer, sizeof(status));
                link::setStatus(status);
            } else {
                load();  // the master just read; queue the newest for its next read
                lastLoadMs = nowMs();
            }
            arm();
        }
        // Nobody reading (Servo 2040 off or not wired): keep the loaded packet fresh anyway
        if (nowMs() - lastLoadMs >= RELOAD_MS) {
            load();
            lastLoadMs = nowMs();
        }
    }
}

}  // namespace

bool begin() {
    i2c_slave_config_t cfg = {};
    cfg.i2c_port = PORT;
    cfg.sda_io_num = (gpio_num_t)config::I2C_SDA_PIN;
    cfg.scl_io_num = (gpio_num_t)config::I2C_SCL_PIN;
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.send_buf_depth = 4 * sizeof(ControlPacket);
    cfg.slave_addr = config::I2C_ADDRESS;
    cfg.addr_bit_len = I2C_ADDR_BIT_LEN_7;
    if (i2c_new_slave_device(&cfg, &slave) != ESP_OK) {
        printf("Failed to start the I2C slave\n");
        return false;
    }
    // The task first, so the callback always has a task to notify
    xTaskCreatePinnedToCore(i2cTask, "i2c", 3072, nullptr, 10, &task, 1);
    i2c_slave_event_callbacks_t cbs = {};
    cbs.on_recv_done = onDone;
    i2c_slave_register_event_callbacks(slave, &cbs, nullptr);
    return true;
}

void report() {
    // Nothing yet: the slave driver doesn't say how long a write was, so bad lengths can't be spotted
}

}  // namespace i2c
