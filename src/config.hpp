#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Preferences.h>

class CCongifData
{
    Preferences preferences;

public:
    String mStrAp;
    String mStrPassword;

    int mDeviceNumber;
    String mTargetIp;
    int mTargetPort;
    int16_t mOffsets[6];
    int mTriggerDelay;

    bool save()
    {
        preferences.begin("config", false);
        preferences.putString("mStrAp", mStrAp);
        preferences.putString("mStrPassword", mStrPassword);
        preferences.putInt("mDeviceNumber", mDeviceNumber);
        preferences.putString("mTargetIp", mTargetIp);
        preferences.putInt("mTargetPort", mTargetPort);
        preferences.putInt("mTriggerDelay", mTriggerDelay);

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

        mStrAp = preferences.getString("mStrAp", "");
        mStrPassword = preferences.getString("mStrPassword", "");

        mTargetIp = preferences.getString("mTargetIp", "");
        mTargetPort = preferences.getInt("mTargetPort", 0);
        mDeviceNumber = preferences.getInt("mDeviceNumber", 0);
        mTriggerDelay = preferences.getInt("mTriggerDelay", 150);

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
        return String("mStrAp: ") + mStrAp + "\n" +
               String("mStrPassword: ") + mStrPassword + "\n" +
               String("mTargetIp: ") + mTargetIp + "\n" +
               String("mTargetPort: ") + String(mTargetPort) + "\n" +
               String("mDeviceNumber: ") + String(mDeviceNumber) + "\n" +
               String("mTriggerDelay: ") + String(mTriggerDelay) + "\n" +
               String("mOffsets: ") + "\n" +
               String("offset0: ") + String(mOffsets[0]) + "\n" +
               String("offset1: ") + String(mOffsets[1]) + "\n" +
               String("offset2: ") + String(mOffsets[2]) + "\n" +
               String("offset3: ") + String(mOffsets[3]) + "\n" +
               String("offset4: ") + String(mOffsets[4]) + "\n" +
               String("offset5: ") + String(mOffsets[5]) + "\n";
    }
};

#endif // CONFIG_HPP