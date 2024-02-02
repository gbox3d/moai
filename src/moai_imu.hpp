#ifndef _MOAI_IMU_HPP_
#define _MOAI_IMU_HPP_

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