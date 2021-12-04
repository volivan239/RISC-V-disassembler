#pragma once
#include <stdint.h>
#include <map>
#define ELF32_ST_BIND(info)          ((info) >> 4)
#define ELF32_ST_TYPE(info)          ((info) & 0xf)
#define ELF32_ST_VISIBILITY(o)       ((o)&0x3)

std::map<char, const char*> symbol_bind = {
    { 0, "LOCAL"},
    { 1, "GLOBAL"},
    { 2, "WEAK"},
    {10, "LOOS"},
    {12, "HIOS"},
    {13, "LOPROC"},
    {15, "HIPROC"}
};

std::map<char, const char*> symbol_type = {
    { 0, "NOTYPE"},
    { 1, "OBJECT"},
    { 2, "FUNC"},
    { 3, "SECTION"},
    { 4, "FILE"},
    { 5, "COMMON"},
    { 6, "TLS"},
    {10, "LOOS"},
    {12, "HIOS"},
    {13, "LOPROC"},
    {15, "HIPROC"}
};

std::map<char, const char*> symbol_visibility = {
    {0, "DEFAULT"},
    {1, "INTERNAL"},
    {2, "HIDDEN"},
    {3, "PROTECTED"},
    {4, "EXPORTED"},
    {5, "SINGLETON"},
    {6, "ELIMINATE"}
};

std::map<uint16_t, const char*> symbol_index = {
    {0x0000, "UNDEF"},
    {0xff00, "LOPROC"},
    {0xff01, "AFTER"},
    {0xff02, "AMD64_LCOMMON"},
    {0xff1f, "HIPROC"},
    {0xff20, "LOOS"},
    {0xff3f, "HIOS"},
    {0xfff1, "ABS"},
    {0xfff2, "COMMON"},
    {0xffff, "XINDEX"},
};
