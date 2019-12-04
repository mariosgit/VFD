#pragma once

// for log output
// - allows simple redirect to another Serial (or whatever)
// - allows quick disable without uncommenting each Seral.print..
// - stream style

#include <Arduino.h>

// does not work with typedef :( ???
#define MbLogOut Serial

class MbLog
{
public:
    MbLog() {}
    inline void begin() {}

    inline void print  (byte val, byte how = DEC) { MbLogOut.print(val,how); }
    inline void println(byte val, byte how = DEC) { MbLogOut.println(val,how); }
    inline void print  (uint16_t val, byte how = DEC) { MbLogOut.print(val,how); }
    inline void println(uint16_t val, byte how = DEC) { MbLogOut.println(val,how); }
    inline void print  (int16_t val, byte how = DEC) { MbLogOut.print(val,how); }
    inline void println(int16_t val, byte how = DEC) { MbLogOut.println(val,how); }
    inline void print  (uint32_t val, byte how = DEC) { MbLogOut.print(val,how); }
    inline void println(uint32_t val, byte how = DEC) { MbLogOut.println(val,how); }
    inline void print  (int32_t val, byte how = DEC) { MbLogOut.print(val,how); }
    inline void println(int32_t val, byte how = DEC) { MbLogOut.println(val,how); }
    inline void print  (const char *msg) { MbLogOut.print(msg); }
    inline void println(const char *msg) { MbLogOut.println(msg); }
    inline void print  (float val) { MbLogOut.print(val); }
    inline void println(float val) { MbLogOut.println(val); }
    inline MbLog& operator<<(const char *msg) { MbLogOut.print(msg); return *this; };
    inline MbLog& operator<<(int val) { MbLogOut.print(val); return *this; };
    inline MbLog& operator<<(int32_t val) { MbLogOut.print(val); return *this; };
    inline MbLog& operator<<(uint32_t val) { MbLogOut.print(val); return *this; };
    inline MbLog& operator<<(byte val) { MbLogOut.print(val); return *this; };
    inline MbLog& operator<<(char val) { MbLogOut.print(val); return *this; };
    inline MbLog& operator<<(float val) { MbLogOut.print(val); return *this; };
};

extern MbLog LOG;
