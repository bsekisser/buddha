#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/shift_roll.h"

#define WRITE_ENABLE 0
#define DUMP_BLOCKS 0
#define DUMP_BLOCK_PARTS 0

#define TRACE(_f, args...) \
		printf("%s @ %u: " _f "\n", __FUNCTION__, __LINE__, ##args)

struct stat sb;

int fd_copy(const char* path, uint8_t* raw, uint32_t start, uint32_t end)
{
	char buf[256];
	snprintf(buf, 0xff, "%s-0x%06x-0x%06x", path, start, end);

	uint8_t* src = &raw[start];
	uint32_t len = end - start;
	
	printf("%s, end = 0x%08x, len = 0x%08x\n", buf, end - start, len);

	int wfd = 0;
	if(WRITE_ENABLE) {
		wfd = creat(buf, /*S_IRWXU | */S_IRUSR | S_IRGRP | S_IROTH);
		if(-1 == wfd)
			{ perror("fd_copy -- open"); return(-1); }

		int err = write(wfd, src, len);
		if(-1 == err)
			{ perror("fd_copy -- write"); return(-1); }
	
		close(wfd);
	}

	return(0);
}

static	uint8_t guessed_key[] = {
//			0xef, 0xff, 0x87, 0xcf,
			0xef, 0xff, 0xdf, 0x9f,
//	1110 1111, 1111 1111, 1101 1111, 1001 1111
			0x1f, 0x3e, 0x7c, 0xf8,		//	2	--	OK
//	0001 1111, 0011 1110, 0111 1100, 1111 0000
			0xf0, 0xc1, 0xa3, 0x67,		//	3	--	OK
//	1111 0000, 1100 0001, 1010 0011, 0110 1110
			0xef, 0xff, 0xdf, 0xcf,
//	1110 1111, 1111 1111, 1000 0111, 1100 1111
/* **** */
//			0x50, 0x4e, 0xc9, 0xde,
			0x50, 0x4c, 0xcc, 0xde,
//	0101 0000, 0010 1111, 1100 1001, 1101 1110
			0x86, 0xd0, 0x82, 0xe7,
//	1000 0110, 1101 0000, 1000 0010, 1110 1000
			0x10, 0x00, 0x20, 0x41,
//	0001 0000, 0000 0000, 0010 0000, 0100 0001
			0x83, 0x07, 0x0f, 0x3e,
//	1000 0011, 0000 0111, 0000 1111, 0011 1110
};

void dump_char(uint8_t* src, uint32_t start, uint32_t len)
{
	uint8_t* pat = &src[start];

	while(len--) {
		uint8_t c = *pat++;
		c = isprint(c) ? c : ' ';

		printf("%c", c);
	}
}


void dump_hex(uint8_t* src, uint32_t start, uint32_t len)
{
	uint8_t* pat = &src[start];

	while(len--)
		printf("%02x ", *pat++);
}

static uint32_t generate_key_next(uint32_t key) {
	key <<= 1;
	if(key & 0x0200)
		key ^= 0x21;

	return(key);
}

static uint8_t* generate_key(uint8_t *key) {
	uint32_t shift_reg = 0xff;

	for(int i = 0; i < 256; i++)
	{
		shift_reg = generate_key_next(shift_reg);
		key[i] = shift_reg & 0xff;
	}
	
	return(key);
}

void fd_dump_xor_21(uint8_t* src, uint32_t start, uint32_t end)
{
	uint8_t line[256];
	const int lcw = 32;

	for(int offset = start; offset < end; offset += lcw)
	{
		uint8_t* pat = &src[offset];
		printf("0x%04x: ", offset);
		
		/*	encoded hex dump */

//		dump_hex(src, offset, lcw);

		printf(" -- ");

		/* encoded text dump */

//		dump_char(src, offset, lcw);

//		printf("  ----  ");

	uint32_t shift_reg = 0xef;

		for(int i = 0; i < lcw; i++)
		{
			shift_reg = generate_key_next(shift_reg);
			line[i] = pat[i] ^ (shift_reg & 0xff);
		}

		/* decoded hex dump */
		
		dump_hex(line, 0, lcw);

		printf(" -- ");

		/* decoded text dump */

		dump_char(line, 0, lcw);

		printf("\n");
	}
	
}

void fd_dump_xor_key(uint8_t* src, uint32_t start, uint32_t end, uint8_t* key)
{
	uint8_t	line[32];
	const int lcw = 16;

	for(int offset = start; offset < end; offset += lcw)
	{
		uint8_t* pat = &src[offset];
		printf("0x%04x: ", offset);
		
		if(1) {
			/*	encoded hex dump */

			dump_hex(src, offset, lcw);

			printf(" -- ");

			/* encoded text dump */

			dump_char(src, offset, lcw);

			printf("  ----  ");
		}

		for(int i = 0; i < lcw; i++)
		{
			line[i] = pat[i] ^ key[(offset + i) & 0x1f];
		}

		/* decoded hex dump */
		
		dump_hex(line, 0, lcw);

		printf(" -- ");

		/* decoded text dump */

		dump_char(line, 0, lcw);

		printf("\n");
	}
}

void dump_hex_block(uint8_t* src, uint32_t start, uint32_t len)
{
	const int lcw = 32;
	uint32_t offset = start;

	do {
		printf("0x%04x: ", offset);

		dump_hex(src, offset, lcw);

		printf("\n");

		len -= lcw;
		offset += lcw;
	} while(len);
}

int main(void)
{
	int fd = open("archive/buddha.bin", O_RDONLY);
	if(-1 == fd)
		{ perror("open"); return(-1); }
	
	if(-1 == fstat(fd, &sb))
		{ perror("stat"); return(-1); }
	
	uint8_t* bin = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if(MAP_FAILED == bin)
		{ perror("mmap"); return(-1); }
	
	close(fd);
	
	/* **** */

	if(DUMP_BLOCKS) {
		const char* path = "dumps/block/buddha";
		fd_copy(path, bin, 0x0000, 0x05ff);
		fd_copy(path, bin, 0x0600, 0x7dff);
		fd_copy(path, bin, 0x7e00, 0x8dff);
		fd_copy(path, bin, 0x8e00, 0x96ff);
		fd_copy(path, bin, 0x9700, sb.st_size);
	}

	uint8_t key[256], *p2key = generate_key(&key);

	fd_dump_xor_21(bin, 0x0000, 0x05ff);
//	fd_dump_xor_key(bin, 0x0000, 0x05ff, &p2key[14]);

	printf("\n\n");

	dump_hex_block(p2key, 0, 256);

	/* **** */

	munmap(bin, sb.st_size);
	
	return(0);
}
