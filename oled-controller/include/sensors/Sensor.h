#ifndef SENSOR_H
#define SENSOR_H

#include "Board.h"

#if BOARD_REV == 5
// v5: MAX17048 fuel gauge on I2C (SDA/SCL in Board.h). The BMI270 IMU has no driver yet: it needs
// Bosch's config blob loaded at start-up, so it isn't listed here.
#include "BatteryGauge.h"

class Sensor {
public:
    BatteryGauge batteryGauge;
    void init() {
        i2cManager.init(PIN_I2C_SDA, PIN_I2C_SCL);
        batteryGauge.init();
    }
};
#else
// v3: INA260 current sensor and BMI330 IMU
#include "CurrentSensor.h"
#include "IMUSensor.h"

class Sensor {
public:
    CurrentSensor currentSensor;
    IMUSensor imuSensor;
    void init() {
        currentSensor.init();
        imuSensor.init();
    }
};
#endif

#endif
