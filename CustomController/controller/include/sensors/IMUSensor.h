#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include "SensorBase.h"
#include "I2CManager.h"
#include <math.h>

#define BMI270_ADDR      (0x68 << 1)   // 7-bit addr shifted for HAL
#define BMI270_CHIP_ID   0x00          // Chip ID register address for BMI270
#define BMI270_CMD       0x7E          // Command register
#define BMI270_ACC_CONF  0x40          // Accel config register (BMI270)
#define BMI270_GYR_CONF  0x42          // Gyro config register (BMI270)
#define BMI270_PWR_CONF  0x7C          // Power config register (BMI270)
#define BMI270_DATA_ACC  0x0D          // Accel data start register (BMI270)

class IMUSensor : public SensorBase {
private:
    void BMI270_readData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
    void updateOrientation();
    float pitch = 0.0f, roll = 0.0f;
    float dt = 0.01f; // 100 Hz loop
public:
    void init() override;
    float getPitch() {updateOrientation(); return pitch;}
    float getRoll() {updateOrientation(); return roll;}
};

#endif