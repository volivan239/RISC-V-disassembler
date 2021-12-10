#include <cstring>
#include <cassert>
#include <stdexcept>
#include "elf32_parser.h"
#define EM_RISCV 0xF3
#define ELFCLASS32 1
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

Elf_File::Elf_File(FILE *src): src_(src) {
    if (fread(&file_hdr_, sizeof(Elf32_Ehdr), 1, src_) != 1)
        bad_read();

    if (file_hdr_.e_ident[0] != ELFMAG0 || file_hdr_.e_ident[1] != ELFMAG1 || 
            file_hdr_.e_ident[2] != ELFMAG2 || file_hdr_.e_ident[3] != ELFMAG3)
        throw std::runtime_error("Error: input file is not an elf-file");

    if (file_hdr_.e_ident[4] != ELFCLASS32)
        throw std::runtime_error("Error: input file is not 32-bit format");

    if (file_hdr_.e_machine != EM_RISCV)
        throw std::runtime_error("Error: input file is not built for RISC-V architecture");


    Elf32_Shdr shstrtab_hdr;
    fseek(src_, file_hdr_.e_shoff + file_hdr_.e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
    if (fread(&shstrtab_hdr, sizeof(Elf32_Shdr), 1, src_) != 1)
        bad_read();

    char *section_names = new char[shstrtab_hdr.sh_size];
    fseek(src_, shstrtab_hdr.sh_offset, SEEK_SET);
    if (fread(section_names, shstrtab_hdr.sh_size, 1, src_) != 1)
        bad_read();

    Elf32_Shdr symtab_hdr, strtab_hdr, text_hdr;
    for (size_t i = 0; i < file_hdr_.e_shnum; i++) {
        Elf32_Shdr cur_sect_hdr;

        fseek(src_, file_hdr_.e_shoff + i * sizeof(Elf32_Shdr), SEEK_SET);
        if (fread(&cur_sect_hdr, sizeof(Elf32_Shdr), 1, src_) != 1)
            bad_read();
        
        if (!strcmp(section_names + cur_sect_hdr.sh_name, ".symtab"))
            symtab_hdr = cur_sect_hdr;

        if (!strcmp(section_names + cur_sect_hdr.sh_name, ".strtab"))
            strtab_hdr = cur_sect_hdr;

        if (!strcmp(section_names + cur_sect_hdr.sh_name, ".text")) {
            text_section_index_ = i;
            text_hdr_ = text_hdr = cur_sect_hdr;
        }
    }

    init_symtable(symtab_hdr);
    init_symbol_names(strtab_hdr);
    init_text(text_hdr);
    delete[] section_names;
}

void Elf_File::bad_read() {
    throw std::runtime_error("Error: input file is not correct");
}

void Elf_File::init_symtable(Elf32_Shdr symtab_hdr) {
    assert(sizeof(Elf32_Sym) == symtab_hdr.sh_entsize);
    size_t sym_cnt = symtab_hdr.sh_size / sizeof(Elf32_Sym);
    fseek(src_, symtab_hdr.sh_offset, SEEK_SET);

    symtab_.resize(sym_cnt);
    for (std::size_t i = 0; i < sym_cnt; i++) {
        if (fread(&symtab_[i], sizeof(Elf32_Sym), 1, src_) != 1)
            bad_read();
    }
}

void Elf_File::init_symbol_names(Elf32_Shdr strtab_hdr) {
    symbol_names_ = new char[strtab_hdr.sh_size];
    fseek(src_, strtab_hdr.sh_offset, SEEK_SET);
    if (fread(symbol_names_, strtab_hdr.sh_size, 1, src_) != 1)
        bad_read();
}

void Elf_File::init_text(Elf32_Shdr text_hdr) {
    for (uint32_t ptr = text_hdr.sh_offset; ptr != text_hdr.sh_offset + text_hdr.sh_size;) {
        fseek(src_, ptr, SEEK_SET);
        uint16_t cmd;
        if (fread(&cmd, 2, 1, src_) != 1)
            bad_read();

        if ((cmd & 3) == 3) {
            uint32_t cmd;
            fseek(src_, ptr, SEEK_SET);
            if (fread(&cmd, 4, 1, src_) != 1)
                bad_read();
            text_.push_back(cmd);
            ptr += 4;
        } else {
            text_.push_back(cmd);
            ptr += 2;
        }
    }
}


Elf_File::~Elf_File() {
    fclose(src_);
    delete[] symbol_names_;
}

std::vector <Elf32_Sym> Elf_File::get_symtable() {
    return symtab_;
}

std::vector <uint32_t> Elf_File::get_text() {
    return text_;
}

const char* Elf_File::get_symbol_name(uint32_t st_name) {
    return symbol_names_ + st_name;
}

uint32_t Elf_File::get_start_addr() {
    return text_hdr_.sh_addr;
}

size_t Elf_File::get_text_section_index() {
    return text_section_index_;
}
