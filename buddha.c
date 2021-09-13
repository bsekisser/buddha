#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//#include "crc16.c"
#define crc_16(_data, _len) 0

//#include "crc32_simple.c"
#define crc32(_data, _len, _out)

#define TRACE(_f, args...) \
		printf("%s @ %u: " _f "\n", __FUNCTION__, __LINE__, ##args)

int fd_copy(const char* path, uint8_t* raw, uint32_t start, uint32_t end)
{
	char buf[256];
	snprintf(buf, 0xff, "%s-0x%06x-0x%06x", path, start, end);

	int do_write = 0;

	int wfd = 0;
	if(do_write) {
		wfd = creat(buf, /*S_IRWXU | */S_IRUSR | S_IRGRP | S_IROTH);
		if(-1 == wfd)
			{ perror("fd_copy -- open"); return(-1); }
	}

	uint8_t* src = &raw[start];
	uint32_t len = end - start;
	
	printf("%s, end = 0x%08x, len = 0x%08x\n", buf, end - start, len);

	if(do_write) {
		int err = write(wfd, src, len);
		if(-1 == err)
			{ perror("fd_copy -- write"); return(-1); }
	
		close(wfd);
	}

	return(0);
}

void fd_header_crc(uint8_t* src, uint32_t start, uint32_t end)
{
	TRACE();

	for(uint32_t offset = start; offset < end; offset += 32) {
		uint8_t* pat = &src[offset];

		uint16_t crc16_out = crc_16(pat, 32);
		uint32_t crc32_out = 0;

		crc32(pat, 32, &crc32_out);

		printf("0x%08x: crc16 = 0x%04x, crc32 = 0x%08x\n", offset, crc16_out, crc32_out);
	}
}

void fd_dump(uint8_t* src, uint32_t start, uint32_t end)
{
	uint8_t* limit = &src[end];
	uint8_t* pat = &src[start];
	
	while(limit > pat)
	{
		printf("0x%08x: ", pat - src);
		
		for(int i = 32; i > 0; i--)
			printf("%02x ", *pat++);
		
		printf("\n");
	}

	if(1) for(uint32_t offset = start; offset < end; offset += 32) {
		uint8_t* pat = &src[offset];

		printf("offset = 0x%08x, length = 0x%08x, -offset = 0x%08x, -length = 0x%08x\n",
			(*(uint32_t*)&pat[4]) ^ 0x1F3E7CF8, (*(uint32_t*)&pat[8]) ^ 0xF0C1A367);
	}
}

void fd_dump_xor(uint8_t* src, uint32_t start, uint32_t end, uint32_t xor)
{
	uint8_t* limit = &src[end];
	uint8_t* pat = &src[start];

	uint8_t* pxor = (uint8_t*)&xor;
	
	while(limit > pat)
	{
		printf("0x%08x: ", pat - src);
		
		for(int i = 32; i > 0; i--)
			printf("%02x ", *pat++ ^ pxor[i & 3]);
		
		printf("\n");
	}
}


int main(void)
{
	int fd = open("buddha.bin", O_RDONLY);
	if(-1 == fd)
		{ perror("open"); return(-1); }
	
	struct stat sb;
	if(-1 == fstat(fd, &sb))
		{ perror("stat"); return(-1); }
	
	uint8_t* bin = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if(MAP_FAILED == bin)
		{ perror("mmap"); return(-1); }
	
	close(fd);
	
	/* **** */

	if(1) {
		fd_copy("buddha", bin, 0x0000, 0x05ff);
		fd_copy("buddha", bin, 0x0600, 0x7dff);
		fd_copy("buddha", bin, 0x7e00, 0x8dff);
		fd_copy("buddha", bin, 0x8e00, 0x96ff);
		fd_copy("buddha", bin, 0x9700, sb.st_size);
	}

//	fd_dump(bin, 0x0000, 0x05ff);
	fd_dump(bin, 0x0000, 0x00ff);
//	fd_dump_xor(bin, 0x0003, 0x05ff, 0x1F3E7CF8);
//	fd_dump_xor(bin, 0x0003, 0x05ff, 0x1F3E7CF8);
//	fd_dump_xor(bin, 0x0603, 0x06ff, 0xF0C1A367);

	/* **** */

	munmap(bin, sb.st_size);
	
	return(0);
}
