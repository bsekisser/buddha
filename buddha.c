#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int fd_copy(const char* path, uint8_t* raw, uint32_t start, uint32_t end)
{
	char buf[256];
	snprintf(buf, 0xff, "%s-0x%06x-0x%06x", path, start, end);

	int do_write = 1;

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

void fd_header_list(uint8_t* src, uint32_t start, uint32_t end)
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

	fd_header_list(bin, 0x0004, 0x05ff);

	/* **** */

	munmap(bin, sb.st_size);
	
	return(0);
}
