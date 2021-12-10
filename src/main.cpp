#include <iostream>
#include <cstring>
#include "elf32_parser.h"
#include "elf32_constants.h"
#include "cmd_parser.h"

void print_symbtable(Elf_File &elf, FILE *fout) {
    fprintf(fout, ".symtab\n");
    fprintf(fout, "%s %-15s %7s %-8s %-8s %-8s %6s %s\n",
            "Symbol", "Value", "Size", "Type", "Bind", "Vis", "Index", "Name");
    std::vector <Elf32_Sym> symbs = elf.get_symtable();
    for (size_t i = 0; i < symbs.size(); i++) {
        auto symbol = symbs[i];
        auto info = symbol.st_info, o = symbol.st_other;
        fprintf(fout, "[%4i] 0x%-15x %5i %-8s %-8s %-8s %6s %s\n",
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

void print_commands(Elf_File &elf, FILE *fout) {
    fprintf(fout, ".text\n");
    std::vector <std::string> res = cmd_parser(elf).parse_cmds(); 
    for (std::string cmd : res)
        fprintf(fout, "%s\n", cmd.c_str());
}

signed main(int argc, const char *argv[]) {
    if (argc != 3) {
        std::cerr << "Error: program should take exactly 2 arguments: input file name and output file name\n";
        return 1;
    }

    FILE *fin = fopen(argv[1], "rb");
    if (fin == nullptr) {
        std::cerr << "Error: can't open input file for reading\n";
        return 2;
    }

    FILE *fout = fopen(argv[2], "w");
    if (fout == nullptr) {
        std::cerr << "Error: can't open output file for writing\n";
        return 2;
    }

    try {
        Elf_File elf(fin);
        print_commands(elf, fout);
        fprintf(fout, "\n");
        print_symbtable(elf, fout);
    } catch (std::exception &e) {
        std::cerr << std::string(e.what()) << "\n";
        fclose(fout);
        return 3;
    }

    fclose(fout);
    return 0;
}
