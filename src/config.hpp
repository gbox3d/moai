#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Preferences.h>

class CCongifData
{
    Preferences preferences;

public:

    int mDeviceNumber;
    String mTargetIp;
    int mTargetPort;
    int16_t mOffsets[6];

    bool save()
    {
        preferences.begin("config", false);
        preferences.putInt("mDeviceNumber", mDeviceNumber);
        preferences.putString("mTargetIp", mTargetIp);
        preferences.putInt("mTargetPort", mTargetPort);

        for (int i = 0; i < 6; i++)
        {
            preferences.putInt((String("offset") + i).c_str(), mOffsets[i]);
        }

        preferences.end();
        return true;
    }

    bool load()
    {
        preferences.begin("config", true);

        mTargetIp = preferences.getString("mTargetIp", "");
        mTargetPort = preferences.getInt("mTargetPort", 0);
        mDeviceNumber = preferences.getInt("mDeviceNumber", 0);

        for (int i = 0; i < 6; i++)
        {
            mOffsets[i] = preferences.getInt((String("offset") + i).c_str(), 0);
        }

        preferences.end();
        return true;
    }

    bool clear()
    {
        preferences.begin("config", false);
        preferences.clear();
        preferences.end();
        return true;
    }
    String dump()
    {
        return String("mTargetIp: ") + mTargetIp + "\n" +
               String("mTargetPort: ") + String(mTargetPort) + "\n" +
               String("mDeviceNumber: ") + String(mDeviceNumber) + "\n" +
               String("mOffsets: ") + "\n" +
               String("offset0: ") + String(mOffsets[0]) + "\n" +
               String("offset1: ") + String(mOffsets[1]) + "\n" +
               String("offset2: ") + String(mOffsets[2]) + "\n" +
               String("offset3: ") + String(mOffsets[3]) + "\n" +
               String("offset4: ") + String(mOffsets[4]) + "\n" +
               String("offset5: ") + String(mOffsets[5]) + "\n";

        // Serial.println("mTargetIp: " + mTargetIp);
        // Serial.println("mTargetPort: " + String(mTargetPort));
        // Serial.println("mDeviceNumber: " + String(mDeviceNumber));
        // Serial.println("mOffsets: ");
        // for (int i = 0; i < 6; i++)
        // {
        //     Serial.println((String("offset") + i + ": ").c_str() + String(mOffsets[i]));
        // }
    }
};

#endif // CONFIG_HPP