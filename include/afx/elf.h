#pragma once

#include "afx/types.h"

#ifdef __cplusplus
namespace afx {
#endif
#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

    enum elfident_t {
        EI_MAG0       = 0,
        EI_MAG1       = 1,
        EI_MAG2       = 2,
        EI_MAG3       = 3,
        EI_CLASS      = 4,
        EI_DATA       = 5,
        EI_VERSION    = 6,
        EI_OSABI      = 7,
        EI_ABIVERSION = 8,
        EI_PAD        = 9,
        EI_NIDENT     = 16,
    };

    enum elfclass_t {
        ELFCLASSNONE = 0,
        ELFCLASS32   = 1,
        ELFCLASS64   = 2,
    };

    enum elfdata_t {
        ELFDATANONE = 0,
        ELFDATA2LSB = 1,
        ELFDATA2MSB = 2,
    };

    enum elfver_t {
        EV_NONE    = 0,
        EV_CURRENT = 1,
    };

    enum elftype_t {
        ET_NONE = 0,
        ET_REL  = 1,
        ET_EXEC = 2,
        ET_DYN  = 3,
        ET_CORE = 4,
    };

    enum elfstyp_t {
        SHN_UNDEF     = 0,
        SHN_LORESERVE = 0xFF00,
        SHN_LOPROC    = 0xFF00,
        SHN_BEFORE    = 0xFF00,
        SHN_AFTER     = 0xFF01,
        SHN_HIPROC    = 0xFF1F,
        SHN_LOOS      = 0xFF20,
        SHN_HIOS      = 0xFF3F,
        SHN_ABS       = 0xFFF1,
        SHN_COMMON    = 0xFFF2,
        SHN_XINDEX    = 0xFFFF,
        SHN_HIRESERVE = 0xFFFF,
    };

    struct elf {
        int         bits;
        u16         type;
        u16         machine;
        void*       entry;
        void*       image;
        bool        loaded;
        bool        flip;
        int         strindex;
        const char* strings;
    };
    typedef struct elf elf_t;

    struct elf32 {
        u8     ident[EI_NIDENT];
        u16    type;
        u16    machine;
        u32    version;
        addr32 entry;
        off32  phoff;
        off32  shoff;
        u32    flags;
        u16    ehsize;
        u16    phentsize;
        u16    phnum;
        u16    shentsize;
        u16    shnum;
        u16    shstrndx;
    };
    typedef struct elf32 elf32_t;

    struct elf64 {
        u8     ident[EI_NIDENT];
        u16    type;
        u16    machine;
        u32    version;
        addr64 entry;
        off64  phoff;
        off64  shoff;
        u32    flags;
        u16    ehsize;
        u16    phentsize;
        u16    phnum;
        u16    shentsize;
        u16    shnum;
        u16    shstrndx;
    };
    typedef struct elf64 elf64_t;

    struct elfcond {
        int   bits;
        u16   type;
        u16   machine;
        void* minaddr;
        void* maxaddr;
    };
    typedef struct elfcond elfcond_t;

    struct elf32phdr {
        u32    type;
        off32  offset;
        addr32 vaddr;
        addr32 paddr;
        u32    filesz;
        u32    memsz;
        u32    flags;
        u32    align;
    };
    typedef struct elf32phdr elf32phdr_t;

    struct elf64phdr {
        u32    type;
        u32    flags;
        off64  offset;
        addr64 vaddr;
        addr64 paddr;
        u64    filesz;
        u64    memsz;
        u64    align;
    };
    typedef struct elf64phdr elf64phdr_t;

    struct elf32shdr {
        u32    name;
        u32    type;
        u32    flags;
        addr32 addr;
        off32  offset;
        u32    size;
        u32    link;
        u32    info;
        u32    addralign;
        u32    entsize;
    };
    typedef struct elf32shdr elf32shdr_t;

    struct elf64shdr {
        u32    name;
        u32    type;
        u64    flags;
        addr64 addr;
        off64  offset;
        u64    size;
        u32    link;
        u32    info;
        u64    addralign;
        u64    entsize;
    };
    typedef struct elf64shdr elf64shdr_t;

    /**
     * @brief Parses an ELF file.
     * Also performs basic file boundary checks.
     * @param ptr  Pointer to a ELF file in memory.
     * @param size Size of the ELF file.
     * @return An ELF descriptor or nullptr on failure.
     */
    export elf_t* elf_parse(void* ptr, usz size);

    /**
     * @brief Checks an ELF file.
     * @param elf   ELF descriptor.
     * @param conds An elfcond_t struct.
     * @return Whether the ELF file passed all checks.
     */
    export bool elf_check(elf_t* elf, elfcond_t* conds);

    /**
     * @brief Links ELF sections into memory.
     * @param elf     ELF descriptor.
     * @param data    Private data passed to the callbacks.
     * @param checkcb Callback called to check whether a section should be linked.
     * @param alloccb Callback called to request a mapping.
     * @param linkcb  Callback called when linking. Expects an address. Returning NULL
     * will result in a failure to load. Callback will get called for every symbol despite
     * any failures.
     */
    export bool
    elf_link(elf_t* elf, void* data, bool (*checkcb)(void* data, const char* name),
             void* (*alloccb)(void* data, void* vaddr, usz size, int align, u32 flags),
             void* (*linkcb)(void* data, const char* name, int flags));

    /**
     * @brief Cleans up an ELF descriptor.
     * @param elf ELF descriptor.
     */
    export void elf_close(elf_t* elf);

    /**
     * @brief Retrieves the address of a symbol.
     * Must be used on a loaded ELF file descriptor.
     * @param elf ELF descriptor.
     */
    export void* elf_symbol(elf_t* elf);

#ifdef __cplusplus
}
#endif