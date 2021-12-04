#include "cmd_parser.h"

bool cmd_parser::get_bit(uint32_t val, size_t pos) {
    return val & (1 << pos);
}

void cmd_parser::set_bit(uint32_t &val, size_t pos, bool bit) {
    if (get_bit(val, pos) != bit)
        val ^= (1 << pos);
}


int32_t cmd_parser::make_signed(uint32_t val, size_t len) {
    uint32_t sign_bit_ptr = 1 << (len - 1);
    int32_t res = val & (sign_bit_ptr - 1);
    if (val & sign_bit_ptr)
        res *= -1;
    return res;
}

void cmd_parser::move_bits(uint32_t src, size_t src_start, size_t src_end, uint32_t &dst, size_t dst_start) {
    for (size_t i = 0; i < src_end - src_start; i++)
        set_bit(dst, dst_start + i, get_bit(src, src_start + i));
}

uint32_t cmd_parser::read_num(uint32_t src, size_t src_start, size_t src_end) {
    uint32_t res = 0;
    move_bits(src, src_start, src_end, res, 0);
    return res;
}

std::string cmd_parser::parse_cmd(uint32_t cmd) {
    if ((cmd & 3) == 3)
        return rv32im::parse_cmd(cmd);
    return rvc::parse_cmd(cmd);
}



std::string cmd_parser::rv32im::parse_cmd(uint32_t cmd) {
    return "unknown_command (rv32im) " + std::to_string(cmd);
}



std::string cmd_parser::rvc::parse_cmd(uint32_t cmd) {
    return "unknown_command (rvc) " + std::to_string(cmd);
}
