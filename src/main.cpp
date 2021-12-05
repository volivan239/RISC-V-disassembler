#include <iostream>
#include <cstring>
#include "elf32_parser.h"
#include "elf32_constants.h"
#include "cmd_parser.h"

void print_symbtable(Elf_File &elf) {
    printf("%s %-15s %7s %-8s %-8s %-8s %6s %s\n",
            "Symbol", "Value", "Size", "Type", "Bind", "Vis", "Index", "Name");
    std::vector <Elf32_Sym> symbs = elf.get_symtable();
    for (size_t i = 0; i < symbs.size(); i++) {
        auto symbol = symbs[i];
        auto info = symbol.st_info, o = symbol.st_other;
        printf("[%4i] 0x%-15x %5i %-8s %-8s %-8s %6s %s\n",
                (int) i,
                symbol.st_value,
                symbol.st_size, 
                symbol_type[ELF32_ST_TYPE(info)],
                symbol_bind[ELF32_ST_BIND(info)],
                symbol_visibility[ELF32_ST_VISIBILITY(o)],
                symbol_index.find(symbol.st_shndx) == symbol_index.end() ?
                    std::to_string(symbol.st_shndx).c_str() : symbol_index[symbol.st_shndx],
                elf.get_symbol_name(symbol.st_name)
                );
    }
}

void print_commands(Elf_File &elf) {
    std::vector <uint32_t> cmds = elf.get_text();
    uint32_t addr = elf.get_start_addr();
    std::vector <Elf32_Sym> symbs = elf.get_symtable();

    std::map <size_t, std::string> labels;
    std::uint32_t unknown_label_counter = 0;
    for (size_t i = 0; i < symbs.size(); i++) {
        const char *label = elf.get_symbol_name(symbs[i].st_name);
        if (strlen(label)) {
            labels[symbs[i].st_value] = std::string(label); 
        } else {
            char buf[10];
            sprintf(buf, "LOC_%05x", unknown_label_counter++);
            labels[symbs[i].st_value] = std::string(buf);
        }
    }

    for (uint32_t cmd : cmds) {
        uint32_t sz = (cmd & 3) == 3 ? 4 : 2;
        std::string parsed = cmd_parser::parse_cmd(cmd).c_str();
        if (labels.find(addr) == labels.end()) {
            printf("%08x %10s  %s\n", addr, "", parsed.c_str());
        } else {
            printf("%08x %10s: %s\n", addr, labels[addr].c_str(), parsed.c_str());
        }
        addr += sz;
    }
}

signed main(int argc, const char *argv[]) {
    Elf_File elf(argv[1]);
    print_symbtable(elf);
    printf("\n");
    print_commands(elf);
    return 0;
}
