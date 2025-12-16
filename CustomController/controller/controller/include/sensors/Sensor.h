#ifndef SENSOR_H
#define SENSOR_H

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