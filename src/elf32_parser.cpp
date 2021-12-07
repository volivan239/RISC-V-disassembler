#include <cstring>
#include <cassert>
#include "elf32_parser.h"

Elf_File::Elf_File(const char *file_name) {
    src_ = fopen(file_name, "r");
    fread(&file_hdr_, sizeof(Elf32_Ehdr), 1, src_);

    Elf32_Shdr shstrtab_hdr;
    fseek(src_, file_hdr_.e_shoff + file_hdr_.e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
    fread(&shstrtab_hdr, sizeof(Elf32_Shdr), 1, src_);

    char *section_names = new char[shstrtab_hdr.sh_size];
    fseek(src_, shstrtab_hdr.sh_offset, SEEK_SET);
    fread(section_names, shstrtab_hdr.sh_size, 1, src_);

    Elf32_Shdr symtab_hdr, strtab_hdr, text_hdr;

    for (size_t i = 0; i < file_hdr_.e_shnum; i++) {
        Elf32_Shdr cur_sect_hdr;

        fseek(src_, file_hdr_.e_shoff + i * sizeof(Elf32_Shdr), SEEK_SET);
        fread(&cur_sect_hdr, 1, sizeof(Elf32_Shdr), src_);
        
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

void Elf_File::init_symtable(Elf32_Shdr symtab_hdr) {
    assert(sizeof(Elf32_Sym) == symtab_hdr.sh_entsize);
    size_t sym_cnt = symtab_hdr.sh_size / sizeof(Elf32_Sym);
    fseek(src_, symtab_hdr.sh_offset, SEEK_SET);

    symtab_.resize(sym_cnt);
    for (std::size_t i = 0; i < sym_cnt; i++) {
        fread(&symtab_[i], sizeof(Elf32_Sym), 1, src_);
    }
}

void Elf_File::init_symbol_names(Elf32_Shdr strtab_hdr) {
    symbol_names_ = new char[strtab_hdr.sh_size];
    fseek(src_, strtab_hdr.sh_offset, SEEK_SET);
    fread(symbol_names_, strtab_hdr.sh_size, 1, src_);
}

void Elf_File::init_text(Elf32_Shdr text_hdr) {
    for (uint32_t ptr = text_hdr.sh_offset; ptr != text_hdr.sh_offset + text_hdr.sh_size;) {
        fseek(src_, ptr, SEEK_SET);
        uint16_t cmd;
        fread(&cmd, 2, 1, src_);
        if ((cmd & 3) == 3) {
            uint32_t cmd;
            fseek(src_, ptr, SEEK_SET);
            fread(&cmd, 4, 1, src_);
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
