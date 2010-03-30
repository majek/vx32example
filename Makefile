VX32DIR = vx32/src
VXCDIR=$(VX32DIR)/libvxc

CC = gcc
LD = $(CC)
LDFLAGS =
CFLAGS = -Wall -I$(VX32DIR)

OBJS =	\
	vx32example.o	\
	$(VX32DIR)/libvx32/libvx32.a \

VX32_CC = $(CC)
VX32_CFLAGS = -m32 -Wall -O2 -Wl,-melf_i386 -nostdlib -mfp-ret-in-387 \
		-I$(VXCDIR)/include -L$(VXCDIR) 
VX32_LIBS=$(VXCDIR)/vx32/crt0.o -lc -lgcc

all: $(VX32DIR) vx32example payload
	@echo "[*] Running untrusted code inside vx32"
	./vx32example ./payload

vx32example: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

payload: $(VXCDIR)/vx32/crt0.o $(VXCDIR)/libc.a payload.c
	$(VX32_CC) $(VX32_CFLAGS) \
		-o payload payload.c \
		$(VX32_LIBS)


$(VX32DIR):
	hg clone http://hg.pdos.csail.mit.edu/hg/vx32/


$(VX32DIR)/libvx32/libvx32.a:
	make -C $(VX32DIR) libvx32/libvx32.a

$(VXCDIR)/vx32/crt0.o:
	make -C $(VX32DIR) libvxc/vx32/crt0.o

$(VXCDIR)/libc.a:
	make -C $(VX32DIR) libvxc/libc.a

clean:
	make -C $(VX32DIR) clean
	rm -f *.o payload vx32example


