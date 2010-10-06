	
void run_elf(char *elf_filename) {
	struct vxproc *p = vxproc_alloc();
	vxproc_loadelffile(p, elf_filename, ...);

	for (;;) {
		int rc = vxproc_run(p); // run the binary!
		switch(rc) {
		case VXTRAP_SYSCALL:
			switch(p->cpu->reg[EAX]) {
			case VXSYSWRITE:
				... handle the syscall ...
				break;
			...
			}
			break;

		default:
			// handle other traps - like segv
			printf("vxproc_run trap %#x\n", rc);
			result = -1;
			goto out;
		}
	}

out:
	vxproc_free(p);
}


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
