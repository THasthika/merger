#ifndef MERGER_H
#define MERGER_H 1

#include <sys/types.h>
#include <sys/stat.h>
#include <list.h>

#define PADDING_SIZE 1024

struct path_info {
	char path[4096];
	off_t size;
	mode_t mode;
};

struct file_info {
	char name[512];
	off_t size;
	mode_t mode;
};

struct merged_file_info {
	char padding[PADDING_SIZE];
	int count;
};

void merge(int input_count, char **input_files, char *output_file);
void split(char *merged_file, char *output_file);
#endif
