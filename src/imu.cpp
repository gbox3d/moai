
#include <Wire.h>

// #include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
#include "SparkFun_BNO08x_Arduino_Library.h"

#define BNO08X_ADDR 0x4B // SparkFun BNO08x Breakout (Qwiic) defaults to 0x4B
// #define BNO08X_ADDR 0x4A // Alternate address if ADR jumper is closed

#if defined(LOLIN_D32)
byte imuINTPin = 19;
#elif defined(SEED_XIAO_ESP32C3)
byte imuINTPin = D3;
#elif defined(WROVER_KIT)
// byte imuINTPin = 19;
#define BNO08X_INT 19
// #define BNO08X_INT  -1
#define BNO08X_RST 18
// #define BNO08X_RST  -1
#endif

namespace BNO080_IMU
{
    BNO08x myIMU;
    // indicators of new data availability
    volatile byte newQuat = 0;
    volatile byte newLinAcc = 0;
    // internal copies of the IMU data
    // float ax, ay, az, qx, qy, qz, qw; // (qx, qy, qz, qw = i,j,k, real)
    // byte linAccuracy = 0;
    // float quatRadianAccuracy = 0;
    // byte quatAccuracy = 0;
    // pin used for interrupts

    float ax, ay, az, gx, gy, gz, qx, qy, qz, qw; //  mx, my, mz, (qx, qy, qz, qw = i,j,k, real)
    byte linAccuracy = 0;
    byte gyroAccuracy = 0;
    // byte magAccuracy = 0;
    float quatRadianAccuracy = 0;
    byte quatAccuracy = 0; // 0 = unreliable, 1 = low, 2 = medium, 3 = high
    unsigned long timeStamp;

    bool begin()
    {
        Wire.begin();
        delay(100);

        if (myIMU.begin(BNO08X_ADDR, Wire, BNO08X_INT, BNO08X_RST) == false)
        {
            // Serial.println("BNO08x not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
            return false;
        }

        return true;
    }

    namespace quaternion
    {
        void setReports(void)
        {
            Serial.println("Setting desired reports");
            if (myIMU.enableRotationVector() == true)
            {
                Serial.println(F("Rotation vector enabled"));
                Serial.println(F("Output in form i, j, k, real, accuracy"));
            }
            else
            {
                Serial.println("Could not enable rotation vector");
            }
        }

        void update(float *imuData)
        {
            if (myIMU.wasReset())
            {
                Serial.println("sensor was reset -----------------");
                setReports();

                delay(500);
            }

            // Has a new event come in on the Sensor Hub Bus?
            if (myIMU.getSensorEvent() == true)
            {

                // is it the correct sensor data we want?
                if (myIMU.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR)
                {

                    timeStamp = myIMU.getTimeStamp();

                    qx = myIMU.getQuatI();
                    qy = myIMU.getQuatJ();
                    qz = myIMU.getQuatK();
                    qw = myIMU.getQuatReal();
                    quatRadianAccuracy = myIMU.getQuatRadianAccuracy();
                    
                    imuData[6] = qx;
                    imuData[7] = qy;
                    imuData[8] = qz;
                    imuData[9] = qw;

                    // Serial.printf("Quat: %2.2f %2.2f %2.2f %2.2f %2.2f %lu\n", qx, qy, qz, qw, quatRadianAccuracy, timeStamp);
                }
            }
        }
    }

    namespace linear_acceleration
    {
        void setReports(void)
        {
            Serial.println("Setting desired reports");
            if (myIMU.enableLinearAccelerometer() == true)
            {
                Serial.println(F("LinearAccelerometer enabled, Output in form x, y, z, accuracy, in m/s^2"));
            }
            else
            {
                Serial.println("Could not enable linear accelerometer");
            }
        }

        void update()
        {
            if (myIMU.wasReset())
            {
                Serial.print("sensor was reset ");
                setReports();
            }

            // Has a new event come in on the Sensor Hub Bus?
            if (myIMU.getSensorEvent() == true)
            {

                // is it the correct sensor data we want?
                if (myIMU.getSensorEventID() == SENSOR_REPORTID_LINEAR_ACCELERATION)
                {

                    uint64_t timeStamp = myIMU.getTimeStamp();

                    ax = myIMU.getLinAccelX();
                    ay = myIMU.getLinAccelY();
                    az = myIMU.getLinAccelZ();
                    linAccuracy = myIMU.getLinAccelAccuracy();

                    Serial.printf("LinAccel: %2.2f %2.2f %2.2f %d %lu\n", ax, ay, az, linAccuracy, timeStamp);
                }
            }
        }
    }

    namespace trace
    {
        float x = 0, y = 0, z = 0;        // 현재 위치
        float vx = 0, vy = 0, vz = 0;     // 현재 속도
        unsigned long previousMicros = 0; // 마지막 업데이트 시각
        float ax = 0, ay = 0, az = 0;     // 현재 가속도
        byte linAccuracy;                 // 가속도 측정 정확도

        void setReports(void)
        {
            Serial.println("Setting desired reports");
            if (myIMU.enableLinearAccelerometer() == true)
            {
                Serial.println(F("Linear Accelerometer enabled, Output in form x, y, z, accuracy, in m/s^2"));
            }
            else
            {
                Serial.println("Could not enable linear accelerometer");
            }
        }

        void update()
        {
            if (myIMU.wasReset())
            {
                Serial.print("sensor was reset ");
                setReports();
            }

            // 새로운 센서 이벤트가 있는지 확인
            if (myIMU.getSensorEvent() == true)
            {
                // 선형 가속도 데이터인지 확인
                if (myIMU.getSensorEventID() == SENSOR_REPORTID_LINEAR_ACCELERATION)
                {
                    unsigned long currentMicros = micros();
                    float deltaTime = (currentMicros - previousMicros) / 1000000.0f; // 초 단위로 변환
                    previousMicros = currentMicros;

                    ax = myIMU.getLinAccelX();
                    ay = myIMU.getLinAccelY();
                    az = myIMU.getLinAccelZ();
                    linAccuracy = myIMU.getLinAccelAccuracy();

                    // 속도 업데이트 (v = v0 + at)
                    vx += ax * deltaTime;
                    vy += ay * deltaTime;
                    vz += az * deltaTime;

                    // 위치 업데이트 (x = x0 + vt)
                    x += vx * deltaTime;
                    y += vy * deltaTime;
                    z += vz * deltaTime;

                    Serial.printf("Position: %2.2f, %2.2f, %2.2f\n", x, y, z);
                }
            }
        }

    }

    namespace euler
    {
        void setReports(void)
        {
            Serial.println("Setting desired reports");
            if (myIMU.enableRotationVector() == true)
            {
                Serial.println(F("Rotation vector enabled"));
                Serial.println(F("Output in form roll, pitch, yaw"));
            }
            else
            {
                Serial.println("Could not enable rotation vector");
            }
        }

        void update()
        {
            if (myIMU.wasReset())
            {
                Serial.print("sensor was reset ");
                setReports();
            }

            // Has a new event come in on the Sensor Hub Bus?
            if (myIMU.getSensorEvent() == true)
            {

                // is it the correct sensor data we want?
                if (myIMU.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR)
                {

                    float roll = (myIMU.getRoll()) * 180.0 / PI;   // Convert roll to degrees
                    float pitch = (myIMU.getPitch()) * 180.0 / PI; // Convert pitch to degrees
                    float yaw = (myIMU.getYaw()) * 180.0 / PI;     // Convert yaw / heading to degrees

                    Serial.print(roll, 1);
                    Serial.print(F(","));
                    Serial.print(pitch, 1);
                    Serial.print(F(","));
                    Serial.print(yaw, 1);

                    Serial.println();
                }
            }
        }
    }

    /*
        void ready_raw()
        {
            myIMU.enableAccelerometer(50);    // We must enable the accel in order to get MEMS readings even if we don't read the reports.
            myIMU.enableRawAccelerometer(50); // Send data update every 50ms
            myIMU.enableGyro(50);
            myIMU.enableRawGyro(50);
            myIMU.enableMagnetometer(50);
            myIMU.enableRawMagnetometer(50);

            Serial.println(F("Raw MEMS readings enabled"));
            Serial.println(F("Output is: (accel) x y z (gyro) x y z (mag) x y z"));
        }

        void updateData_raw()
        {
            // Look for reports from the IMU
            if (myIMU.dataAvailable() == true)
            {
                float ax = myIMU.getRawAccelX();
                float ay = myIMU.getRawAccelY();
                float az = myIMU.getRawAccelZ();

                float gx = myIMU.getRawGyroX();
                float gy = myIMU.getRawGyroY();
                float gz = myIMU.getRawGyroZ();

                float mx = myIMU.getRawMagX();
                float my = myIMU.getRawMagY();
                float mz = myIMU.getRawMagZ();

                Serial.printf("Raw Accel: %f %f %f Raw Gyro : %f %f %f Raw Mag: %f %f %f\n", ax, ay, az, gx, gy, gz, mx, my, mz);
            }
        }

        // 참고소스
        // https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/blob/main/examples/Example18-AccessMultiple/Example18-AccessMultiple.ino
        void ready_all()
        {

            myIMU.enableLinearAccelerometer(100); // m/s^2 no gravity
            myIMU.enableRotationVector(100);      // quat
            myIMU.enableGyro(100);                // rad/s

            Serial.println(F("LinearAccelerometer enabled, Output in form x, y, z, accuracy, in m/s^2"));
            Serial.println(F("Gyro enabled, Output in form x, y, z, accuracy, in radians per second"));
            Serial.println(F("Rotation vector, Output in form i, j, k, real, accuracy"));
        }

        void updateData_all()
        {
            // Look for reports from the IMU
            if (myIMU.dataAvailable() == true)
            {
                // get IMU data in one go for each sensor type
                myIMU.getLinAccel(ax, ay, az, linAccuracy);
                myIMU.getGyro(gx, gy, gz, gyroAccuracy);
                myIMU.getQuat(qx, qy, qz, qw, quatRadianAccuracy, quatAccuracy);

                Serial.printf("LinAccel: %f %f %f %d Gyro: %f %f %f %d Quat: %f %f %f %f %f %d\n", ax, ay, az, linAccuracy, gx, gy, gz, gyroAccuracy, qx, qy, qz, qw, quatRadianAccuracy, quatAccuracy);
            }
        }

        void ready_euler()
        {
            myIMU.enableRotationVector(50); // Send data update every 50ms

            Serial.println(F("Rotation vector enabled"));
            Serial.println(F("Output in form i, j, k, real, accuracy"));
        }

        void updateData_euler()
        {
            // Look for reports from the IMU
            if (myIMU.dataAvailable() == true)
            {
                float roll = (myIMU.getRoll()) * 180.0 / PI;   // Convert roll to degrees
                float pitch = (myIMU.getPitch()) * 180.0 / PI; // Convert pitch to degrees
                float yaw = (myIMU.getYaw()) * 180.0 / PI;     // Convert yaw / heading to degrees

                Serial.printf("Euler: %3.4f %3.4f %3.4f\n", roll, pitch, yaw);
            }
        }

        void ready_linAcc()
        {
            myIMU.enableLinearAccelerometer(50); // m/s^2 no gravity, data update every 50 ms

            Serial.println(F("LinearAccelerometer enabled, Output in form x, y, z, accuracy, in m/s^2"));
        }

        void updateData_linAcc()
        {
            // Look for reports from the IMU
            if (myIMU.dataAvailable() == true)
            {
                timeStamp = myIMU.getTimeStamp();
                myIMU.getLinAccel(ax, ay, az, linAccuracy); // m/s^2

                Serial.printf("LinAccel: %3.2f %3.2f %3.2f %d %lu\n", ax, ay, az, linAccuracy, timeStamp);
            }
        }

        // This function is called whenever an interrupt is detected by the arduino
        void interrupt_handler()
        {
            // code snippet from ya-mouse
            switch (myIMU.getReadings())
            {
            case SENSOR_REPORTID_LINEAR_ACCELERATION:
            {
                newLinAcc = 1;
            }
            break;

            case SENSOR_REPORTID_ROTATION_VECTOR:
            case SENSOR_REPORTID_GAME_ROTATION_VECTOR:
            {
                newQuat = 1;
            }
            break;
            default:
                // Unhandled Input Report
                break;
            }
        }

        void ready_interrupt()
        {
            // prepare interrupt on falling edge (= signal of new data available)
            attachInterrupt(digitalPinToInterrupt(imuINTPin), interrupt_handler, FALLING);
            // enable interrupts right away to not miss first reports
            interrupts();

            myIMU.enableLinearAccelerometer(50); // m/s^2 no gravity, data update every 50 ms
            myIMU.enableRotationVector(100);     // Send data update every 100 ms

            Serial.println(F("LinearAccelerometer enabled, Output in form x, y, z, accuracy, in m/s^2"));
            Serial.println(F("Rotation vector, Output in form i, j, k, real, accuracy"));
        }

        void updateData_interrupt()
        {
            if (newLinAcc)
            {
                myIMU.getLinAccel(ax, ay, az, linAccuracy);
                Serial.printf("LinAccel: %f %f %f %d\n", ax, ay, az, linAccuracy);
                newLinAcc = 0;
            }
            if (newQuat)
            {
                myIMU.getQuat(qx, qy, qz, qw, quatRadianAccuracy, quatAccuracy);
                Serial.printf("Quat: %f %f %f %f %f %d\n", qx, qy, qz, qw, quatRadianAccuracy, quatAccuracy);
                newQuat = 0;
            }
        }

        // tare the IMU
        // what is tare : https://en.wikipedia.org/wiki/Tare_weight
        namespace Tare
        {
            void Now(bool zAxisOnly = false)
            {
                myIMU.tareNow(zAxisOnly);
            }
            void Save()
            {
                myIMU.saveTare();
            }
            void Clear()
            {
                myIMU.clearTare();
            }
        }

        void calibrate()
        {
            // calibrate the IMU
            myIMU.calibrateAll();

            Serial.println("Calibrating...");

            // wait for calibration to finish
            while (!myIMU.calibrationComplete())
            {
                Serial.print(".");

                delay(1000);
            }
            Serial.println("Calibration complete");
        }
    */

}