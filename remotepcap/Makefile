all : burn watch

REMOTEIP?=192.168.1.1
ROOTPASS?=thumpingbunnies93
SSHLINE?=sshpass -p $(ROOTPASS) ssh root@$(REMOTEIP)
CFLAGS:=$(CFLAGS) -I../util 
CFLAGS:=$(CFLAGS) -DMAKE_PCAP_STREAM
CC:=/win/lede/source/staging_dir/toolchain-mips_24kc_gcc-5.5.0_musl/bin/mips-openwrt-linux-gcc

#mips-linux-gnu-gcc

#LDFLAGS:=dev/libgcc_s.so.1 dev/libc.so -nostdlib

remotepcap : remotepcap.c ../util/librawp.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

burn : remotepcap
	cat remotepcap | $(SSHLINE) "cat - > /tmp/remotepcap; chmod +x /tmp/remotepcap"

watch : burn
	$(SSHLINE) "/tmp/remotepcap" | sudo wireshark -k -i -

clean :
	rm -rf *.o *~ remotepcap
