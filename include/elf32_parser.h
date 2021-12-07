#pragma once
#include <stdio.h>
#include <vector>
#include "elf32.h"

class Elf_File{
public:
    Elf_File(const char *file_name);
    ~Elf_File();
    std::vector <Elf32_Sym> get_symtable();
    std::vector <uint32_t> get_text();
    const char* get_symbol_name(uint32_t st_name);
    uint32_t get_start_addr();
    size_t get_text_section_index();

private:
    FILE *src_;
    Elf32_Ehdr file_hdr_;
    Elf32_Shdr text_hdr_;
    size_t text_section_index_;
    char *symbol_names_;
    std::vector <Elf32_Sym> symtab_;
    std::vector <uint32_t> text_;

    void init_symtable(Elf32_Shdr symtab_hdr);
    void init_symbol_names(Elf32_Shdr strtab_hdr);
    void init_text(Elf32_Shdr text_hdr);
};
