#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <set>
#include "elf32_parser.h"

class cmd_parser {
public:
    cmd_parser(Elf_File &elf);
    std::vector <std::string> parse_cmds();

private:
    Elf_File &elf_;
    std::map <uint32_t, std::string> symtab_;
    std::set <uint32_t> loc_addresses_;
    uint32_t cur_addr_;

private:
    static bool get_bit(uint32_t val, size_t pos);
    static void set_bit(uint32_t &val, size_t pos, bool bit);

    static int32_t sign_extend(int32_t val, size_t sign_bit);
    static void move_bits(uint32_t src, size_t src_start, size_t src_end, uint32_t &dst, size_t dst_start);
    static uint32_t read_unsigned(uint32_t src, size_t src_start, size_t src_end);
    static int32_t read_signed(uint32_t src, size_t src_start, size_t src_end);

    std::string get_label(int32_t offset);
    std::string parse_cmd(uint32_t cmd);

    class rv32im {
    public:

        rv32im(cmd_parser &outer): outer_(outer) {}

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
    private:
        cmd_parser &outer_;

    } rv32im_ = rv32im(*this);

    class rvc {
    public:
        rvc(cmd_parser &outer): outer_(outer) {}

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

    private:
        cmd_parser &outer_;
    } rvc_ = rvc(*this);


};
