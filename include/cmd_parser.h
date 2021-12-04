#pragma once
#include <stdint.h>
#include <string>
#include <tuple>

namespace cmd_parser {

bool get_bit(uint32_t val, size_t pos);
void set_bit(uint32_t &val, size_t pos, bool bit);

int32_t make_signed(uint32_t val, size_t len);
void move_bits(uint32_t src, size_t src_start, size_t src_end, uint32_t &dst, size_t dst_start);
uint32_t read_num(uint32_t src, size_t src_start, size_t src_end);

std::string parse_cmd(uint32_t cmd);

namespace rv32im {

    std::string parse_cmd(uint32_t cmd);

}

namespace rvc {

    std::string parse_cmd(uint32_t cmd);

}


}
