#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <list.h>
#include <queue.h>

#include "merger.h"

#define BUFFER_SIZE 4096

void write_file(int fd, int output_fd) {

	void *buffer = malloc(BUFFER_SIZE);

	ssize_t read_count;

	memset(buffer, 0, BUFFER_SIZE);

	while((read_count = read(fd, buffer, BUFFER_SIZE)) > 0) {
		if(write(output_fd, buffer, read_count) == -1) {
			printf("%s\n", strerror(errno));
			free(buffer);
			exit(1);
		}
		memset(buffer, 0, BUFFER_SIZE);
	}

	free(buffer);
}

void get_file_name(char *path, char **name) {
	char *l_del = path;	
	for(int i = 0; i < strlen(path); i++) {
		if(path[i] == '/') {
			l_del = path + i + 1;
		}
	}
	*name = l_del;
}

void scan_dir(AL_List *list, char *directory) {
        DIR *dp;
	struct dirent *files;

	char path[4096];

	if((dp = opendir(directory)) == NULL) {
		printf("%s: %s\n", directory, strerror(errno));
		return;
	}

	struct stat stat_buffer;
	struct path_info path_info_buffer;

	while((files = readdir(dp)) != NULL) {
		if(!strcmp(files->d_name, ".") || !strcmp(files->d_name, ".."))
			continue;
		strcpy(path, directory);
		if(path[strlen(path) - 1] != '/')
			strcat(path, "/");
		strcat(path, files->d_name);
		if(stat(path, &stat_buffer) == -1) {
			printf("skipping %s: %s\n", path, strerror(errno));
			continue;
		}
		if(S_ISDIR(stat_buffer.st_mode)) {
			printf("skipping %s: is a directory\n", path);
			continue;
		}

		strcpy(path_info_buffer.path, path);
		path_info_buffer.size = stat_buffer.st_size;
		path_info_buffer.mode = stat_buffer.st_mode;

		AL_list_insert_before(list, NULL, &path_info_buffer);
	}
}

void write_split_file(int rfd, int wfd, struct file_info *fi) {
	void *buffer = malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);

	size_t read_size = BUFFER_SIZE;

	if(fi->size < BUFFER_SIZE) read_size = fi->size;

	while(read(rfd, buffer, read_size) > 0 && fi->size > 0) {
		if(write(wfd, buffer, read_size) == -1) {
			printf("%s: %s\n", fi->name, strerror(errno));
			exit(1);
		}
		
		memset(buffer, 0, BUFFER_SIZE);
		fi->size -= read_size;
		if(fi->size < BUFFER_SIZE) read_size = fi->size;
	}

	free(buffer);
}

void split_files(int fd, AL_Queue *queue, char *output_dir) {
	
	struct file_info fi;

	char path[4096];

	int i = 1;
	int total = AL_list_count(queue);

	while(AL_list_count(queue) > 0) {
		AL_queue_dequeue(queue, (void*)&fi);

		strcpy(path, output_dir);
		strcat(path, "/");
		strcat(path, fi.name);

		int wfd = creat(path, fi.mode);
		
		write_split_file(fd, wfd, &fi);

		printf("%d/%d\t %s ...done\n", i++, total, path);

		close(wfd);
	}
}

void merge(int input_count, char **input_files, char *output_file) {
	struct stat stat_buffer;
	struct path_info path_info_buffer;

	short is_regs = 0;
	
	if(stat(output_file, &stat_buffer) != -1) {
		printf("%s: %s\n", output_file, "already exists!");
		return;
	}

	AL_List *path_list = (AL_List*)malloc(sizeof(AL_List));
	AL_list_create(path_list, sizeof(struct path_info), NULL);

	for(int i = 0; i < input_count; i++) {
		char *f = input_files[i];
		if(stat(f, &stat_buffer) == -1) {
			printf("%s: %s\n", f, strerror(errno));
			return;
		}
		if(!is_regs && S_ISDIR(stat_buffer.st_mode)) {
			scan_dir(path_list, f);
			break;
		} else {
			if(!is_regs) is_regs = 1;
			strcpy(path_info_buffer.path, f);
			path_info_buffer.size = stat_buffer.st_size;
			path_info_buffer.mode = stat_buffer.st_mode;
			AL_list_insert_before(path_list, NULL, &path_info_buffer);
		}
	}

	printf("%d files are to be merged into %s\n", AL_list_count(path_list), output_file);

	int output_fd = creat(output_file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	
	struct merged_file_info mfi;
	mfi.count = AL_list_count(path_list);
	memset(&mfi.padding, 0, PADDING_SIZE);

	if(write(output_fd, (void*)&mfi, sizeof(mfi)) == -1) {
		printf("%s: %s\n", output_file, strerror(errno));
		goto clean_out;
	}

	AL_ListItem *item = AL_list_head(path_list);
	
	struct path_info *p_path_info;
	struct file_info file_info_buffer;

	while(item != NULL) {
		char *file_name;
		memset(&file_info_buffer, 0, sizeof(file_info_buffer));

		p_path_info = (struct path_info*) item->data;
		get_file_name(p_path_info->path, &file_name);
		
		strcpy(file_info_buffer.name, file_name);
		file_info_buffer.mode = p_path_info->mode;
		file_info_buffer.size = p_path_info->size;
		
		if(write(output_fd, (void*)&file_info_buffer, sizeof(file_info_buffer)) == -1) {
			printf("%s: %s\n", output_file, strerror(errno));
		}

		item = item->next;
	}

	item = AL_list_head(path_list);

	int i = 1;

	while(item != NULL) {
		p_path_info = (struct path_info*) item->data;

		int fd = open(p_path_info->path, O_SYNC, O_RDONLY);
		write_file(fd, output_fd);
		close(fd);

		printf("%d/%d\t %s ...done\n", i, AL_list_count(path_list), p_path_info->path);

		item = item->next;
		i++;
	}

clean_out:

	close(output_fd);

	AL_list_destroy(path_list);
	free(path_list);
}

void split(char *merged_file, char *output_dir) {
	if(output_dir[strlen(output_dir) - 1] == '/') output_dir[strlen(output_dir) - 1] = '\0';

	struct stat stat_buffer;

	if(stat(merged_file, &stat_buffer) == -1) {
		printf("%s: %s\n", merged_file, strerror(errno));
		return;
	}

	struct merged_file_info mfi;
	int mfd = open(merged_file, O_SYNC, O_RDONLY);
	if(read(mfd, (void*)&mfi, sizeof(mfi)) == -1) {
		printf("%s: %s\n", merged_file, strerror(errno));
		return;
	}

	for(int i = 0; i < PADDING_SIZE; i++) {
		if(mfi.padding[i] != 0) {
			printf("%s: %s\n", merged_file, "is not a merged file.");
			return;
		}
	}
	
	if(stat(output_dir, &stat_buffer) == -1) {
		mkdir(output_dir, 0775);
		if(stat(output_dir, &stat_buffer) == -1) {
			printf("%s: %s\n", output_dir, strerror(errno));
		}
	}

	if(!S_ISDIR(stat_buffer.st_mode)) {
		printf("%s: %s\n", output_dir, "not a directory");
		return;
	}

	AL_Queue *queue = (AL_Queue*) malloc(sizeof(AL_Queue));
	AL_queue_create(queue, sizeof(struct file_info), NULL);

	struct file_info fi;
	for(int i = 0; i < mfi.count; i++) {
		if(read(mfd, (void*)&fi, sizeof(fi)) == -1) {
			printf("%s: %s\n", merged_file, strerror(errno));
			return;
		}

		AL_queue_enqueue(queue, (void*)&fi);
	}

	printf("%d files to be splitted into '%s' directory\n", mfi.count, output_dir);

	split_files(mfd, queue, output_dir);	

	AL_queue_destroy(queue);
	free(queue);
}
