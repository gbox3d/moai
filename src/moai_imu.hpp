#ifndef _MOAI_IMU_HPP_
#define _MOAI_IMU_HPP_

// namespace arhs
// {
//     void setup();
//     // void loop(float *pData);
//     bool updateXYZ(float *pData);
//     bool updateXZY(float *pData);

//     void calibration();

//     float getGyroXoffset();
//     float getGyroYoffset();
//     float getGyroZoffset();

//     void setGyroOffsets(float x, float y, float z);

//     void resetFilter();
// }

void initDmp(int16_t *pOffset);

void closeDmp();
bool getDmpReady();
bool getQuaternion(float *pQuaternion);
bool getYPR(float *pYPR);
bool getAccel(float *pAccel);
bool getAccYpr(float *pData);

int16_t *GetActiveOffset();
void doZero();

#endif