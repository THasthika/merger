#ifndef MERGER_H
#define MERGER_H 1

#include <sys/types.h>
#include <sys/stat.h>
#include <THB/list.h>

struct path_info {
	char path[4096];
	off_t size;
	off_t ssize;
	mode_t mode;
};

struct file_info {
	char name[512];
	off_t size;
	off_t ssize;
	mode_t mode;
};

struct merged_file_info {
	int count;
};

void merge(int input_count, char **input_files, char *output_file);
void split(char *merged_file, char *output_file);
#endif
