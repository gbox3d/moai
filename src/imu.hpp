
namespace BNO080_IMU
{
    bool begin();

    namespace quaternion {
        // void ready();
        void setReports(void);
        void update();
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