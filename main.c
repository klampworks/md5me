#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>
#include <alloca.h>
#include <openssl/md5.h>

int is_hex(const char input) {

	//30 - 39 0-9
	for (char i = 0x30; i < 0x40; i++)
		if (input == i) return 1;

	//61 - 66 a-f
	for (char i = 0x61; i < 0x67; i++)
		if (input == i) return 1;

	//41 - 46 A-F
	for (char i = 0x41; i < 0x47; i++)
		if (input == i) return 1;

	return 0;
}

int is_md5(const char *input) {

	int i = 0;
	for (; input[i]; i++) {

		//Check that this digit is [0-9a-fA-F]
		if (is_hex(input[i]))
			continue;

		//Exclude the dot extension from the equation.
		if (input[i] == '.')
			break;

		//This character was not a hex digit or a dot, not an md5.
		return 0;
	}

	//The individual string characters are legit but are there the right number of them?
	return i == 32;	
}

//Return value is on the heap.
char* md5(const char *filename) {

	//Open the input file.
	FILE *in_file = fopen(filename, "rb");

	if (!in_file) {
		printf ("%s can't be opened.\n", filename);
		return NULL;
	}

	//Initialise the md5 function.
	MD5_CTX md_context;
	MD5_Init (&md_context);
	
	//Read in the data from the file and feed it to the md5 function.
	int bytes;
	unsigned char data[1024];

	while ((bytes = fread(data, 1, 1024, in_file)) != 0) {
		MD5_Update(&md_context, data, bytes);
	}

	//Recieve the final md5 value inside c.
	unsigned char c[MD5_DIGEST_LENGTH];
	MD5_Final(c,&md_context);

	//Allocate memory for the return value.
	char tmp[2];
	char *hash = malloc((MD5_DIGEST_LENGTH * 2) + 1);

	//Format the md5 digits as chars.
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(tmp, "%02x", c[i]);
		memcpy(hash+(i*2), tmp, 2);
	}

	hash[MD5_DIGEST_LENGTH*2] = '\0';
	fclose (in_file);

	return hash;
}

void process_file(const char *filepath, int s) {

	const char *filename = strrchr(filepath, '/');
	int path_len;

	if (!filename) {
		//No forward slashes in this string, the path is the name.
		filename = filepath;
		path_len = 0;
	} else {

		filename++;
		path_len = filename - filepath;
	}

	if (!s && is_md5(filename)) {
		printf("Skipping %s (Use -s)\n", filename);
		return;
	}

	char *hash = md5(filepath);

	if (!hash)
		//Bad things.
		return;


	char *ext = strrchr(filename, '.');
	int ext_len = ext? strlen(ext): 0;

	int size = (path_len + 32 + ext_len + 1);

	char *new_filepath = alloca(size);

	if (path_len)
		memcpy(new_filepath, filepath, path_len);

	memcpy(new_filepath + path_len, hash, 32);

	if (ext)
		memcpy(new_filepath + path_len + 32, ext, ext_len);

	new_filepath[size-1] = '\0';

	rename(filepath, new_filepath);
	printf("Renaming %s to %s\n", filepath, new_filepath);

	free(hash);
}

void file_loop(const char *filename, int r, int s) {

	if (filename[0] == '-' || filename[0] == '.') {
		return;
	}

	struct stat buf;
	stat(filename, &buf);

	if (r && S_ISDIR(buf.st_mode)) {

		DIR *dir;
		struct dirent *ent;

		int path_size = strlen(filename) + 2;
		int size = path_size + 20;
		char *path = malloc(size);
		
		//Already checked length of filename.
		strcpy(path, filename);
		path[path_size - 2] = '/';
		path[path_size - 1] = '\0';

		if ((dir = opendir(path))) {

			//For each file ent.
			while ((ent = readdir(dir)) != NULL) {

				if (ent->d_name[0] == '.')
					continue;

				if (strlen(ent->d_name) + path_size >= size) {
					
					size = strlen(ent->d_name) + path_size;
					path = realloc(path, size);

				}

				//Already checked length of ent->d_name.
				memcpy(path+path_size - 1, ent->d_name, strlen(ent->d_name) + 1);
				file_loop(path, r, s);
			}
		} else {
			printf("Could not open dir %s\n", filename);
		}
		
		free(path);
		closedir(dir);

	} else if(S_ISREG(buf.st_mode)) {
		process_file(filename, s);
	}
}

void main(int argc, char **argv) {

	if (argc < 2)
		return;
		//Not enough arguments.

	// -r recusive
	// -l lazy
	// -s strict

	int r = 0, s = 0;
	for (int i = 1; i < argc; i++) {

		if (strlen(argv[i]) == 2 && argv[i][0] == '-') {

			switch (argv[i][1]) {

			case 'r':
				r = 1;
				break;
			case 'l':
				s = 0;
				break;

			case 's':
				s = 1;

			}
		}
	}

	for (int i = 1; i < argc; i++) {
		file_loop(argv[i], r, s);
	}
}

