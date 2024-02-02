#ifndef _MOAI_IMU_HPP_
#define _MOAI_IMU_HPP_

namespace arhs
{
    void setup();
    void setup(float gyOffsetX, float gyOffsetY, float gyOffsetZ);
    // void loop(float *pData);
    bool updateXYZ(float *pData);
    bool updateXZY(float *pData);

    void calibration();

    float getGyroXoffset();
    float getGyroYoffset();
    float getGyroZoffset();

    void setGyroOffsets(float x, float y, float z);

    void resetFilter();
}

#endif