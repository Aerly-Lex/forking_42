#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <pthread.h>
#include <immintrin.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef unsigned long u64;

#define PRINT_ERROR(cstring) write(STDERR_FILENO, cstring, sizeof(cstring) - 1)

#pragma pack(1)
struct bmp_header
{
	// Note: header
	i8  signature[2]; // should equal to "BM"
	u32 file_size;
	u32 unused_0;
	u32 data_offset;

	// Note: info header
	u32 info_header_size;
	u32 width; // in px
	u32 height; // in px
	u16 number_of_planes; // should be 1
	u16 bit_per_pixel; // 1, 4, 8, 16, 24 or 32
	u32 compression_type; // should be 0
	u32 compressed_image_size; // should be 0
	// Note: there are more stuff there but it is not important here
};

struct file_content
{
	i8*   data;
	u32   size;
};

struct file_content   read_entire_file(char* filename)
{
	char* file_data = 0;
	unsigned long	file_size = 0;
	int input_file_fd = open(filename, O_RDONLY);
	if (input_file_fd >= 0)
	{
		struct stat input_file_stat = {0};
		stat(filename, &input_file_stat);
		file_size = input_file_stat.st_size;
		file_data = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, input_file_fd, 0);
		close(input_file_fd);
	}
	return (struct file_content){file_data, file_size};
}

// Extract Blue [0] and Red [2]
// return the sum
int	get_length(const u8* pixel)
{
	return (pixel[0] + pixel[2]);
}

// Iterating through rows and colomns to find the header!
const u8*	find_the_header(const u8* image_data, u32 width, u32 height)
{
	// Iterating through rows
	for (u32 y = 0; y < height; y++)
	{
		// Iterating through colomns
		for (u32 x = 0; x < width; x++)
		{
			const u8* pixel = image_data + ((height - 1 - y) * width + x) * 4;
			if (pixel[0] == 127 && pixel[1] == 188 && pixel[2] == 217)
				return (pixel);
		}
	}
	return NULL;
}

void	get_pixel(const u8* pixel, char* buffer)
{
	buffer[0] = (char)pixel[0];
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		PRINT_ERROR("Usage: decode <input_filename>\n");
		return 1;
	}
	struct file_content file_content = read_entire_file(argv[1]);
	if (file_content.data == NULL)
	{
		PRINT_ERROR("Failed to read file\n");
		return 1;
	}
	struct bmp_header* header = (struct bmp_header*) file_content.data;
	printf("signature: %.2s\nfile_size: %u\ndata_offset: %u\ninfo_header_size: %u\nwidth: %u\nheight: %u\nplanes: %i\nbit_per_px: %i\ncompression_type: %u\ncompression_size: %u\n", \
			header->signature, header->file_size, header->data_offset, header->info_header_size, header->width, header->height, header->number_of_planes, header->bit_per_pixel, header->compression_type, header->compressed_image_size);

	// Finding the Header and reading the length
	const u8*	image_data = (const u8*)file_content.data + header->data_offset;
	const u8*	header_pixel = find_the_header(image_data, header->width, header->height);
	for (int i = 0; i < 7; i++) // horizontal line
	{
		header_pixel += 4; // next pixel in the line
		printf("%u %u %u\n", header_pixel[0], header_pixel[1], header_pixel[2]);
	}

	// Reading the length
	int length = get_length(header_pixel);
	printf("Message length: %d\n", length);

	// Reading out the message and printing

	return 0;
}
