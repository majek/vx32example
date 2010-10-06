#include <stdio.h>
#include <stdlib.h>

#include "libvx32/vx32.h"
#include "libvx32/args.h"

#define syscall xxxsyscall // don't redefine 'syscall' function. FIXME
#include "libvxc/syscall.h"

void syscall_brk(vxproc *proc);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s elf_binary\n", argv[0]);
		abort();
	}
	char *elf_filename = argv[1];
	
	vx32_siginit();

	int result = 0;
	struct vxproc *p = vxproc_alloc();
	const char *p_argv[] = {elf_filename, NULL};
	const char *p_env[] = {NULL, NULL};

	int r = vxproc_loadelffile(p, elf_filename, p_argv, p_env);
	if(r < 0) {
		printf("vxproc_loadelffile(\"%s\")\n", elf_filename);
		result = -1;
		goto out;
	}

	for (;;) {
		int rc = vxproc_run(p);
		switch(rc) {
		case VXTRAP_SYSCALL:
			switch(p->cpu->reg[EAX]) {
			case VXSYSBRK:
				syscall_brk(p);
				break;

			case VXSYSEXIT:
				printf("exited with %i\n", p->cpu->reg[EDX]);
				goto out;
			case VXSYSSTAT:
			case VXSYSFSTAT:
				p->cpu->reg[EAX] = -EINVAL;
				break;
			case VXSYSWRITE: {
				uint32_t addr = p->cpu->reg[ECX];
				uint32_t len = p->cpu->reg[EBX];
				int r = 0;
				if (!vxmem_checkperm(p->mem, addr, len,
						     VXPERM_READ, NULL)) {
					r = -EINVAL;
				} else {
					vxmmap *m = vxmem_map(p->mem, 0);
					r = printf("%s", (char*)m->base + addr);
				}
				p->cpu->reg[EAX] = r;
				break; }
			default:
				printf("bad syscall %#x\n", p->cpu->reg[EAX]);
				result = -1;
				goto out;
			}
			break;

		default:
			printf("vxproc_run trap %#x\n", rc);  // like segv
			result = -1;
			goto out;
		}
	}

out:
	vxproc_free(p);
	return result;
}



/* Don't worry about this one, it's usually a boilerplate. */
void syscall_brk(vxproc *proc) {
        uint32_t arg1 = proc->cpu->reg[EDX];
        uint32_t oaddr;
        uint32_t addr = arg1;
        uint32_t inc = 1<<20;
        int ret = 0;
	vxmmap *m = vxmem_map(proc->mem, 0);

        addr = (addr + inc - 1) & ~(inc - 1);
        oaddr = m->size;
        if(addr == oaddr) {
                ret = 0;
		goto out;
        }

        if(addr > m->size) {
                ret = vxmem_resize(proc->mem, addr);
                if(ret < 0) {
                        printf("sbrk failed. caller will be unhappy!\n");
			ret = -EINVAL;
			goto out;
                }
        }
        if (ret >= 0) {
                if (addr > oaddr) {
                        ret = vxmem_setperm(proc->mem, oaddr, addr - oaddr,
                                                      VXPERM_READ|VXPERM_WRITE);
                        if(ret < 0) {
                                printf("setperm is failing!\n");
				ret = -EINVAL;
				goto out;
                        }
                }
        }
out:;
        proc->cpu->reg[EAX] = ret;
}
