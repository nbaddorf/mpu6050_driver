// Copyright (c) 2012 Jeff Rowberg
// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 8/24/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//  2019-07-08 - Added Auto Calibration routine
//     ... - ongoing debug release

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "mpu6050_driver/mpu6050.hpp"

MPU6050::MPU6050(uint8_t address): mpu(address) {}

void MPU6050::initialize() {
  mpu.openI2CBus("/dev/i2c-1");
  setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  setSleepEnabled(false);  // thanks to Jack Elston for pointing this one out!
}

bool MPU6050::testConnection() {
  return getDeviceID() == 0x34;
}

uint8_t MPU6050::getAuxVDDIOLevel() {
  return mpu.readByteBit(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_PWR_MODE_BIT);
}

void MPU6050::setAuxVDDIOLevel(uint8_t level) {
  if (level == 1) {
    mpu.setByteBit(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_PWR_MODE_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_PWR_MODE_BIT);
}

uint8_t MPU6050::getRate() {
  return mpu.readByte(MPU6050_RA_SMPLRT_DIV);
}

void MPU6050::setRate(uint8_t rate) {
  mpu.writeByte(MPU6050_RA_SMPLRT_DIV, rate);
}

uint8_t MPU6050::getExternalFrameSync() {
  return mpu.readByteBits(MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH);
}

void MPU6050::setExternalFrameSync(uint8_t sync) {
  mpu.setByteBits(MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, sync);
}

uint8_t MPU6050::getDLPFMode() {
  return mpu.readByteBits(MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH);
}

void MPU6050::setDLPFMode(uint8_t mode) {
  mpu.setByteBits(MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}

uint8_t MPU6050::getFullScaleGyroRange() {
  return mpu.readByteBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH);
}

void MPU6050::setFullScaleGyroRange(uint8_t range) {
  mpu.setByteBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}

uint8_t MPU6050::getAccelXSelfTestFactoryTrim() {
  uint8_t buffer[2];
  buffer[0] = mpu.readByte(MPU6050_RA_SELF_TEST_X);
  buffer[1] = mpu.readByte(MPU6050_RA_SELF_TEST_A);
  return (buffer[0] >> 3) | ((buffer[1] >> 4) & 0x03);
}

uint8_t MPU6050::getAccelYSelfTestFactoryTrim() {
  uint8_t buffer[2];
  buffer[0] = mpu.readByte(MPU6050_RA_SELF_TEST_Y);
  buffer[1] = mpu.readByte(MPU6050_RA_SELF_TEST_A);
  return (buffer[0] >> 3) | ((buffer[1] >> 2) & 0x03);
}

uint8_t MPU6050::getAccelZSelfTestFactoryTrim() {
  uint8_t buffer[2];
  mpu.readBytes(MPU6050_RA_SELF_TEST_Z, sizeof(buffer), buffer);
  return (buffer[0] >> 3) | (buffer[1] & 0x03);
}

uint8_t MPU6050::getGyroXSelfTestFactoryTrim() {
  return mpu.readByte(MPU6050_RA_SELF_TEST_X) & 0x1F;
}

uint8_t MPU6050::getGyroYSelfTestFactoryTrim() {
  return mpu.readByte(MPU6050_RA_SELF_TEST_Y) & 0x1F;
}

uint8_t MPU6050::getGyroZSelfTestFactoryTrim() {
  return mpu.readByte(MPU6050_RA_SELF_TEST_Z) & 0x1F;
}

bool MPU6050::getAccelXSelfTest() {
  return mpu.readByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_XA_ST_BIT);
}

void MPU6050::setAccelXSelfTest(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_XA_ST_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_XA_ST_BIT);
}

bool MPU6050::getAccelYSelfTest() {
  return mpu.readByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_YA_ST_BIT);
}

void MPU6050::setAccelYSelfTest(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_YA_ST_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_YA_ST_BIT);
}

bool MPU6050::getAccelZSelfTest() {
  return mpu.readByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ZA_ST_BIT);
}

void MPU6050::setAccelZSelfTest(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ZA_ST_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ZA_ST_BIT);
}

uint8_t MPU6050::getFullScaleAccelRange() {
  return mpu.readByteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH);
}

void MPU6050::setFullScaleAccelRange(uint8_t range) {
  mpu.setByteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}

uint8_t MPU6050::getDHPFMode() {
  return mpu.readByteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ACCEL_HPF_BIT, MPU6050_ACONFIG_ACCEL_HPF_LENGTH);
}

void MPU6050::setDHPFMode(uint8_t mode) {
  mpu.setByteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ACCEL_HPF_BIT, MPU6050_ACONFIG_ACCEL_HPF_LENGTH, bandwidth);
}

uint8_t MPU6050::getFreefallDetectionThreshold() {
  return mpu.readByte(MPU6050_RA_FF_THR);
}

void MPU6050::setFreefallDetectionThreshold(uint8_t threshold) {
  mpu.writeByte(MPU6050_RA_FF_THR, threshold);
}

uint8_t MPU6050::getFreefallDetectionDuration() {
  return mpu.readByte(MPU6050_RA_FF_DUR);
}

void MPU6050::setFreefallDetectionDuration(uint8_t duration) {
  mpu.writeByte(MPU6050_RA_FF_DUR, duration);
}

uint8_t MPU6050::getMotionDetectionThreshold() {
  return mpu.readByte(MPU6050_RA_MOT_THR);
}

void MPU6050::setMotionDetectionThreshold(uint8_t threshold) {
  mpu.writeByte(MPU6050_RA_MOT_THR, threshold);
}

uint8_t MPU6050::getMotionDetectionDuration() {
  return mpu.readByte(MPU6050_RA_MOT_DUR);
}

void MPU6050::setMotionDetectionDuration(uint8_t duration) {
  mpu.writeByte(MPU6050_RA_MOT_DUR, duration);
}

uint8_t MPU6050::getZeroMotionDetectionThreshold() {
  return mpu.readByte(MPU6050_RA_ZRMOT_THR);
}

void MPU6050::setZeroMotionDetectionThreshold(uint8_t threshold) {
  mpu.writeByte(MPU6050_RA_ZRMOT_THR, threshold);
}

uint8_t MPU6050::getZeroMotionDetectionDuration() {
  return mpu.readByte(MPU6050_RA_ZRMOT_DUR);
}

void MPU6050::setZeroMotionDetectionDuration(uint8_t duration) {
  mpu.writeByte(MPU6050_RA_ZRMOT_DUR, duration);
}

bool MPU6050::getTempFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_TEMP_FIFO_EN_BIT);
}

void MPU6050::setTempFIFOEnabled(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_TEMP_FIFO_EN_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_FIFO_EN, MPU6050_TEMP_FIFO_EN_BIT);
}

bool MPU6050::getXGyroFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_XG_FIFO_EN_BIT);
}

void MPU6050::setXGyroFIFOEnabled(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_XG_FIFO_EN_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_FIFO_EN, MPU6050_XG_FIFO_EN_BIT);
}

bool MPU6050::getYGyroFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_YG_FIFO_EN_BIT);
}

void MPU6050::setYGyroFIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_YG_FIFO_EN_BIT);
}

bool MPU6050::getZGyroFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_ZG_FIFO_EN_BIT);
}

void MPU6050::setZGyroFIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_ZG_FIFO_EN_BIT);
}

bool MPU6050::getAccelFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_ACCEL_FIFO_EN_BIT);
}

void MPU6050::setAccelFIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_ACCEL_FIFO_EN_BIT);
}

bool MPU6050::getSlave2FIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV2_FIFO_EN_BIT);
}

void MPU6050::setSlave2FIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV2_FIFO_EN_BIT);
}

bool MPU6050::getSlave1FIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV1_FIFO_EN_BIT);
}

void MPU6050::setSlave1FIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV1_FIFO_EN_BIT);
}

bool MPU6050::getSlave0FIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV0_FIFO_EN_BIT);
}

void MPU6050::setSlave0FIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_FIFO_EN, MPU6050_SLV0_FIFO_EN_BIT);
}

bool MPU6050::getMultiMasterEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_MULT_MST_EN_BIT);
}

void MPU6050::setMultiMasterEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_MULT_MST_EN_BIT);
}

bool MPU6050::getWaitForExternalSensorEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_WAIT_FOR_ES_BIT);
}

void MPU6050::setWaitForExternalSensorEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_WAIT_FOR_ES_BIT);
}

bool MPU6050::getSlave3FIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_SLV_3_FIFO_EN_BIT);
}

void MPU6050::setSlave3FIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_SLV_3_FIFO_EN_BIT);
}

bool MPU6050::getSlaveReadWriteTransitionEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_P_NSR_BIT);
}

void MPU6050::setSlaveReadWriteTransitionEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_P_NSR_BIT);
}

uint8_t MPU6050::getMasterClockSpeed() {
  return mpu.readByteBits(MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_CLK_BIT, MPU6050_I2C_MST_CLK_LENGTH);
}

void MPU6050::setMasterClockSpeed(uint8_t speed) {
  mpu.setByteBits(MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_CLK_BIT, MPU6050_I2C_MST_CLK_LENGTH, speed);
}

uint8_t MPU6050::getSlaveAddress(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByte(MPU6050_RA_I2C_SLV0_ADDR + (num * 3));
}

void MPU6050::setSlaveAddress(uint8_t num, uint8_t address) {
  if (num > 3) return;
  mpu.writeByte(MPU6050_RA_I2C_SLV0_ADDR + (num * 3), address);
}

uint8_t MPU6050::getSlaveRegister(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByte(MPU6050_RA_I2C_SLV0_REG + (num * 3));
}

void MPU6050::setSlaveRegister(uint8_t num, uint8_t reg) {
  if (num > 3) return;
  mpu.writeByte(MPU6050_RA_I2C_SLV0_REG + (num * 3), reg);
}

bool MPU6050::getSlaveEnabled(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_EN_BIT);
}

void MPU6050::setSlaveEnabled(uint8_t num, bool enabled) {
  if (num > 3) return;
  mpu.setByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_EN_BIT, enabled);
}

bool MPU6050::getSlaveWordByteSwap(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_BYTE_SW_BIT);
}

void MPU6050::setSlaveWordByteSwap(uint8_t num, bool enabled) {
  if (num > 3) return;
  mpu.setByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_BYTE_SW_BIT, enabled);
}

bool MPU6050::getSlaveWriteMode(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_REG_DIS_BIT);
}

void MPU6050::setSlaveWriteMode(uint8_t num, bool mode) {
  if (num > 3) return;
  mpu.setByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_REG_DIS_BIT, mode);
}

bool MPU6050::getSlaveWordGroupOffset(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_GRP_BIT);
}

void MPU6050::setSlaveWordGroupOffset(uint8_t num, bool enabled) {
  if (num > 3) return;
  mpu.setByteBit(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_GRP_BIT, enabled);
}

uint8_t MPU6050::getSlaveDataLength(uint8_t num) {
  if (num > 3) return 0;
  return mpu.readByteBits(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH);
}

void MPU6050::setSlaveDataLength(uint8_t num, uint8_t length) {
  if (num > 3) return;
  mpu.setByteBits(MPU6050_RA_I2C_SLV0_CTRL + (num * 3), MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH, length);
}

uint8_t MPU6050::getSlave4Address() {
  return mpu.readByte(MPU6050_RA_I2C_SLV4_ADDR);
}

void MPU6050::setSlave4Address(uint8_t address) {
  mpu.writeByte(MPU6050_RA_I2C_SLV4_ADDR, address);
}

uint8_t MPU6050::getSlave4Register() {
  return mpu.readByte(MPU6050_RA_I2C_SLV4_REG);
}

void MPU6050::setSlave4Register(uint8_t reg) {
  mpu.writeByte(MPU6050_RA_I2C_SLV4_REG, reg);
}

void MPU6050::setSlave4OutputByte(uint8_t data) {
  mpu.writeByte(MPU6050_RA_I2C_SLV4_DO, data);
}

bool MPU6050::getSlave4Enabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN_BIT);
}

void MPU6050::setSlave4Enabled(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN_BIT);
}

bool MPU6050::getSlave4InterruptEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_INT_EN_BIT);
}

void MPU6050::setSlave4InterruptEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_INT_EN_BIT, enabled);
}

bool MPU6050::getSlave4WriteMode() {
  return mpu.readByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_REG_DIS_BIT);
}

void MPU6050::setSlave4WriteMode(bool mode) {
  mpu.setByteBit(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_REG_DIS_BIT, mode);
}

uint8_t MPU6050::getSlave4MasterDelay() {
  return mpu.readByteBits(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_MST_DLY_BIT, MPU6050_I2C_SLV4_MST_DLY_LENGTH);
}

void MPU6050::setSlave4MasterDelay(uint8_t delay) {
  mpu.setByteBits(MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_MST_DLY_BIT, MPU6050_I2C_SLV4_MST_DLY_LENGTH, delay);
}

uint8_t MPU6050::getSlate4InputByte() {
  return mpu.readByte(MPU6050_RA_I2C_SLV4_DI);
}

bool MPU6050::getPassthroughStatus() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_PASS_THROUGH_BIT);
}

bool MPU6050::getSlave4IsDone() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV4_DONE_BIT);
}

bool MPU6050::getLostArbitration() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_LOST_ARB_BIT);
}

bool MPU6050::getSlave4Nack() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV4_NACK_BIT);
}

bool MPU6050::getSlave3Nack() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV3_NACK_BIT);
}

bool MPU6050::getSlave2Nack() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV2_NACK_BIT);
}

bool MPU6050::getSlave1Nack() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV1_NACK_BIT);
}

bool MPU6050::getSlave0Nack() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV0_NACK_BIT);
}

bool MPU6050::getInterruptMode() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_LEVEL_BIT);
}

void MPU6050::setInterruptMode(bool mode) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_LEVEL_BIT, mode);
}

bool MPU6050::getInterruptDrive() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_OPEN_BIT);
}

void MPU6050::setInterruptDrive(bool drive) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_OPEN_BIT, drive);
}

bool MPU6050::getInterruptLatch() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_LATCH_INT_EN_BIT);
}

void MPU6050::setInterruptLatch(bool latch) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_LATCH_INT_EN_BIT, latch);
}

bool MPU6050::getInterruptLatchClear() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_RD_CLEAR_BIT);
}

void MPU6050::setInterruptLatchClear(bool clear) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_RD_CLEAR_BIT, clear);
}

bool MPU6050::getFSyncInterruptLevel() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT);
}

void MPU6050::setFSyncInterruptLevel(bool level) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT, level);
}

bool MPU6050::getFSyncInterruptEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_EN_BIT);
}

void MPU6050::setFSyncInterruptEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_EN_BIT, enabled);
}

bool MPU6050::getI2CBypassEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT);
}

void MPU6050::setI2CBypassEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}

bool MPU6050::getClockOutputEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_CLKOUT_EN_BIT);
}

void MPU6050::setClockOutputEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_CLKOUT_EN_BIT, enabled);
}

uint8_t MPU6050::getIntEnabled() {
  return mpu.readByte(MPU6050_RA_INT_ENABLE);
}

void MPU6050::setIntEnabled(uint8_t enabled) {
  mpu.writeByte(MPU6050_RA_INT_ENABLE, enabled);
}

bool MPU6050::getIntFreefallEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FF_BIT);
}

void MPU6050::setIntFreefallEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FF_BIT, enabled);
}

bool MPU6050::getIntMotionEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_MOT_BIT);
}

void MPU6050::setIntMotionEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_MOT_BIT, enabled);
}

bool MPU6050::getIntZeroMotionEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_ZMOT_BIT);
}

void MPU6050::setIntZeroMotionEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_ZMOT_BIT, enabled);
}

bool MPU6050::getIntFIFOBufferOverflowEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FIFO_OFLOW_BIT);
}

void MPU6050::setIntFIFOBufferOverflowEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FIFO_OFLOW_BIT, enabled);
}

bool MPU6050::getIntI2CMasterEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_I2C_MST_INT_BIT);
}

void MPU6050::setIntI2CMasterEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_I2C_MST_INT_BIT, enabled);
}

bool MPU6050::getIntDataReadyEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DATA_RDY_BIT);
}

void MPU6050::setIntDataReadyEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DATA_RDY_BIT, enabled);
}

uint8_t MPU6050::getIntStatus() {
  return mpu.readByte(MPU6050_RA_INT_STATUS);
}

bool MPU6050::getIntFreefallStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_FF_BIT);
}

bool MPU6050::getIntMotionStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_MOT_BIT);
}

bool MPU6050::getIntZeroMotionStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_ZMOT_BIT);
}

bool MPU6050::getIntFIFOBufferOverflowStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_FIFO_OFLOW_BIT);
}

bool MPU6050::getIntI2CMasterStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_I2C_MST_INT_BIT);
}

bool MPU6050::getIntDataReadyStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_DATA_RDY_BIT);
}

void MPU6050::getMotion9(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz, int16_t* mx, int16_t* my, int16_t* mz) {
  getMotion6(ax, ay, az, gx, gy, gz);
  // TODO: magnetometer integration
}

void MPU6050::getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
  uint8_t buffer[14];
  mpu.readBytes(MPU6050_RA_ACCEL_XOUT_H, sizeof(buffer), buffer);
  *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
  *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
  *az = (((int16_t)buffer[4]) << 8) | buffer[5];
  *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
  *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
  *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

void MPU6050::getAcceleration(int16_t* x, int16_t* y, int16_t* z) {
  uint8_t buffer[6];
  mpu.readBytes(MPU6050_RA_ACCEL_XOUT_H, sizeof(buffer), buffer);
  *x = (((int16_t)buffer[0]) << 8) | buffer[1];
  *y = (((int16_t)buffer[2]) << 8) | buffer[3];
  *z = (((int16_t)buffer[4]) << 8) | buffer[5];
}

int16_t MPU6050::getAccelerationX() {
  return mpu.readWord(MPU6050_RA_ACCEL_XOUT_H);
}

int16_t MPU6050::getAccelerationY() {
  return mpu.readWord(MPU6050_RA_ACCEL_YOUT_H);
}

int16_t MPU6050::getAccelerationZ() {
  return mpu.readWord(MPU6050_RA_ACCEL_ZOUT_H);
}

int16_t MPU6050::getTemperature() {
  return mpu.readWord(MPU6050_RA_TEMP_OUT_H);
}

void MPU6050::getRotation(int16_t* x, int16_t* y, int16_t* z) {
  uint8_t buffer[6];
  mpu.readBytes(MPU6050_RA_GYRO_XOUT_H, sizeof(buffer), buffer);
  *x = (((int16_t)buffer[0]) << 8) | buffer[1];
  *y = (((int16_t)buffer[2]) << 8) | buffer[3];
  *z = (((int16_t)buffer[4]) << 8) | buffer[5];
}

int16_t MPU6050::getRotationX() {
  return mpu.readWord(MPU6050_RA_GYRO_XOUT_H);
}

int16_t MPU6050::getRotationY() {
  return mpu.readWord(MPU6050_RA_GYRO_YOUT_H);
}

int16_t MPU6050::getRotationZ() {
  return mpu.readWord(MPU6050_RA_GYRO_ZOUT_H);
}

uint8_t MPU6050::getExternalSensorByte(int position) {
  return mpu.readByte(MPU6050_RA_EXT_SENS_DATA_00 + position);
}

uint16_t MPU6050::getExternalSensorWord(int position) {
  return mpu.readWord(MPU6050_RA_EXT_SENS_DATA_00);
}

uint32_t MPU6050::getExternalSensorDWord(int position) {
  uint8_t buffer[4];
  mpu.readBytes(MPU6050_RA_EXT_SENS_DATA_00 + position, 4, buffer);
  return (((uint32_t)buffer[0]) << 24) | (((uint32_t)buffer[1]) << 16) | (((uint16_t)buffer[2]) << 8) | buffer[3];
}

uint8_t MPU6050::getMotionStatus() {
  return mpu.readByte(MPU6050_RA_MOT_DETECT_STATUS);
}

bool MPU6050::getXNegMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_XNEG_BIT);
}

bool MPU6050::getXPosMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_XPOS_BIT);
}

bool MPU6050::getYNegMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_YNEG_BIT);
}

bool MPU6050::getYPosMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_YPOS_BIT);
}

bool MPU6050::getZNegMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZNEG_BIT);
}

bool MPU6050::getZPosMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZPOS_BIT);
}

bool MPU6050::getZeroMotionDetected() {
  return mpu.readByteBit(MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZRMOT_BIT);
}

void MPU6050::setSlaveOutputByte(uint8_t num, uint8_t data) {
  if (num > 3) return;
  mpu.writeByte(MPU6050_RA_I2C_SLV0_DO + num, data);
}

bool MPU6050::getExternalShadowDelayEnabled() {
  return mpu.readByteBit(MPU6050_RA_I2C_MST_DELAY_CTRL, MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT);
}

void MPU6050::setExternalShadowDelayEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_DELAY_CTRL, MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT, enabled);
}

bool MPU6050::getSlaveDelayEnabled(uint8_t num) {
  // MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT is 4, SLV3 is 3, etc.
  if (num > 4) return 0;
  return mpu.readByteBit(MPU6050_RA_I2C_MST_DELAY_CTRL, num);
}

void MPU6050::setSlaveDelayEnabled(uint8_t num, bool enabled) {
  mpu.setByteBit(MPU6050_RA_I2C_MST_DELAY_CTRL, num, enabled);
}

void MPU6050::resetGyroscopePath() {
  mpu.setByteBit(MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_GYRO_RESET_BIT);
}

void MPU6050::resetAccelerometerPath() {
  mpu.setByteBit(MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_ACCEL_RESET_BIT);
}

void MPU6050::resetTemperaturePath() {
  mpu.setByteBit(MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_TEMP_RESET_BIT);
}

uint8_t MPU6050::getAccelerometerPowerOnDelay() {
  return mpu.readByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_ACCEL_ON_DELAY_BIT,
                          MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH);
}

void MPU6050::setAccelerometerPowerOnDelay(uint8_t delay) {
  mpu.setByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_ACCEL_ON_DELAY_BIT,
                  MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH, delay);
}

uint8_t MPU6050::getFreefallDetectionCounterDecrement() {
  return mpu.readByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_FF_COUNT_BIT, MPU6050_DETECT_FF_COUNT_LENGTH);
}

void MPU6050::setFreefallDetectionCounterDecrement(uint8_t decrement) {
  mpu.setByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_FF_COUNT_BIT, MPU6050_DETECT_FF_COUNT_LENGTH, decrement);
}

uint8_t MPU6050::getMotionDetectionCounterDecrement() {
  return mpu.readByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_MOT_COUNT_BIT, MPU6050_DETECT_MOT_COUNT_LENGTH);
}

void MPU6050::setMotionDetectionCounterDecrement(uint8_t decrement) {
  mpu.setByteBits(MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_MOT_COUNT_BIT, MPU6050_DETECT_MOT_COUNT_LENGTH, decrement);
}

bool MPU6050::getFIFOEnabled() {
  return mpu.readByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT);
}

void MPU6050::setFIFOEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, enabled);
}

bool MPU6050::getI2CMasterModeEnabled() {
  return mpu.readByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT);
}

void MPU6050::setI2CMasterModeEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}

void MPU6050::switchSPIEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_IF_DIS_BIT, enabled);
}

void MPU6050::resetFIFO() {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT);
}

void MPU6050::resetI2CMaster() {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT);
}

void MPU6050::resetSensors() {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_SIG_COND_RESET_BIT);
}

void MPU6050::reset() {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT);
}

bool MPU6050::getSleepEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT);
}

void MPU6050::setSleepEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

bool MPU6050::getWakeCycleEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CYCLE_BIT);
}

void MPU6050::setWakeCycleEnabled(bool enabled) {
  if (enabled) {
    mpu.setByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CYCLE_BIT);
    return;
  }
  mpu.clearByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CYCLE_BIT);
}

bool MPU6050::getTempSensorEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_TEMP_DIS_BIT) == 0;  // 1 is actually disabled here
}

void MPU6050::setTempSensorEnabled(bool enabled) {
  if (enabled) {
    mpu.clearByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_TEMP_DIS_BIT);
    return;
  }
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_TEMP_DIS_BIT);
}

uint8_t MPU6050::getClockSource() {
  return mpu.readByteBits(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH);
}

void MPU6050::setClockSource(uint8_t source) {
  mpu.setByteBits(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}

uint8_t MPU6050::getWakeFrequency() {
  return mpu.readByteBits(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_LP_WAKE_CTRL_BIT, MPU6050_PWR2_LP_WAKE_CTRL_LENGTH);
}

void MPU6050::setWakeFrequency(uint8_t frequency) {
  mpu.setByteBits(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_LP_WAKE_CTRL_BIT, MPU6050_PWR2_LP_WAKE_CTRL_LENGTH, frequency);
}

bool MPU6050::getStandbyXAccelEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XA_BIT);
}

void MPU6050::setStandbyXAccelEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XA_BIT, enabled);
}

bool MPU6050::getStandbyYAccelEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YA_BIT);
}

void MPU6050::setStandbyYAccelEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YA_BIT, enabled);
}

bool MPU6050::getStandbyZAccelEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZA_BIT);
}

void MPU6050::setStandbyZAccelEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZA_BIT, enabled);
}

bool MPU6050::getStandbyXGyroEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XG_BIT);
}

void MPU6050::setStandbyXGyroEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XG_BIT, enabled);
}

bool MPU6050::getStandbyYGyroEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YG_BIT);
}

void MPU6050::setStandbyYGyroEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YG_BIT, enabled);
}

bool MPU6050::getStandbyZGyroEnabled() {
  return mpu.readByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZG_BIT);
}

void MPU6050::setStandbyZGyroEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZG_BIT, enabled);
}

uint16_t MPU6050::getFIFOCount() {
  return mpu.readWord(MPU6050_RA_FIFO_COUNTH);
}

uint8_t MPU6050::getFIFOByte() {
  return mpu.readByte(MPU6050_RA_FIFO_R_W);
}

void MPU6050::getFIFOBytes(uint8_t *data, uint8_t length) {
  if (length > 0) {
    mpu.readBytes(MPU6050_RA_FIFO_R_W, length, data);
    return;
  }
  *data = 0;
}

void MPU6050::setFIFOByte(uint8_t data) {
  mpu.writeByte(MPU6050_RA_FIFO_R_W, data);
}

uint8_t MPU6050::getDeviceID() {
  return mpu.readByteBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH);
}

void MPU6050::setDeviceID(uint8_t id) {
  mpu.setByteBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, id);
}

// ======== UNDOCUMENTED/DMP REGISTERS/METHODS ========

// XG_OFFS_TC register

uint8_t MPU6050::getOTPBankValid() {
  return mpu.readByteBit(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT);
}

void MPU6050::setOTPBankValid(bool enabled) {
  mpu.setByteBit(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, enabled);
}

int8_t MPU6050::getXGyroOffsetTC() {
  return mpu.readByteBits(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}

void MPU6050::setXGyroOffsetTC(int8_t offset) {
  mpu.setByteBits(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// YG_OFFS_TC register

int8_t MPU6050::getYGyroOffsetTC() {
  return mpu.readByteBits(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}

void MPU6050::setYGyroOffsetTC(int8_t offset) {
  mpu.setByteBits(MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// ZG_OFFS_TC register

int8_t MPU6050::getZGyroOffsetTC() {
  return mpu.readByteBits(MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}

void MPU6050::setZGyroOffsetTC(int8_t offset) {
  mpu.setByteBits(MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// X_FINE_GAIN register

int8_t MPU6050::getXFineGain() {
  return mpu.readByte(MPU6050_RA_X_FINE_GAIN);
}

void MPU6050::setXFineGain(int8_t gain) {
  mpu.writeByte(MPU6050_RA_X_FINE_GAIN, gain);
}

// Y_FINE_GAIN register

int8_t MPU6050::getYFineGain() {
  return mpu.readByte(MPU6050_RA_Y_FINE_GAIN);
}

void MPU6050::setYFineGain(int8_t gain) {
  mpu.writeByte(MPU6050_RA_Y_FINE_GAIN, gain);
}

// Z_FINE_GAIN register

int8_t MPU6050::getZFineGain() {
  return mpu.readByte(MPU6050_RA_Z_FINE_GAIN);
}

void MPU6050::setZFineGain(int8_t gain) {
    mpu.writeByte(MPU6050_RA_Z_FINE_GAIN, gain);
}

// XA_OFFS_* registers

int16_t MPU6050::getXAccelOffset() {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_XA_OFFS_H : 0x77;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  return mpu.readWord(slave_address);
}

void MPU6050::setXAccelOffset(int16_t offset) {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_XA_OFFS_H : 0x77;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  mpu.writeWord(slave_address, offset);
}

// YA_OFFS_* register

int16_t MPU6050::getYAccelOffset() {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_YA_OFFS_H : 0x7A;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  return mpu.readWord(slave_address);
}

void MPU6050::setYAccelOffset(int16_t offset) {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_YA_OFFS_H : 0x7A;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  mpu.writeWord(slave_address, offset);
}

// ZA_OFFS_* register

int16_t MPU6050::getZAccelOffset() {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_ZA_OFFS_H : 0x7D;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  return mpu.readWord(slave_address);
}

void MPU6050::setZAccelOffset(int16_t offset) {
  uint8_t slave_address = getDeviceID() < 0x38 ? MPU6050_RA_ZA_OFFS_H : 0x7D;  // MPU6050,MPU9150 Vs MPU6500,MPU9250
  mpu.writeWord(slave_address, offset);
}

// XG_OFFS_USR* registers

int16_t MPU6050::getXGyroOffset() {
  return mpu.readWord(MPU6050_RA_XG_OFFS_USRH);
}

void MPU6050::setXGyroOffset(int16_t offset) {
  mpu.writeWord(MPU6050_RA_XG_OFFS_USRH, offset);
}

// YG_OFFS_USR* register

int16_t MPU6050::getYGyroOffset() {
  return mpu.readWord(MPU6050_RA_YG_OFFS_USRH);
}

void MPU6050::setYGyroOffset(int16_t offset) {
  mpu.writeWord(MPU6050_RA_YG_OFFS_USRH, offset);
}

// ZG_OFFS_USR* register

int16_t MPU6050::getZGyroOffset() {
  return mpu.readWord(MPU6050_RA_ZG_OFFS_USRH);
}

void MPU6050::setZGyroOffset(int16_t offset) {
  mpu.writeWord(MPU6050_RA_ZG_OFFS_USRH, offset);
}

// INT_ENABLE register (DMP functions)

bool MPU6050::getIntPLLReadyEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_PLL_RDY_INT_BIT);
}

void MPU6050::setIntPLLReadyEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_PLL_RDY_INT_BIT, enabled);
}

bool MPU6050::getIntDMPEnabled() {
  return mpu.readByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DMP_INT_BIT);
}

void MPU6050::setIntDMPEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DMP_INT_BIT, enabled);
}

// DMP_INT_STATUS

bool MPU6050::getDMPInt5Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_5_BIT);
}

bool MPU6050::getDMPInt4Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_4_BIT);
}

bool MPU6050::getDMPInt3Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_3_BIT);
}

bool MPU6050::getDMPInt2Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_2_BIT);
}

bool MPU6050::getDMPInt1Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_1_BIT);
}

bool MPU6050::getDMPInt0Status() {
  return mpu.readByteBit(MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_0_BIT);
}

// INT_STATUS register (DMP functions)

bool MPU6050::getIntPLLReadyStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_PLL_RDY_INT_BIT);
}

bool MPU6050::getIntDMPStatus() {
  return mpu.readByteBit(MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_DMP_INT_BIT);
}

// USER_CTRL register (DMP functions)

bool MPU6050::getDMPEnabled() {
  return mpu.readByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT);
}

void MPU6050::setDMPEnabled(bool enabled) {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
}

void MPU6050::resetDMP() {
  mpu.setByteBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT);
}

// BANK_SEL register

void MPU6050::setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank) {
  bank &= 0x1F;
  if (userBank) bank |= 0x20;
  if (prefetchEnabled) bank |= 0x40;
  mpu.writeByte(MPU6050_RA_BANK_SEL, bank);
}

// MEM_START_ADDR register

void MPU6050::setMemoryStartAddress(uint8_t address) {
  mpu.writeByte(MPU6050_RA_MEM_START_ADDR, address);
}

// MEM_R_W register

uint8_t MPU6050::readMemoryByte() {
  return mpu.readByte(MPU6050_RA_MEM_R_W);
}

void MPU6050::writeMemoryByte(uint8_t data) {
  mpu.writeByte(MPU6050_RA_MEM_R_W, data);
}

// void MPU6050::readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address) {
//     setMemoryBank(bank);
//     setMemoryStartAddress(address);
//     uint8_t chunkSize;
//     for (uint16_t i = 0; i < dataSize;) {
//         // determine correct chunk size according to bank position and data size
//         chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

//         // make sure we don't go past the data size
//         if (i + chunkSize > dataSize) chunkSize = dataSize - i;

//         // make sure this chunk doesn't go past the bank boundary (256 bytes)
//         if (chunkSize > 256 - address) chunkSize = 256 - address;

//         // read the chunk of data as specified
//         I2Cdev::readBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, data + i);

//         // increase byte index by [chunkSize]
//         i += chunkSize;

//         // uint8_t automatically wraps to 0 at 256
//         address += chunkSize;

//         // if we aren't done, update bank (if necessary) and address
//         if (i < dataSize) {
//             if (address == 0) bank++;
//             setMemoryBank(bank);
//             setMemoryStartAddress(address);
//         }
//     }
// }
// bool MPU6050::writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify, bool useProgMem) {
//     setMemoryBank(bank);
//     setMemoryStartAddress(address);
//     uint8_t chunkSize;
//     uint8_t *verifyBuffer = 0;
//     uint8_t *progBuffer = 0;
//     uint16_t i;
//     uint8_t j;
//     if (verify) verifyBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
//     if (useProgMem) progBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
//     for (i = 0; i < dataSize;) {
//         // determine correct chunk size according to bank position and data size
//         chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

//         // make sure we don't go past the data size
//         if (i + chunkSize > dataSize) chunkSize = dataSize - i;

//         // make sure this chunk doesn't go past the bank boundary (256 bytes)
//         if (chunkSize > 256 - address) chunkSize = 256 - address;

//         if (useProgMem) {
//             // write the chunk of data as specified
//             for (j = 0; j < chunkSize; j++) progBuffer[j] = pgm_read_byte(data + i + j);
//         } else {
//             // write the chunk of data as specified
//             progBuffer = (uint8_t *)data + i;
//         }

//         I2Cdev::writeBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, progBuffer);

//         // verify data if needed
//         if (verify && verifyBuffer) {
//             setMemoryBank(bank);
//             setMemoryStartAddress(address);
//             I2Cdev::readBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);
//             if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
//                 /*Serial.print("Block write verification error, bank ");
//                 Serial.print(bank, DEC);
//                 Serial.print(", address ");
//                 Serial.print(address, DEC);
//                 Serial.print("!\nExpected:");
//                 for (j = 0; j < chunkSize; j++) {
//                     Serial.print(" 0x");
//                     if (progBuffer[j] < 16) Serial.print("0");
//                     Serial.print(progBuffer[j], HEX);
//                 }
//                 Serial.print("\nReceived:");
//                 for (uint8_t j = 0; j < chunkSize; j++) {
//                     Serial.print(" 0x");
//                     if (verifyBuffer[i + j] < 16) Serial.print("0");
//                     Serial.print(verifyBuffer[i + j], HEX);
//                 }
//                 Serial.print("\n");*/
//                 free(verifyBuffer);
//                 if (useProgMem) free(progBuffer);
//                 return false; // uh oh.
//             }
//         }

//         // increase byte index by [chunkSize]
//         i += chunkSize;

//         // uint8_t automatically wraps to 0 at 256
//         address += chunkSize;

//         // if we aren't done, update bank (if necessary) and address
//         if (i < dataSize) {
//             if (address == 0) bank++;
//             setMemoryBank(bank);
//             setMemoryStartAddress(address);
//         }
//     }
//     if (verify) free(verifyBuffer);
//     if (useProgMem) free(progBuffer);
//     return true;
// }
// bool MPU6050::writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify) {
//     return writeMemoryBlock(data, dataSize, bank, address, verify, true);
// }
// bool MPU6050::writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, bool useProgMem) {
//     uint8_t *progBuffer = 0;
// 	uint8_t success, special;
//     uint16_t i, j;
//     if (useProgMem) {
//         progBuffer = (uint8_t *)malloc(8); // assume 8-byte blocks, realloc later if necessary
//     }

//     // config set data is a long string of blocks with the following structure:
//     // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
//     uint8_t bank, offset, length;
//     for (i = 0; i < dataSize;) {
//         if (useProgMem) {
//             bank = pgm_read_byte(data + i++);
//             offset = pgm_read_byte(data + i++);
//             length = pgm_read_byte(data + i++);
//         } else {
//             bank = data[i++];
//             offset = data[i++];
//             length = data[i++];
//         }

//         // write data or perform special action
//         if (length > 0) {
//             // regular block of data to write
//             /*Serial.print("Writing config block to bank ");
//             Serial.print(bank);
//             Serial.print(", offset ");
//             Serial.print(offset);
//             Serial.print(", length=");
//             Serial.println(length);*/
//             if (useProgMem) {
//                 if (sizeof(progBuffer) < length) progBuffer = (uint8_t *)realloc(progBuffer, length);
//                 for (j = 0; j < length; j++) progBuffer[j] = pgm_read_byte(data + i + j);
//             } else {
//                 progBuffer = (uint8_t *)data + i;
//             }
//             success = writeMemoryBlock(progBuffer, length, bank, offset, true);
//             i += length;
//         } else {
//             // special instruction
//             // NOTE: this kind of behavior (what and when to do certain things)
//             // is totally undocumented. This code is in here based on observed
//             // behavior only, and exactly why (or even whether) it has to be here
//             // is anybody's guess for now.
//             if (useProgMem) {
//                 special = pgm_read_byte(data + i++);
//             } else {
//                 special = data[i++];
//             }
//             /*Serial.print("Special command code ");
//             Serial.print(special, HEX);
//             Serial.println(" found...");*/
//             if (special == 0x01) {
//                 // enable DMP-related interrupts

//                 //setIntZeroMotionEnabled(true);
//                 //setIntFIFOBufferOverflowEnabled(true);
//                 //setIntDMPEnabled(true);
//                 mpu.writeByte(MPU6050_RA_INT_ENABLE, 0x32);  // single operation

//                 success = true;
//             } else {
//                 // unknown special command
//                 success = false;
//             }
//         }

//         if (!success) {
//             if (useProgMem) free(progBuffer);
//             return false; // uh oh
//         }
//     }
//     if (useProgMem) free(progBuffer);
//     return true;
// }
// bool MPU6050::writeProgDMPConfigurationSet(const uint8_t *data, uint16_t dataSize) {
//     return writeDMPConfigurationSet(data, dataSize, true);
// }

// // DMP_CFG_1 register

// uint8_t MPU6050::getDMPConfig1() {
//     return mpu.readByte(MPU6050_RA_DMP_CFG_1);
//
// }
// void MPU6050::setDMPConfig1(uint8_t config) {
//     mpu.writeByte(MPU6050_RA_DMP_CFG_1, config);
// }

// // DMP_CFG_2 register

// uint8_t MPU6050::getDMPConfig2() {
//     return mpu.readByte(MPU6050_RA_DMP_CFG_2);
//
// }
// void MPU6050::setDMPConfig2(uint8_t config) {
//     mpu.writeByte(MPU6050_RA_DMP_CFG_2, config);
// }


//***************************************************************************************
//**********************           Calibration Routines            **********************
//***************************************************************************************
/**
  @brief      Fully calibrate Gyro from ZERO in about 6-7 Loops 600-700 readings
*/
// void MPU6050::CalibrateGyro(uint8_t Loops ) {
//   double kP = 0.3;
//   double kI = 90;
//   float x;
//   x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
//   kP *= x;
//   kI *= x;

//   PID( 0x43,  kP, kI,  Loops);
// }

// /**
//   @brief      Fully calibrate Accel from ZERO in about 6-7 Loops 600-700 readings
// */
// void MPU6050::CalibrateAccel(uint8_t Loops ) {

// 	float kP = 0.3;
// 	float kI = 20;
// 	float x;
// 	x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
// 	kP *= x;
// 	kI *= x;
// 	PID( 0x3B, kP, kI,  Loops);
// }

// void MPU6050::PID(uint8_t ReadAddress, float kP,float kI, uint8_t Loops){
//   uint8_t SaveAddress = (ReadAddress == 0x3B)?((getDeviceID() < 0x38 )? 0x06:0x77):0x13;

//   int16_t  Data;
//   float Reading;
//   int16_t BitZero[3];
//   uint8_t shift =(SaveAddress == 0x77)?3:2;
//   float Error, PTerm, ITerm[3];
//   int16_t eSample;
//   uint32_t eSum;
//   Serial.write('>');
//   for (int i = 0; i < 3; i++) {
//   I2Cdev::readWords(devAddr, SaveAddress + (i * shift), 1, (uint16_t *)&Data);  // reads 1 or more 16 bit integers (Word)
//   Reading = Data;
//   if (SaveAddress != 0x13) {
//     BitZero[i] = Data & 1;  // Capture Bit Zero to properly handle Accelerometer calibration
//     ITerm[i] = ((float)Reading) * 8;
//     } else {
//     ITerm[i] = Reading * 4;
//   }
//   }
//   for (int L = 0; L < Loops; L++) {
// 	  eSample = 0;
// 	  for (int c = 0; c < 100; c++) {// 100 PI Calculations
// 		  eSum = 0;
// 		  for (int i = 0; i < 3; i++) {
// 			  I2Cdev::readWords(devAddr, ReadAddress + (i * 2), 1, (uint16_t *)&Data); // reads 1 or more 16 bit integers (Word)
// 			  Reading = Data;
// 			  if ((ReadAddress == 0x3B)&&(i == 2)) Reading -= 16384;	//remove Gravity
// 			  Error = -Reading;
// 			  eSum += abs(Reading);
// 			  PTerm = kP * Error;
// 			  ITerm[i] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
// 			  if(SaveAddress != 0x13){
// 				  Data = round((PTerm + ITerm[i] ) / 8);		//Compute PID Output
// 				  Data = ((Data)&0xFFFE) |BitZero[i];			// Insert Bit0 Saved at beginning
// 			  } else Data = round((PTerm + ITerm[i] ) / 4);	//Compute PID Output
// 			  I2Cdev::writeWords(devAddr, SaveAddress + (i * shift), 1, (uint16_t *)&Data);
// 		  }
// 		  if((c == 99) && eSum > 1000){						// Error is still to great to continue 
// 			  c = 0;
// 			  Serial.write('*');
// 		  }
// 		  if((eSum * ((ReadAddress == 0x3B)?.05: 1)) < 5) eSample++;	// Successfully found offsets prepare to  advance
// 		  if((eSum < 100) && (c > 10) && (eSample >= 10)) break;		// Advance to next Loop
// 		  delay(1);
// 	  }
// 	  Serial.write('.');
// 	  kP *= .75;
// 	  kI *= .75;
// 	  for (int i = 0; i < 3; i++){
// 		  if(SaveAddress != 0x13) {
// 			  Data = round((ITerm[i] ) / 8);		//Compute PID Output
// 			  Data = ((Data)&0xFFFE) |BitZero[i];	// Insert Bit0 Saved at beginning
// 		  } else Data = round((ITerm[i]) / 4);
// 		  I2Cdev::writeWords(devAddr, SaveAddress + (i * shift), 1, (uint16_t *)&Data);
// 	  }
//   }
//   resetFIFO();
//   resetDMP();
// }

// #define printfloatx(Name,Variable,Spaces,Precision,EndTxt) { Serial.print(F(Name)); {char S[(Spaces + Precision + 3)];Serial.print(F(" ")); Serial.print(dtostrf((float)Variable,Spaces,Precision ,S));}Serial.print(F(EndTxt)); }//Name,Variable,Spaces,Precision,EndTxt
// void MPU6050::PrintActiveOffsets() {
// 	uint8_t AOffsetRegister = (getDeviceID() < 0x38 )? MPU6050_RA_XA_OFFS_H:0x77;
// 	int16_t Data[3];
// 	//Serial.print(F("Offset Register 0x"));
// 	//Serial.print(AOffsetRegister>>4,HEX);Serial.print(AOffsetRegister&0x0F,HEX);
// 	Serial.print(F("\n//           X Accel  Y Accel  Z Accel   X Gyro   Y Gyro   Z Gyro\n//OFFSETS   "));
// 	if(AOffsetRegister == 0x06)	I2Cdev::readWords(devAddr, AOffsetRegister, 3, (uint16_t *)Data);
// 	else {
// 		I2Cdev::readWords(devAddr, AOffsetRegister, 1, (uint16_t *)Data);
// 		I2Cdev::readWords(devAddr, AOffsetRegister+3, 1, (uint16_t *)Data+1);
// 		I2Cdev::readWords(devAddr, AOffsetRegister+6, 1, (uint16_t *)Data+2);
// 	}
// 	//	A_OFFSET_H_READ_A_OFFS(Data);
// 	printfloatx("", Data[0], 5, 0, ",  ");
// 	printfloatx("", Data[1], 5, 0, ",  ");
// 	'printfloatx("", Data[2], 5, 0, ",  ");
// 	'I2Cdev::readWords(devAddr, 0x13, 3, (uint16_t *)Data);
// '	//	XG_OFFSET_H_READ_OFFS_USR(Data);
// 	printfloatx("", Data[0], 5, 0, ",  ");
// 	printfloatx("", Data[1], 5, 0, ",  ");
// 	printfloatx("", Data[2], 5, 0, "\n");
// }
