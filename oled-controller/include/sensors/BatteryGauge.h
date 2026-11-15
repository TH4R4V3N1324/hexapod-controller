#ifndef BATTERY_GAUGE_H
#define BATTERY_GAUGE_H

#include "SensorBase.h"
#include "I2CManager.h"

// MAX17048 single-cell fuel gauge (PCB rev v5). Replaces v3's INA260: it reports cell voltage and
// state of charge, not current.
#define MAX17048_ADDRESS     0x36
#define MAX17048_REG_VCELL   0x02   // 78.125 uV/LSB
#define MAX17048_REG_SOC     0x04   // 1/256 %/LSB
#define MAX17048_REG_MODE    0x06
#define MAX17048_REG_VERSION 0x08
#define MAX17048_REG_CRATE   0x16   // 0.208 %/h per LSB, signed
#define MAX17048_REG_STATUS  0x1A

class BatteryGauge : public SensorBase {
private:
    bool readWord(uint8_t reg, uint16_t& value);
    bool present = false;
public:
    void init() override;
    bool isPresent() const { return present; }
    float readVoltage();       // cell voltage in mV, 0 on error
    float readPercent();       // state of charge in %, 0 on error
    float readChargeRate();    // %/hour, positive while charging
};

#endif
