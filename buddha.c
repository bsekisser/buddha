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

#define WRITE_ENABLE 0
#define DUMP_BLOCKS 0
#define DUMP_BLOCK_PARTS 0

//#include "crc16.c"
#define crc_16(_data, _len) 0

//#include "crc32_simple.c"
#define crc32(_data, _len, _out)

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

void fd_header_crc(uint8_t* src, uint32_t start, uint32_t end)
{
	TRACE();

	for(uint32_t offset = start; offset < end; offset += 32) {
//		uint8_t* pat = &src[offset];

		uint16_t crc16_out = crc_16(pat, 32);
		uint32_t crc32_out = 0;

		crc32(pat, 32, &crc32_out);

		printf("0x%08x: crc16 = 0x%04x, crc32 = 0x%08x\n", offset, crc16_out, crc32_out);
	}
}

static uint32_t xor32_at(uint8_t* src, uint32_t key)
{
	uint32_t data = 0;

//	printf("\n");

	for(int i = 4; i > 0; i--)
	{
		uint8_t c = *src++;
		data = (data << 8) | c;
//		printf("(0x%08x, 0x%02x) ", data, c);
	}

	data ^= key;

//	printf("\n");

	return(data);
}

void fd_dump(uint8_t* src, uint32_t start, uint32_t end)
{
	uint8_t* limit = &src[end];
	uint8_t* pat = &src[start];
	
	while(limit > pat)
	{
		printf("0x%08x: ", pat - src);
		
//		for(int i = 0; i < 32; i++)
//			printf("%02x ", pat[i]);
		
		uint32_t offset = xor32_at(pat + 4, 0x1F3E7CF8);
		uint32_t length = xor32_at(pat + 8, 0xF0C1A367);

		uint32_t eend = offset + length;
			
		if((offset > 0) && (offset < sb.st_size) && (offset + length < sb.st_size)) {
			if(DUMP_BLOCK_PARTS)
				fd_copy("dumps/part/buddha", src, offset, eend);

if(0) {
			const char blockstart[] = { 0xCB, 0x80, 0xBF, 0xF0,
				0x06, 0x67, 0x72, 0xAA,
				0x66, 0x17, 0xF7, 0x00 };

			if(0 == strncmp(&blockstart, &src[offset], 11))
				printf(" -- blockstart match\n");
}

		for(int i = 0; i < 32; i++)
			printf("%02x ", (&src[offset])[i]);
			
		}

		printf("\n");

//		printf(" -- 0x%08x, \n",
//			xor32_at(pat, 0xefff1d5c));
//			xor32_at(pat, 0x0000fe00 - 1));
//			xor32_at(pat + 4, 0x00000600));


//		printf(" -- offset = 0x%08x, length = 0x%08x\n", offset, length);
		
		pat += 32;
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

void fd_dump_xor_c16(uint8_t* src, uint32_t start, uint32_t end, uint8_t* xor)
{
	uint8_t* limit = &src[end];
	uint8_t* pat = &src[start];

	while(limit > pat)
	{
		printf("0x%08x: ", pat - src);
		
		for(int i = 32; i > 0; i--)
			printf("%02x ", *pat++ ^ xor[i & 0xf]);
		
		printf("\n");
	}
}

void fd_dump_xor_shift(uint8_t* src, uint32_t start, uint32_t end, uint32_t shift)
{
	uint8_t* limit = &src[end - shift];
	uint8_t* pat = &src[start];

	uint8_t* pxor = &src[shift];
	
	while(limit > pat)
	{
		printf("0x%08x: ", pat - src);
		
		for(int i = 32; i > 0; i--)
			printf("%02x ", pat[i & 0x1f] ^ pxor[i & 0x1f]);
		
		pat += 32;
		pxor += 32;
		
		printf("\n");
	}
	
	printf("\n");
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

//	fd_dump(bin, 0x0000, 0x05ff);
	fd_dump(bin, 0x0000, 0x05ff);
	
//	for(int i = 1; i < 15; i++)
//		fd_dump_xor_shift(bin, 0x0000, 0x00ff, i);

//	fd_dump_xor(bin, 0x0003, 0x05ff, 0x1F3E7CF8);
//	fd_dump_xor(bin, 0x0003, 0x05ff, 0x1F3E7CF8);
//	fd_dump_xor(bin, 0x0603, 0x06ff, 0xF0C1A367);
//	fd_dump_xor_c16(bin, 0x0000, 0x05ff, "\xef\xff\xff\xde\x1f\xd0\x82\xe7\xf0\x00 A\xef\xff\xdf");
//	fd_dump_xor_c16(bin, 0x0000, 0x05ff, "\xef\xff\xff\xde\x1f\xd0\x82\xe7\xf0\x00 A\xef\xff\x0f");
	/* **** */

	munmap(bin, sb.st_size);
	
	return(0);
}
