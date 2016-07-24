#include <stdio.h>
#include <string.h>

#include "merger.h"

void usage() {
	printf("usage: \n");
	printf("merger [file01] [file02] [file03] ... [output_file] | merge the list of files\n");
	printf("merger [directory] [output_file] | merge the files in the directory\n");
	printf("merger -s [merged_file] [output_directory] | splits the merged files\n");
}

int main(int argc, char **argv) {
	if(argc < 3) {
		usage();
		return 1;
	}

	if(!strcmp(argv[1], "-s")) {
		char *merged_file = argv[2];
		char *output_dir = argv[3];
		split(merged_file, output_dir);
		return 0;
	}

	char *output_file = argv[argc - 1];
	int input_count = argc - 2;
	char *input_files[input_count];
	for(int i = 1; i < argc - 1; i++) {
		input_files[i - 1] = argv[i];
	}
	merge(input_count, input_files, output_file);
	return 0;
}
