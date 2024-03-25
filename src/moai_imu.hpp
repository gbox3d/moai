#ifndef _MOAI_IMU_HPP_
#define _MOAI_IMU_HPP_


namespace BNO080_IMU
{
    bool begin();

    namespace quaternion {
        // void ready();
        void setReports(void);
        // void update();
        void update(float *imuData);
    }

    namespace linear_acceleration {
        void setReports(void);
        void update();
    }

    namespace trace {
        void setReports(void);
        void update();
    }

    namespace euler {
        void setReports(void);
        void update();
    }


}

#endif