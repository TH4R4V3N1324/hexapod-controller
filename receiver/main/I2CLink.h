// I2C slave for the Servo2040 stack: the Servo 2040 (master) reads ControlPackets and writes
// HexPackets
#ifndef RECEIVER_I2C_LINK_H
#define RECEIVER_I2C_LINK_H

namespace i2c {

bool begin();
// Call from the loop: prints what the callbacks noticed
void report();

}  // namespace i2c

#endif
