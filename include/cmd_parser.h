#pragma once
#include <stdint.h>
#include <string>
#include <tuple>

namespace cmd_parser {

bool get_bit(uint32_t val, size_t pos);
void set_bit(uint32_t &val, size_t pos, bool bit);

int32_t sign_extend(int32_t val, size_t sign_bit);
void move_bits(uint32_t src, size_t src_start, size_t src_end, uint32_t &dst, size_t dst_start);
uint32_t read_unsigned(uint32_t src, size_t src_start, size_t src_end);
int32_t read_signed(uint32_t src, size_t src_start, size_t src_end);

std::string parse_cmd(uint32_t cmd);

namespace rv32im {

    uint32_t get_opcode(uint32_t cmd);
    uint32_t get_rd(uint32_t cmd);
    uint32_t get_rs1(uint32_t cmd);
    uint32_t get_rs2(uint32_t cmd);
    uint32_t get_funct2(uint32_t cmd);
    uint32_t get_funct3(uint32_t cmd);
    uint32_t get_funct5(uint32_t cmd);

    std::string parse_type_U(uint32_t cmd);
    std::string parse_type_UJ(uint32_t cmd);
    std::string parse_type_I(uint32_t cmd);
    std::string parse_type_SB(uint32_t cmd);
    std::string parse_type_S(uint32_t cmd);
    std::string parse_type_R(uint32_t cmd);


    std::string parse_cmd(uint32_t cmd);
}

namespace rvc {

    uint32_t get_opcode(uint32_t cmd);
    uint32_t get_funct3(uint32_t cmd);
    uint32_t get_funct4(uint32_t cmd);
    uint32_t get_funct6(uint32_t cmd);
    uint32_t get_rd_rs1(uint32_t cmd);
    uint32_t get_rs2(uint32_t cmd);
    uint32_t get_rs1_p_rd_p(uint32_t cmd);
    uint32_t get_rs2_p_rd_p(uint32_t cmd);

    std::string parse_type_00(uint32_t cmd);
    std::string parse_type_01(uint32_t cmd);
    std::string parse_type_10(uint32_t cmd);

    std::string parse_cmd(uint32_t cmd);

}


}
