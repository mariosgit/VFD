#pragma once

#include <Arduino.h>

class VFDFont
{
  public:
    VFDFont(const uint8_t *fontmap, int shift = 1) : map(fontmap), shift(shift) { charslen = strlen(chars); }
    const uint8_t *map;
    const int shift;
    const int width = 8;
    const int height = 8;
    const char *chars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    int charslen = 0;
};

class VFDFont6 : public VFDFont
{
  public:
    VFDFont6() : VFDFont( _map, 2 ) {}
    const static uint8_t _map[];
};

class VFDFont7 : public VFDFont
{
  public:
    VFDFont7() : VFDFont( _map ) {}
    const static uint8_t _map[];
};
