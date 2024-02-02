#include <Wire.h>
#include <MPU6050_tockn.h>
#include <MadgwickAHRS.h>

MPU6050 mpu6050(Wire);
Madgwick filter;
unsigned long microsPerReading, microsPrevious;

namespace arhs
{
    void setup()
    {
        // Serial.begin(115200);
        // Wire.end();

        Wire.begin();

        mpu6050.begin();
        
        filter.begin(25); // 25Hz sample rate

        // initialize variables to pace updates to correct rate
        microsPerReading = 1000000 / 25; // 40000 microseconds/reading = 25Hz
        microsPrevious = micros();
    }

    void setup(float gyOffsetX, float gyOffsetY, float gyOffsetZ)
    {   
        // initialize variables to pace updates to correct rate
        microsPerReading = 1000000 / 25; // 40000 microseconds/reading = 25Hz
        microsPrevious = micros();

        Wire.begin();
        mpu6050.begin();

        filter.begin(25); // 25Hz sample rate
        mpu6050.setGyroOffsets(gyOffsetX, gyOffsetY, gyOffsetZ);
        // mpu6050.writeMPU6050(MPU6050_SMPLRT_DIV, 0x00);
        // mpu6050.writeMPU6050(MPU6050_CONFIG, 0x00);
        mpu6050.writeMPU6050(MPU6050_GYRO_CONFIG, 0x08); // 새로운 자이로 오브셋값을 적용하기 위해 0x08로 설정  
        // mpu6050.writeMPU6050(MPU6050_ACCEL_CONFIG, 0x00);
        // mpu6050.writeMPU6050(MPU6050_PWR_MGMT_1, 0x01);

    }   


    void stop()
    {
        Wire.end();
    }

    void resetFilter() {

        mpu6050.begin();
        // mpu6050.begin();
        filter.begin(25); // 25Hz sample rate
        // initialize variables to pace updates to correct rate
        microsPerReading = 1000000 / 25; // 40000 microseconds/reading = 25Hz
        microsPrevious = micros();
    }

    void calibration() {
        mpu6050.calcGyroOffsets(true);
    }

    float getGyroXoffset() {
        return mpu6050.getGyroXoffset();
    }
    float getGyroYoffset() {
        return mpu6050.getGyroYoffset();
    }
    float getGyroZoffset() {
        return mpu6050.getGyroZoffset();
    }

    void setGyroOffsets(float x, float y, float z) {
        mpu6050.setGyroOffsets(x, y, z);
    }

    bool updateXYZ(float *pData)
    {
        unsigned long microsNow = micros();

        // 25Hz 로 센서값 읽기
        if (microsNow - microsPrevious >= microsPerReading)
        {
            mpu6050.update();

            float ax = mpu6050.getAccX();
            float ay = mpu6050.getAccY();
            float az = mpu6050.getAccZ();

            float gx = mpu6050.getGyroX();
            float gy = mpu6050.getGyroY();
            float gz = mpu6050.getGyroZ();

            
            // Update Madgwick filter
            filter.updateIMU(gx, gy, gz, ax, ay, az);

            // Get Euler angles (roll, pitch, yaw)
            // float roll = filter.getRoll();
            // float pitch = filter.getPitch();
            // float yaw = filter.getYaw();

            pData[0] = ax;
            pData[1] = ay;
            pData[2] = az;
            
            // pData[3] = yaw;
            // pData[4] = pitch;
            // pData[5] = roll;

            filter.getQuaternion(pData + 6);

            // Print angles with Serial.printf, right-aligned and 6 digits (3 decimal places)
            // Serial.printf("Roll: %6.3f\tPitch: %6.3f\tYaw: %6.3f\n", roll, pitch, yaw);

            // increment previous time, so we keep proper pace
            microsPrevious = microsPrevious + microsPerReading;
            return true;
        }
        return false;
    }

/*

//굳이 이렇게 할필요는 소프트웨어적으로 처리하는게 좋을듯

    bool updateXZY(float *pData)
    {
        unsigned long microsNow = micros();

        // 25Hz 로 센서값 읽기
        if (microsNow - microsPrevious >= microsPerReading)
        {
            mpu6050.update();
            //xzy
            float ax = mpu6050.getAccX();
            float ay = mpu6050.getAccZ();
            float az = mpu6050.getAccY();

            float gx = mpu6050.getGyroX();
            float gy = mpu6050.getGyroZ();
            float gz = mpu6050.getGyroY();

            
            // Update Madgwick filter
            filter.updateIMU(gx, gy, gz, ax, ay, az);

            // Get Euler angles (roll, pitch, yaw)
            float roll = -filter.getRoll();
            float pitch = -filter.getPitch();
            float yaw = -filter.getYaw();

            pData[0] = ax;
            pData[1] = ay;
            pData[2] = az;
            
            pData[3] = yaw;
            pData[4] = pitch;
            pData[5] = roll;

            // Print angles with Serial.printf, right-aligned and 6 digits (3 decimal places)
            // Serial.printf("Roll: %6.3f\tPitch: %6.3f\tYaw: %6.3f\n", roll, pitch, yaw);

            // increment previous time, so we keep proper pace
            microsPrevious = microsPrevious + microsPerReading;
            return true;
        }
        return false;
    }
    */
}