#include "registers.h"

std::string get_register_name(uint32_t reg) {
    switch (reg) {
        case 0 : return "zero";
        case 1 : return "ra";
        case 2 : return "sp";
        case 3 : return "gp";
        case 4 : return "tp";
        case 5 : return "t0";
        case 6 : return "t1";
        case 7 : return "t2";
        case 8 : return "s0";
        case 9 : return "s1";
        default : {
            if (reg <= 17)
                return "a" + std::to_string(reg - 10);
            if (reg <= 27)
                return "s" + std::to_string(reg - 16);
            return "t" + std::to_string(reg - 25);
        }
    }
}

std::string get_rvc_register_name(uint32_t reg) {
    return get_register_name(reg + 8);
}
