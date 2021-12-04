#include <iostream>
#include "elf32_parser.h"
#include "elf32_constants.h"

void print_symbtable(Elf_File &elf) {
    printf("%s %-15s %7s %-8s %-8s %-8s %6s %s\n", "Symbol", "Value", "Size", "Type", "Bind", "Vis", "Index", "Name");
    auto symbs = elf.get_symtable();
    for (size_t i = 0; i < symbs.size(); i++) {
        auto symbol = symbs[i];
        auto info = symbol.st_info, o = symbol.st_other;
        printf("[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n",
                (int) i,
                symbol.st_value,
                symbol.st_size, 
                symbol_type[ELF32_ST_TYPE(info)],
                symbol_bind[ELF32_ST_BIND(info)],
                symbol_visibility[ELF32_ST_VISIBILITY(o)],
                symbol_index.find(symbol.st_shndx) == symbol_index.end() ? std::to_string(symbol.st_shndx).c_str() : symbol_index[symbol.st_shndx],
                elf.get_symbol_name(symbol.st_name)
                );
    }
}

signed main(int argc, const char *argv[]) {
    Elf_File elf(argv[1]);
    print_symbtable(elf);
    return 0;
}