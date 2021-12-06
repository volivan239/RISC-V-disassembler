#include <cassert>
#include <map>
#include <vector>
#include "cmd_parser.h"
#include "registers.h"

bool cmd_parser::get_bit(uint32_t val, size_t pos) {
    return val & (1 << pos);
}

void cmd_parser::set_bit(uint32_t &val, size_t pos, bool bit) {
    if (get_bit(val, pos) != bit)
        val ^= (1 << pos);
}


int32_t cmd_parser::sign_extend(int32_t val, size_t sign_bit) {
    bool f = get_bit(val, sign_bit);
    if (f) {
        for (size_t bit = sign_bit + 1; bit < 32; bit++) {
            val ^= (1u << bit); 
        }
    }
    return val;
}

void cmd_parser::move_bits(uint32_t src, size_t src_start, size_t src_end, uint32_t &dst, size_t dst_start) {
    for (size_t i = 0; i <= src_end - src_start; i++)
        set_bit(dst, dst_start + i, get_bit(src, src_start + i));
}

uint32_t cmd_parser::read_unsigned(uint32_t src, size_t src_start, size_t src_end) {
    uint32_t res = 0;
    move_bits(src, src_start, src_end, res, 0);
    return res;
}

int32_t cmd_parser::read_signed(uint32_t src, size_t src_start, size_t src_end) {
    return sign_extend(read_unsigned(src, src_start, src_end), src_end - src_start);
}

std::string cmd_parser::parse_cmd(uint32_t cmd) {
    if ((cmd & 3) == 3)
        return rv32im::parse_cmd(cmd);
    return rvc::parse_cmd(cmd);
}



uint32_t cmd_parser::rv32im::get_opcode(uint32_t cmd) {
    return read_unsigned(cmd, 0, 6);
}

uint32_t cmd_parser::rv32im::get_rd(uint32_t cmd) {
    return read_unsigned(cmd, 7, 11);
}

uint32_t cmd_parser::rv32im::get_rs1(uint32_t cmd) {
    return read_unsigned(cmd, 15, 19);
}

uint32_t cmd_parser::rv32im::get_rs2(uint32_t cmd) {
    return read_unsigned(cmd, 20, 24);
}

uint32_t cmd_parser::rv32im::get_funct2(uint32_t cmd) {
    return read_unsigned(cmd, 25, 26);
}

uint32_t cmd_parser::rv32im::get_funct3(uint32_t cmd) {
    return read_unsigned(cmd, 12, 14);
}

uint32_t cmd_parser::rv32im::get_funct5(uint32_t cmd) {
    return read_unsigned(cmd, 27, 31);
}


std::string cmd_parser::rv32im::parse_type_U(uint32_t cmd) {
    uint32_t opcode = get_opcode(cmd);
    uint32_t rd = get_rd(cmd);
    uint32_t uimm = 0;
    move_bits(cmd, 12, 31, uimm, 12);
    int32_t simm = uimm;

    if (opcode == 0b0110111)
        return "lui " + get_register_name(rd) + ", " + std::to_string(simm);

    if (opcode == 0b0010111)
        return "auipc " + get_register_name(rd) + ", " + std::to_string(simm);
    
    assert(0);
}

std::string cmd_parser::rv32im::parse_type_UJ(uint32_t cmd) {
    uint32_t rd = get_rd(cmd);
    uint32_t uimm = 0;
    move_bits(cmd, 12, 19, uimm, 12);
    move_bits(cmd, 20, 20, uimm, 11);
    move_bits(cmd, 21, 30, uimm, 1);
    move_bits(cmd, 31, 31, uimm, 20);
    int32_t offset = sign_extend(uimm, 20);
    return "jal " + get_register_name(rd) + ", " + std::to_string(offset);
}

std::string cmd_parser::rv32im::parse_type_I(uint32_t cmd) {
    uint32_t opcode = get_opcode(cmd);
    uint32_t rd = get_rd(cmd);
    uint32_t rs1 = get_rs1(cmd);
    uint32_t funct3 = get_funct3(cmd);
    int32_t imm = read_signed(cmd, 20, 31);

    if (opcode == 0b1100111 && funct3 == 0)
        return "jalr " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + std::to_string(imm);
    
    if (opcode == 0b0000011) {
        std::vector <std::string> cmds = {"lb", "lh", "lw", "", "lbu", "lhu", "", ""};
        std::string cmd_name = cmds[funct3];
        if (cmd_name != "")
            return cmd_name + " " + get_register_name(rd) + ", " + std::to_string(imm) + "(" + get_register_name(rs1) + ")";
    }

    if (opcode == 0b0010011) {
        std::vector <std::string> cmds = {"addi", "", "slti", "sltiu", "xori", "", "ori", "andi"};
        std::string cmd_name = cmds[funct3];
        if (cmd_name != "")
            return cmd_name + " " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + std::to_string(imm);

        uint32_t funct5 = get_funct5(cmd);
        uint32_t funct2 = get_funct2(cmd);
        uint32_t shamt = get_rs2(cmd);
        if (funct5 == 0 && funct2 == 0 && funct3 == 0b001) {
            return "slli " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + std::to_string(shamt);
        }
        if (funct5 == 0 && funct2 == 0 && funct3 == 0b101) {
            return "srli " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + std::to_string(shamt);
        }
        if (funct5 == 0b01000 && funct2 == 0 && funct3 == 0b101) {
            return "srai " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + std::to_string(shamt);
        }
    }

    if (opcode == 0b1110011) {
        if (rd == 0 && funct3 == 0 && rs1 == 0 && imm == 0)
            return "ecall";
        if (rd == 0 && funct3 == 0 && rs1 == 0 && imm == 1)
            return "ebreak";
    }

    return "unknown_command";
}

std::string cmd_parser::rv32im::parse_type_SB(uint32_t cmd) {
    uint32_t funct3 = get_funct3(cmd);
    uint32_t rs1 = get_rs1(cmd);
    uint32_t rs2 = get_rs2(cmd);
    uint32_t imm = 0;
    move_bits(cmd, 7, 7, imm, 11);
    move_bits(cmd, 8, 11, imm, 1);
    move_bits(cmd, 25, 30, imm, 5);
    move_bits(cmd, 31, 31, imm, 12);
    int32_t offset = sign_extend(imm, 12); 

    std::vector <std::string> cmds = {"beq", "bne", "", "", "blt", "bge", "bltu", "bgeu"};
    std::string cmd_name = cmds[funct3];
    if (cmd_name == "")
        return "unknown_command";

    return cmd_name + " " + get_register_name(rs1) + ", " + get_register_name(rs2) + ", " + std::to_string(offset);
}

std::string cmd_parser::rv32im::parse_type_S(uint32_t cmd) {
    uint32_t funct3 = get_funct3(cmd);
    uint32_t rs1 = get_rs1(cmd);
    uint32_t rs2 = get_rs2(cmd);
    uint32_t imm = 0;
    move_bits(cmd, 7, 11, imm, 0);
    move_bits(cmd, 25, 31, imm, 5);
    int32_t offset = sign_extend(imm, 11); 

    if (funct3 > 2)
        return "unknown_command";

    std::vector <std::string> cmds = {"sb", "sh", "sw"};
    std::string cmd_name = cmds[funct3];
    return cmd_name + " " + get_register_name(rs2) + ", " + std::to_string(offset) + "(" + get_register_name(rs1) + ")";
}

std::string cmd_parser::rv32im::parse_type_R(uint32_t cmd) {
    uint32_t rd = get_rd(cmd);
    uint32_t funct3 = get_funct3(cmd);
    uint32_t rs1 = get_rs1(cmd);
    uint32_t rs2 = get_rs2(cmd);
    uint32_t funct2 = get_funct2(cmd);
    uint32_t funct5 = get_funct5(cmd);

    if (funct2 == 0) {
        std::map <std::pair <uint32_t, uint32_t>, std::string> cmds = {
            {{0b00000, 0b000}, "add"},
            {{0b01000, 0b000}, "sub"},
            {{0b00000, 0b001}, "sll"},
            {{0b00000, 0b010}, "slt"},
            {{0b00000, 0b011}, "sltu"},
            {{0b00000, 0b100}, "xor"},
            {{0b00000, 0b101}, "srl"},
            {{0b01000, 0b101}, "sra"},
            {{0b00000, 0b110}, "or"},
            {{0b00000, 0b111}, "and"},
        };
        std::string cmd_name = cmds[{funct5, funct3}];
        if (cmd_name != "")
            return cmd_name + " " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + get_register_name(rs2);
    }

    if (funct2 == 1 && funct5 == 0) {
        std::vector <std::string> cmds = {"mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu"};
        std::string cmd_name = cmds[funct3];
        return cmd_name + " " + get_register_name(rd) + ", " + get_register_name(rs1) + ", " + get_register_name(rs2);
    }
    return "unknown_command";
}


std::string cmd_parser::rv32im::parse_cmd(uint32_t cmd) {
    uint32_t opcode = get_opcode(cmd);
    if (opcode == 0b0110111 || opcode == 0b0010111)
        return parse_type_U(cmd);
    if (opcode == 0b1101111)
        return parse_type_UJ(cmd);
    if (opcode == 0b1100111 || opcode == 0b0000011 || opcode == 0b0010011 || opcode == 0b1110011)
        return parse_type_I(cmd);
    if (opcode == 0b1100011)
        return parse_type_SB(cmd);
    if (opcode == 0b0100011)
        return parse_type_S(cmd);
    if (opcode == 0b0110011)
        return parse_type_R(cmd);
    return "unknown_command (rv32im) ";
}




uint32_t cmd_parser::rvc::get_opcode(uint32_t cmd) {
    return read_unsigned(cmd, 0, 1);
}

uint32_t cmd_parser::rvc::get_funct3(uint32_t cmd) {
    return read_unsigned(cmd, 13, 15);
}

uint32_t cmd_parser::rvc::get_funct4(uint32_t cmd) {
    return read_unsigned(cmd, 12, 15);
}

uint32_t cmd_parser::rvc::get_funct6(uint32_t cmd) {
    return read_unsigned(cmd, 10, 15);
}

uint32_t cmd_parser::rvc::get_rd_rs1(uint32_t cmd) {
    return read_unsigned(cmd, 7, 11);
}

uint32_t cmd_parser::rvc::get_rs2(uint32_t cmd) {
    return read_unsigned(cmd, 2, 6);
}

uint32_t cmd_parser::rvc::get_rs1_p_rd_p(uint32_t cmd) {
    return read_unsigned(cmd, 7, 9);
}

uint32_t cmd_parser::rvc::get_rs2_p_rd_p(uint32_t cmd) {
    return read_unsigned(cmd, 2, 4);
}

std::string cmd_parser::rvc::parse_type_00(uint32_t cmd) {
    uint32_t rd_p = get_rs2_p_rd_p(cmd);
    uint32_t funct3 = get_funct3(cmd);
    if (funct3 == 0) {
        uint32_t imm = 0;
        move_bits(cmd, 5, 5, imm, 3);
        move_bits(cmd, 6, 6, imm, 2);
        move_bits(cmd, 7, 10, imm, 6);
        move_bits(cmd, 11, 12, imm, 4);
        return "c.addi4spn " + get_rvc_register_name(rd_p) + ", sp, " + std::to_string(imm);
    }
    uint32_t imm = 0;
    move_bits(cmd, 5, 5, imm, 6);
    move_bits(cmd, 6, 6, imm, 2);
    move_bits(cmd, 10, 12, imm, 3);
    uint32_t rs1_p = get_rs1_p_rd_p(cmd);
    if (funct3 == 0b010)
        return "c.lw " + get_rvc_register_name(rd_p) + ", " + std::to_string(imm) + "(" + get_rvc_register_name(rs1_p) + ")";
    if (funct3 == 0b110)
        return "c.sw " + get_rvc_register_name(rd_p) + ", " + std::to_string(imm) + "(" + get_rvc_register_name(rs1_p) + ")";
    return "unknown_command";
}

std::string cmd_parser::rvc::parse_type_01(uint32_t cmd) {
    uint32_t funct3 = get_funct3(cmd);
    if (funct3 == 0b000 || funct3 == 0b010) {
        uint32_t imm = 0;
        move_bits(cmd, 2, 6, imm, 0);
        move_bits(cmd, 12, 12, imm, 5);
        int32_t simm = sign_extend(imm, 5);
        uint32_t rd_rs1 = get_rd_rs1(cmd);
        if (funct3 == 0 && rd_rs1 == 0 && imm == 0)
            return "c.nop";
        if (funct3 == 0 && rd_rs1 != 0)
            return "c.addi " + get_register_name(rd_rs1) + ", " + get_register_name(rd_rs1) + ", " + std::to_string(simm);
        if (funct3 == 0b010 && rd_rs1 != 0)
            return "c.li " + get_register_name(rd_rs1) + ", " + std::to_string(simm);
    }
    if (funct3 == 0b001 || funct3 == 0b101) {
        uint32_t imm = 0;
        move_bits(cmd, 2, 2, imm, 5);
        move_bits(cmd, 3, 5, imm, 1);
        move_bits(cmd, 6, 6, imm, 7);
        move_bits(cmd, 7, 7, imm, 6);
        move_bits(cmd, 8, 8, imm, 10);
        move_bits(cmd, 9, 10, imm, 8);
        move_bits(cmd, 11, 11, imm, 4);
        move_bits(cmd, 12, 12, imm, 11);
        int32_t offset = sign_extend(imm, 11);
        if (funct3 == 0b001)
            return "c.jal ra, " + std::to_string(offset);
        if (funct3 == 0b101)
            return "c.j " + std::to_string(offset);
    }
    if (funct3 == 0b011) {
        uint32_t rd_rs1 = get_rd_rs1(cmd);
        uint32_t imm = 0;
        if (rd_rs1 == 2) {
            move_bits(cmd, 2, 2, imm, 5);
            move_bits(cmd, 3, 4, imm, 7);
            move_bits(cmd, 5, 5, imm, 6);
            move_bits(cmd, 6, 6, imm, 4);
            move_bits(cmd, 12, 12, imm, 9);
            int32_t simm = sign_extend(imm, 9);
            return "c.addi16sp sp, sp, " + std::to_string(simm);
        }
        move_bits(cmd, 2, 6, imm, 12); 
        move_bits(cmd, 12, 12, imm, 17);
        int32_t simm = sign_extend(imm, 17);
        if (rd_rs1 != 0)
            return "c.lui " + get_register_name(rd_rs1) + ", " + std::to_string(simm);
    }
    if (funct3 == 0b100) {
        uint32_t funct2 = read_unsigned(cmd, 10, 11);
        uint32_t rs1_p_rd_p = get_rs1_p_rd_p(cmd);
        if (funct2 == 0b11) {
            uint32_t funct3 = read_unsigned(cmd, 10, 12);
            uint32_t funct2 = read_unsigned(cmd, 5, 6);
            uint32_t rs2p = get_rs2_p_rd_p(cmd);
            if (funct3 == 0b011) {
                std::vector <std::string> cmd_names = {"c.sub", "c.xor", "c.or", "c.and"};
                return cmd_names[funct2] + " " + get_rvc_register_name(rs1_p_rd_p) + ", " + get_rvc_register_name(rs1_p_rd_p) + ", " + get_register_name(rs2p);
            }
        }
        if ((funct2 == 0b00 || funct2 == 0b01) && read_unsigned(cmd, 12, 12) == 0) {
            std::string cmd_name = funct2 == 0b00 ? "c.srli" : "c.srai";
            uint32_t imm = read_unsigned(cmd, 2, 6);
            return cmd_name + " " + get_rvc_register_name(rs1_p_rd_p) + ", " + get_register_name(rs1_p_rd_p) + ", " + std::to_string(imm);
        }
        if (funct2 == 0b10) {
            uint32_t imm = read_unsigned(cmd, 2, 6);
            move_bits(cmd, 12, 12, imm, 5);
            int32_t simm = sign_extend(imm, 5);
            return "c.andi " + get_rvc_register_name(rs1_p_rd_p) + ", " + get_rvc_register_name(rs1_p_rd_p) + ", " + std::to_string(simm);
        }
    }
    if (funct3 == 0b110 || funct3 == 0b111) {
        uint32_t imm = 0;
        move_bits(cmd, 2, 2, imm, 5);
        move_bits(cmd, 3, 4, imm, 1);
        move_bits(cmd, 5, 6, imm, 6);
        move_bits(cmd, 10, 11, imm, 3);
        move_bits(cmd, 12, 12, imm, 8);
        uint32_t rs1_p = get_rs1_p_rd_p(cmd);
        int32_t offset = sign_extend(imm, 8);
        if (funct3 == 0b110)
            return "c.beqz " + get_rvc_register_name(rs1_p) + ", " + std::to_string(offset);
        if (funct3 == 0b111)
            return "c.bnez " + get_rvc_register_name(rs1_p) + ", " + std::to_string(offset);
    }
    return "unknown_command";
}

std::string cmd_parser::rvc::parse_type_10(uint32_t cmd) {
    uint32_t funct3 = get_funct3(cmd);
    if (funct3 == 0b000 && read_unsigned(cmd, 12, 12) == 0) {
        uint32_t rs1_rd = get_rd_rs1(cmd);
        uint32_t imm = read_unsigned(cmd, 2, 6);
        return "c.slli " + get_register_name(rs1_rd) + ", " + get_register_name(rs1_rd) + ", " + std::to_string(imm);
    }
    if (funct3 == 0b010) {
        uint32_t rd_rs1 = get_rd_rs1(cmd);
        uint32_t imm = 0;
        move_bits(cmd, 2, 3, imm, 6);
        move_bits(cmd, 4, 6, imm, 2);
        move_bits(cmd, 12, 12, imm, 5);
        return "c.lwsp " + get_register_name(rd_rs1) + ", " + std::to_string(imm) + "(sp)";
    }
    if (funct3 == 0b100) {
        uint32_t funct4 = get_funct4(cmd);
        uint32_t rd_rs1 = get_rd_rs1(cmd);
        uint32_t rs2 = get_rs2(cmd);
        if (funct4 == 0b1000 && rs2 == 0)
            return "c.jr " + get_register_name(rd_rs1);
        if (funct4 == 0b1000 && rd_rs1 != 0)
            return "c.mv " + get_register_name(rd_rs1) + ", " + get_register_name(rs2);
        if (funct4 == 0b1001 && rd_rs1 == 0 && rs2 == 0)
            return "c.ebreak";
        if (funct4 == 0b1001 && rs2 == 0)
            return "c.jalr ra, " + get_register_name(rd_rs1) + ", 0";
        if (funct4 == 0b1001 && rd_rs1 != 0)
            return "c.add " + get_register_name(rd_rs1) + ", " + get_register_name(rd_rs1) + ", " + get_register_name(rs2);
    }
    if (funct3 == 0b110) {
        uint32_t rs2 = get_rs2(cmd);
        uint32_t imm = 0;
        move_bits(cmd, 7, 8, imm, 6);
        move_bits(cmd, 9, 12, imm, 2);
        return "c.swsp " + get_register_name(rs2) + ", " + std::to_string(imm) + "(sp)";
    }
    return "unknown_command";
}

std::string cmd_parser::rvc::parse_cmd(uint32_t cmd) {
    uint32_t opcode = get_opcode(cmd);
    if (opcode == 0b00)
        return parse_type_00(cmd);
    if (opcode == 0b01)
        return parse_type_01(cmd);
    if (opcode == 0b10)
        return parse_type_10(cmd);
    return "unknown_command";
}
