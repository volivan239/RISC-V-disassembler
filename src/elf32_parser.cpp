#include <cstring>
#include <cassert>
#include "elf32_parser.h"
#include <iostream> // TODO: delete

Elf_File::Elf_File(const char *file_name) {
    src_ = fopen(file_name, "r");
    fread(&file_hdr_, 1, sizeof(Elf32_Ehdr), src_);

    Elf32_Shdr shstrtab_hdr;
    fseek(src_, file_hdr_.e_shoff + file_hdr_.e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
    fread(&shstrtab_hdr, 1, sizeof(Elf32_Shdr), src_);

    section_names_ = new char[shstrtab_hdr.sh_size];
    fseek(src_, shstrtab_hdr.sh_offset, SEEK_SET);
    fread(section_names_, 1, shstrtab_hdr.sh_size, src_);

    Elf32_Shdr strtab_hdr;

    for (size_t i = 0; i < file_hdr_.e_shnum; i++) {
        Elf32_Shdr cur_sect_hdr;

        fseek(src_, file_hdr_.e_shoff + i * sizeof(Elf32_Shdr), SEEK_SET);
        fread(&cur_sect_hdr, 1, sizeof(Elf32_Shdr), src_);
        
        if (!strcmp(section_names_ + cur_sect_hdr.sh_name, ".symtab"))
            symtab_hdr_ = cur_sect_hdr;

        if (!strcmp(section_names_ + cur_sect_hdr.sh_name, ".strtab"))
            strtab_hdr = cur_sect_hdr;

        if (!strcmp(section_names_ + cur_sect_hdr.sh_name, ".text"))
            text_hdr_ = cur_sect_hdr;
    }

    section_names_ = new char[strtab_hdr.sh_size];
    fseek(src_, strtab_hdr.sh_offset, SEEK_SET);
    fread(section_names_, 1, strtab_hdr.sh_size, src_);
}

Elf_File::~Elf_File() {
    fclose(src_);
    delete[] section_names_;
    delete[] symbol_names_;
}

std::vector <Elf32_Sym> Elf_File::get_symtable() {
    assert(sizeof(Elf32_Sym) == symtab_hdr_.sh_entsize);
    size_t sym_cnt = symtab_hdr_.sh_size / sizeof(Elf32_Sym);

    std::vector <Elf32_Sym> res(sym_cnt);
    for (std::size_t i = 0; i < sym_cnt; i++) {
        fseek(src_, symtab_hdr_.sh_offset + i * sizeof(Elf32_Sym), SEEK_SET);
        fread(&res[i], 1, sizeof(Elf32_Sym), src_);
    }
    return res;
}

std::vector <uint32_t> Elf_File::read_text() {
    return {};
}

const char* Elf_File::get_symbol_name(uint32_t st_name) {
    return section_names_ + st_name;
}
