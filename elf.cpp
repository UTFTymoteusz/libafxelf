#include "afx/elf.h"

#include "elf/checks.hpp"
#include "host.h"

namespace afx {
    static elf_t* parse32(void* ptr, usz size);
    static elf_t* parse64(void* ptr, usz size);
    static bool   check32(elf_t* elf, elfcond_t* cond);
    static bool   check64(elf_t* elf, elfcond_t* cond);

    struct linkaddr {
        const char* name;
        void*       addr;
    };

    union {
        u8  a;
        u16 b;
    } endian = {.b = 0x00AA};

    template <typename A, typename B>
    bool inbounds(A val, B max) {
        return val < max;
    }

    template <typename A, typename B, typename C>
    bool inbounds(A val, B size, C max) {
        return val < max && size < max && val + size < max;
    }

    u16 endflip(u16 i) {
        return ((i & 0xFF) >> 8) | (i << 8);
    }

    u32 endflip(u32 i) {
        return ((i & 0xFF000000) >> 24) | ((i & 0x00FF0000) >> 8) |
               ((i & 0x0000FF00) << 8) | ((i & 0x000000FF) << 24);
    }

    u64 endflip(u64 i) {
        return ((i & 0xFF00000000000000) >> 56) | ((i & 0x00FF000000000000) >> 48) |
               ((i & 0x0000FF0000000000) >> 40) | ((i & 0x000000FF00000000) >> 32) |
               ((i & 0x00000000FF000000) << 32) | ((i & 0x0000000000FF0000) << 40) |
               ((i & 0x000000000000FF00) << 48) | ((i & 0x00000000000000FF) << 56);
    }

    template <typename A>
    A fl(A val, bool flip) {
        return flip ? endflip(val) : val;
    }

    const char* strderef(elf_t* elf, int offset) {
        return elf->strings + offset;
    }

    elf_t* elf_parse(void* ptr, usz size) {
        u8* ident = (u8*) ptr;

        if (ident[0] != ELFMAG0 || ident[1] != ELFMAG1 || ident[2] != ELFMAG2 ||
            ident[3] != ELFMAG3)
            return nullptr;

        if (ident[EI_VERSION] != EV_CURRENT)
            return nullptr;

        if (ident[EI_CLASS] == ELFCLASS32)
            return parse32(ptr, size);
        else if (ident[EI_CLASS] == ELFCLASS64)
            return parse64(ptr, size);
        else
            return nullptr;
    }

    bool elf_check(elf_t* elf, elfcond_t* conds) {
        if (!elf)
            return false;

        return elf->bits == 32 ? check32(elf, conds) : check64(elf, conds);
    }

    bool elf_load(elf_t* elf, void* data,
                  void* (*alloccb)(void* data, void* vaddr, usz size, int align,
                                   u32 flags),
                  void* (*freecb)(void* data, void* vaddr)) {
        if (elf->loaded)
            return false;

        elf64* in   = (elf64*) elf->image;
        bool   flip = elf->flip;

        for (u16 i = 0; i < in->phnum; i++) {
            elf64phdr* phdr = (elf64phdr*) ((u8*) elf->image + fl(in->phoff, flip) +
                                            i * fl(in->phentsize, flip));

            alloccb(data, (void*) phdr->vaddr, phdr->memsz, phdr->align, phdr->flags);
        }

        elf->loaded = true;
        return true;
    }

    bool elf_link(elf_t* elf, void* data, bool (*checkcb)(void* data, const char* name),
                  void* (*alloccb)(void* data, void* vaddr, usz size, int align,
                                   u32 flags),
                  void* (*linkcb)(void* data, const char* name, int flags)) {
        if (elf->loaded)
            return false;

        linkaddr* addrv = (linkaddr*) afxhost_malloc(sizeof(linkaddr) * 32);
        usz       addrc = 0;
        usz       addrs = 32;
        elf64*    in    = (elf64*) elf->image;
        bool      flip  = elf->flip;

        for (u16 i = 1; i < in->shnum; i++) {
            if (!shvalid(i))
                continue;

            elf64shdr* shdr = (elf64shdr*) ((u8*) elf->image + fl(in->shoff, flip) +
                                            i * fl(in->shentsize, flip));

            if (!checkcb(data, strderef(elf, shdr->name)))
                continue;
        }

        afxhost_free(addrv);

        elf->loaded = true;
        return true;
    }

    elf_t* parse32(void* ptr, usz size) {
        /*elf32* in   = (elf32*) ptr;
         bool   flip = in->ident[EI_DATA] != (endian.a ? ELFDATA2LSB : ELFDATA2MSB);

         if (!inbounds(fl(in->phoff, flip), size) || !inbounds(fl(in->shoff, flip), size))
             return nullptr;

         if (!inbounds(fl(in->phoff, flip), fl(in->phnum, flip) * fl(in->phentsize, flip),
                       size))
             return nullptr;

         if (!inbounds(fl(in->shoff, flip), fl(in->shnum, flip) * fl(in->shentsize, flip),
                       size))
             return nullptr;

         for (u16 i = 0; i < in->phnum; i++) {
             elf32phdr* phdr = (elf32phdr*) ((u8*) ptr + fl(in->phoff, flip) +
                                             i * fl(in->phentsize, flip));

             if (!inbounds(fl(phdr->offset, flip), fl(phdr->filesz, flip), size))
                 return nullptr;
         }

         if (fl(in->shstrndx, flip) >= in->shnum)
             return nullptr;

         elf32shdr* strhdr = (elf32shdr*) ((u8*) ptr + fl(in->shoff, flip) +
                                           in->shstrndx * fl(in->shentsize, flip));

         if (!inbounds(fl(strhdr->offset, flip), fl(strhdr->size, flip), size))
             return nullptr;

         for (u16 i = 0; i < in->shnum; i++) {
             elf32shdr* shdr = (elf32shdr*) ((u8*) ptr + fl(in->shoff, flip) +
                                             i * fl(in->shentsize, flip));

             if (!inbounds(fl(shdr->offset, flip), fl(shdr->size, flip), size))
                 return nullptr;

             if (!inbounds(fl(shdr->name, flip), fl(strhdr->size, flip)))
                 return nullptr;
         }

         for (u16 i = 0; i < in->shnum; i++) {
             elf32shdr* shdr = (elf32shdr*) ((u8*) ptr + fl(in->shoff, flip) +
                                             i * fl(in->shentsize, flip));

             if (!inbounds(fl(shdr->offset, flip), fl(shdr->size, flip), size))
                 return nullptr;
         }

         elf* out = (elf*) afxhost_malloc(sizeof(elf));

         out->bits     = 32;
         out->type     = flip ? endflip(in->type) : in->type;
         out->machine  = flip ? endflip(in->machine) : in->machine;
         out->entry    = (void*) (usz) (flip ? endflip(in->entry) : in->entry);
         out->image    = ptr;
         out->loaded   = false;
         out->flip     = flip;
         out->strindex = fl(in->shstrndx, flip);

         return out;*/

        return nullptr;
    }

    elf_t* parse64(void* ptr, usz size) {
        elf64* in   = (elf64*) ptr;
        bool   flip = in->ident[EI_DATA] != (endian.a ? ELFDATA2LSB : ELFDATA2MSB);

        if (!inbounds(fl(in->phoff, flip), size) || !inbounds(fl(in->shoff, flip), size))
            return nullptr;

        if (!inbounds(fl(in->phoff, flip), fl(in->phnum, flip) * fl(in->phentsize, flip),
                      size))
            return nullptr;

        if (!inbounds(fl(in->shoff, flip), fl(in->shnum, flip) * fl(in->shentsize, flip),
                      size))
            return nullptr;

        for (u16 i = 0; i < in->phnum; i++) {
            elf64phdr* phdr = (elf64phdr*) ((u8*) ptr + fl(in->phoff, flip) +
                                            i * fl(in->phentsize, flip));

            if (!inbounds(fl(phdr->offset, flip), fl(phdr->filesz, flip), size))
                return nullptr;
        }

        if (fl(in->shstrndx, flip) >= in->shnum)
            return nullptr;

        if (!shvalid(fl(in->shstrndx, flip)))
            return nullptr;

        elf64shdr* strhdr = (elf64shdr*) ((u8*) ptr + fl(in->shoff, flip) +
                                          in->shstrndx * fl(in->shentsize, flip));

        if (!inbounds(fl(strhdr->offset, flip), fl(strhdr->size, flip), size))
            return nullptr;

        for (u16 i = 1; i < in->shnum; i++) {
            if (!shvalid(i))
                continue;

            elf64shdr* shdr = (elf64shdr*) ((u8*) ptr + fl(in->shoff, flip) +
                                            i * fl(in->shentsize, flip));

            if (!inbounds(fl(shdr->offset, flip), fl(shdr->size, flip), size))
                return nullptr;

            if (!inbounds(fl(shdr->name, flip), fl(strhdr->size, flip)))
                return nullptr;
        }

        elf* out = (elf*) afxhost_malloc(sizeof(elf));

        out->bits     = 64;
        out->type     = flip ? endflip(in->type) : in->type;
        out->machine  = flip ? endflip(in->machine) : in->machine;
        out->entry    = (void*) (usz) (flip ? endflip(in->entry) : in->entry);
        out->image    = ptr;
        out->loaded   = false;
        out->flip     = flip;
        out->strindex = fl(in->shstrndx, flip);
        out->strings  = (const char*) ptr + strhdr->offset;

        return out;
    }

    bool check32(elf_t* elf, elfcond_t* cond) {
        return true;
    }

    bool check64(elf_t* elf, elfcond_t* cond) {
        return true;
    }
}