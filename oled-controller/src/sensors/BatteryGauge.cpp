#include "BatteryGauge.h"

/*
@brief Read a 16-bit MAX17048 register (big-endian)
*/
bool BatteryGauge::readWord(uint8_t reg, uint16_t& value) {
    uint8_t data[2];
    if (!i2cManager.readRegister(MAX17048_ADDRESS, reg, data, 2)) return false;
    value = (uint16_t)((data[0] << 8) | data[1]);
    return true;
}

/*
@brief Check the MAX17048 answers. It needs no setup: ModelGauge runs from power-up.
*/
void BatteryGauge::init() {
    uint16_t version;
    present = readWord(MAX17048_REG_VERSION, version);
}

/*
@brief Cell voltage in mV
*/
float BatteryGauge::readVoltage() {
    uint16_t raw;
    return readWord(MAX17048_REG_VCELL, raw) ? raw * 0.078125f : 0.0f;
}

/*
@brief State of charge in percent
*/
float BatteryGauge::readPercent() {
    uint16_t raw;
    return readWord(MAX17048_REG_SOC, raw) ? raw / 256.0f : 0.0f;
}

/*
@brief Charge or discharge rate in %/hour (positive while charging)
*/
float BatteryGauge::readChargeRate() {
    uint16_t raw;
    return readWord(MAX17048_REG_CRATE, raw) ? (int16_t)raw * 0.208f : 0.0f;
}
