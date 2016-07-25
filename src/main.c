#include <stdio.h>
#include <string.h>

#include "merger.h"

void usage() {
	printf("usage: \n");
	printf("\tmerger [file01] [file02] [file03] ... [output_file] | merge the list of files\n");
	printf("\tmerger [directory] [output_file] | merge the files in the directory\n");
	printf("\tmerger -s [merged_file] [output_directory] | splits the merged files\n");
}

int main(int argc, char **argv) {
	if(argc < 3) {
		usage();
		return 1;
	}

	for(int i = 0; i < argc; i++) {
		if(!strcmp(argv[i], "--help")) {
			usage();
			return 0;
		}
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
