#ifndef TONKEY_HPP
#define TONKEY_HPP

// #define MAXTOKENS 64

class tonkey
{
private:
    static const int MAXTOKENS = 64;
    /* data */
    String mTokens[MAXTOKENS];
    int mTokenCount = 0;

public:
    tonkey(/* args */) {}
    ~tonkey() {}

    // void parse(String _strLine);
    inline static int getMaxTokens() { return MAXTOKENS; }
    inline int getTokenCount() { return mTokenCount; }
    inline String getToken(int _index) { return mTokens[_index]; }

    // 공백문자로 구분된 문자열을 mTokens에 저장한다. 그리고 mTokenCount를 설정한다.
    inline int parse(String _strLine)
    {
        int StringCount = 0;
        // parse command
        while (_strLine.length() > 0)
        {
            int index = _strLine.indexOf(' ');
            if (index == -1) // No space found
            {
                mTokens[StringCount++] = _strLine;
                break;
            }
            else
            {
                mTokens[StringCount++] = _strLine.substring(0, index);
                _strLine = _strLine.substring(index + 1);
            }
        }

        mTokenCount = StringCount;
        return mTokenCount;
    }
};

#endif // TONKEY_HPP