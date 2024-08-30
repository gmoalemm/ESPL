#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

extern int startup(int argc,char** argv,int(*func)(int,char**));

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);

void readelfEntry(Elf32_Phdr *entry);

void load_phdr(Elf32_Phdr *phdr, int fd);

int main(int argc, char *argv[])
{
    char *fileName = NULL;
    int fd = -1;
    Elf32_Ehdr *elf_head = NULL;
    off_t fileSize = -1;

    if(argc < 2)
    {
        printf("Usage: %s <file> <args>\n", argv[0]);
        return 1;
    }
    
    fileName = argv[1];

    if((fd = open(fileName, O_RDONLY)) < 0)
    {
        perror("open");
        return 1;
    }

    /* Get the file size */
    fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    elf_head = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);

    if(elf_head == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    puts("Type\t\tOffset\t\tVirtAddr\tPhysAddr\tFileSiz\t\tMemSiz\t\tFlg\tAlign\t\tMap\t\t\tProt");

    foreach_phdr(elf_head, load_phdr, fd);

    startup(argc-1, argv+1, (void *)(elf_head->e_entry));
    
    close(fd);

    return 0;
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;

    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr -> e_phoff);

    int i = 0;

    for(; i < ehdr->e_phnum; i++){
        if (func)
            func(phdr, arg);

        phdr++;
    }

    return 0;
}

void readelfEntry(Elf32_Phdr *entry)
{
    switch (entry->p_type)
    {
    case PT_NULL:
        printf("NULL\t");
        break;
    case PT_LOAD:
        printf("LOAD\t");
        break;
    case PT_DYNAMIC:
        printf("DYNAMIC\t");
        break;
    case PT_INTERP:
        printf("INTERP\t");
        break;
    case PT_NOTE:
        printf("NOTE\t");
        break;
    case PT_SHLIB:
        printf("SHLIB\t");
        break;
    case PT_PHDR:
        printf("PHDR\t");
        break;
    case PT_TLS:
        printf("TLS\t");
        break;
    case PT_NUM:
        printf("NUM\t");
        break;
    default:
        printf("%08x", entry->p_type);
        break;
    }

    printf("\t%08x\t%08x\t%08x\t%08x\t%08x\t", entry->p_offset, entry->p_vaddr, entry->p_paddr, entry->p_filesz, entry->p_memsz);

    if (entry->p_flags & PF_R)
        printf("R");
    else
        printf(" ");

    if (entry->p_flags & PF_W)
        printf("W");
    else
        printf(" ");

    if (entry->p_flags & PF_X)
        printf("E");
    else
        printf(" ");

    printf("\t%08x", entry->p_align);

    if (entry->p_type == PT_INTERP || entry->p_type == PT_DYNAMIC){
        puts("");
        return;
    }

    printf("\tMAP_PRIVATE");

    if (entry->p_vaddr != 0) {
        printf(" MAP_FIXED");
    }
    else
    {
        printf("          ");
    }

    printf("\t");

    if (entry->p_flags & PF_R)
        printf("PROT_READ ");

    if (entry->p_flags & PF_W)
        printf("PROT_WRITE ");

    if (entry->p_flags & PF_X)
        printf("PROT_EXEC");

    puts(" ");
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    int prot = 0;
    Elf32_Off offset;
    void *vaddr = 0;
    int padding;
    void *map;

    if (phdr == NULL) {
        fprintf(stderr, "phdr is NULL\n");
        return;
    }

    if (phdr->p_type != PT_LOAD)
        return;

    readelfEntry(phdr);

    if (phdr->p_flags & PF_R)
        prot |= PROT_READ;
    
    if (phdr->p_flags & PF_W)    
        prot |= PROT_WRITE;

    if (phdr->p_flags & PF_X)
        prot |= PROT_EXEC;

    if (phdr->p_vaddr != 0)
    {
        vaddr = (void*) (phdr->p_vaddr&0xfffff000);
        offset = phdr->p_offset&0xfffff000;
        padding = phdr->p_vaddr & 0xfff;
        map = mmap(vaddr, phdr->p_memsz+padding, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
    }
    else
    {
        map = mmap(NULL, phdr->p_memsz, prot, MAP_PRIVATE, fd, phdr->p_offset);
    }

    if (map == MAP_FAILED)
    {
        perror("mmap");
        return;
    }
}
