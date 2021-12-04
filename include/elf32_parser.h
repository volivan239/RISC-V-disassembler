#include <stdio.h>
#include <vector>
#include "elf32.h"

class Elf_File{
public:
    Elf_File(const char *file_name);
    ~Elf_File();
    std::vector <Elf32_Sym> get_symtable();
    std::vector <uint32_t> read_text();
    const char* get_symbol_name(uint32_t st_name);
    uint32_t get_start_addr();

private:
    FILE *src_;
    Elf32_Ehdr file_hdr_;
    Elf32_Shdr symtab_hdr_, text_hdr_;
    char *section_names_;
    char *symbol_names_;
};
