#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"
#define BUFFER_SIZE 256
#define TOP 10

/*return 1 if the character is the end of a tag, 0 if not
 */
static const char end_chars[] = {' ', '#', '\n', '\t', '\r', '\0', EOF};
int is_tag_end(char c) {
	int len = strlen(end_chars) + 2; //including 0 and EOF
	int i;
	for(i = 0; i < len; i++) {
		if(end_chars[i] == c) {
			return 1;
		}
	}
	return 0;
}

/* read a word from input stream, use dynamic buffer to avoid buffer overflow.
 * return the tag string
 */
char* read_word(char** buffer_ptr, FILE* file) {
	unsigned int buffer_len = BUFFER_SIZE; //current available buffer size
	unsigned int count, i, len; //count length of input string
	char c;
	char* more_buffer = NULL;
	if(buffer_ptr == NULL || file == NULL) {
		fprintf(stderr, "Invalid arguments for read_word()\n");
		return NULL;
	}
	if((*buffer_ptr) != NULL) {
		free(*buffer_ptr);
		*buffer_ptr = NULL;
	}
	*buffer_ptr = malloc(buffer_len * sizeof(char));
	if(*buffer_ptr == NULL) {
		fprintf(stderr, "dynamic memory allocation for input buffer error\n");
		return NULL;
	}

	//read in char by char, initial 'a' is just to avoid making it end of tag
	for(count = 0, c = fgetc(file); !is_tag_end(c); count++) {
		//insufficient buffer, dynamic buffer reallocation
		if(count >= buffer_len) {
			//reallocate more space for buffer
			buffer_len += BUFFER_SIZE;
			more_buffer = realloc(*buffer_ptr, buffer_len * sizeof(char));
			if(more_buffer == NULL) {
				fprintf(stderr, "Memory reallocation for input buffer error.\n");
				if(*buffer_ptr != NULL) {
					free(*buffer_ptr);
				}
				return NULL;
			}
			*buffer_ptr = more_buffer;
			more_buffer = NULL;
		}

		(*buffer_ptr)[count] = c;
		c = fgetc(file);
	}

	(*buffer_ptr)[count] = '\0';

	len = strlen(*buffer_ptr);
	for(i = 0; i < len; i++) {
		(*buffer_ptr)[i] = tolower((*buffer_ptr)[i]);
	}
	return *buffer_ptr;
}

/* have the TOP 10 rank refreshed for each input tag with count
 * return 1 if update rank successful, 0 if fail
 * maybe a linked list is better...
 */
int update_rank(char** rank, unsigned int* rank_count, 
	            char* tag, unsigned int tag_count) {
	int i, j, end = TOP - 1;
	char* temp;
	if(rank == NULL || rank_count == NULL || tag == NULL) {
		fprintf(stderr, "Invalid arguments to update_rank()\n");
		return 0;
	}

	//find the place to insert new string
	for(i = TOP - 1; i >= 0 && rank_count[i] <= tag_count; i--) {
		if(rank[i] != NULL && strcmp(rank[i], tag) == 0) {
			rank_count[i] = tag_count;
			end = i; //already in the list
		}
	}
	i++;
	//this tag is not in TOP 10
	if(i >= TOP) {
		return 1;
	}

	//this tag is in TOP 10
	if(rank[end] != NULL) { //dump the end
		free(rank[end]);
	}
	//shift pointers
	for(j = end; j > i; j--) {
		rank[j] = rank[j - 1];
		rank_count[j] = rank_count[j - 1];
	}
	rank[i] = malloc(strlen(tag) + 1);
	if(rank[i] == NULL) {
		fprintf(stderr, "dynamic memory allocation for tag string error\n");
		return 0;
	}
	strcpy(rank[i], tag);
	rank_count[i] = tag_count;
	return 1;
}

/* clean allocated space, close file.
 * return 1 if success, 0 if failed
 */
int garbage_collection(hashtable *h, char* tag, char** rank) {
	int i = TOP;
	if(h != NULL) {
		free_all(h);
	}
	if(tag != NULL) {
		free(tag);
	}
	for(i--; i >=0; i--) {
		if(rank[i] != NULL) {
			free(rank[i]);
		}
	}
	return 1;
}

/* print TOP 10
*/
void print_top(char** rank, unsigned int* rank_count) {
	unsigned int i, j, len;
	for(i = 0; i < TOP; i++) {
		if(rank[i] != NULL) {
			printf("#%s: %u\n", rank[i], rank_count[i]);
		} else {
			break;
		}
	}
	return;
}

int find_top(FILE* file) {
	char c;
	char* tag = NULL;
	//hashtable to count tags
	hashtable* ht = NULL;
	unsigned int i, tag_count;
	char* rank[TOP]; //TOP 10 of tags
	unsigned int rank_count[TOP];
	if(file == NULL) {
		fprintf(stderr, "Illegal arguments to find_top.\n");
		return 0;
	}

	//initiate TOP 10 list
	for(i = 0; i < TOP; i++) {
		rank[i] = NULL;
		rank_count[i] = 0;
	}
	//create new hashtable
	ht = initiate();
	if(ht == NULL) {
		return 0;
	}

	//read words from file one by one
	for(c = fgetc(file); !feof(file); c = fgetc(file)) {
		if(c == '#') {
			read_word(&tag, file);
			if(strcmp(tag, "") == 0) { //ignore empty tags
				continue;
			}

			int tag_count = put(tag, ht);
			if(tag_count == 0) {
				fprintf(stderr, "tag #%s insertion to hashtable failed.\n", tag);
				//failed, so garbage collection
				garbage_collection(ht, tag, rank);
				return 0;
			} 

			if(update_rank(rank, rank_count, tag, tag_count) == 0) {
				fprintf(stderr, "update_rank failed\n");
				garbage_collection(ht, tag, rank);
				return 0;
			}
		}
	}

	print_top(rank, rank_count);
	return garbage_collection(ht, tag, rank);
}


int parse_file(char* filename) {
	FILE * file = NULL;
	int status = 0;
	if(filename == NULL) {
		return 0;
	}
	file = fopen(filename, "r");
	if(file == NULL) {
		fprintf(stderr, "Can't open the file.\n");
		return 0;
	}

	//status: whether the program is normal. return 0 if something is wrong
	status = find_top(file);
	if(fclose(file) != 0) {
		fprintf(stderr, "File close error\n");
		status = 0;
	}
	return status;
}

/* helper function, flag == -1 or 1
*/
node* print_get_next(node* node, int flag) {
	if(node == NULL) {
		return NULL;
	}
	if(flag == -1) {
		return node->prev_list;
	} else if (flag == 1) {
		return node->next_list;
	} else {
		fprintf(stderr, "Error: flag has to be -1 or 1\n");
		return NULL;
	}
}

/* print a doublly linked list in the hashtable, either from head or tail
 * flag == 1: print from head;
 * flag == -1: print from tail;
 * other: illegal
 */
int print_double_list(char* start, char* end, hashtable* h, int flag) {
	node* iterator;
	// char start = "", end[4];
	// char head[] = {'h', 'e', 'a', 'd'};
	// char tail[] = {'t', 'a', 'i', 'l'};

	if(h == NULL || start == NULL || end == NULL) {
		fprintf(stderr, "Uninitialized arguments to print_double_list()!\n");
		return 0;
	}

	if(flag == 1) {
		if(h->head == NULL) {
			printf("Empty hashtable!\n");
			return 0;
		}
		iterator = h->head;
	}
	else if(flag == -1) {
		if(h->tail == NULL) {
			printf("Empty hashtable!\n");
			return 0;
		}
		iterator = h->tail;
	} else {
		fprintf(stderr, "Illegal flag!\n");
		return 0;
	}

	printf("%s-> ", start);
	for( ; print_get_next(iterator, flag) != NULL; 
		iterator = print_get_next(iterator, flag)) {
		if(iterator->value == NULL) {
			printf("NULL <=> ");
		} else {
			printf("%s <=> ", iterator->value);
		}
	}
	printf("%s <- %s\n", iterator->value, end);
	return 1;
}

int print_double_list_from_head(hashtable* h) {
	return print_double_list("head", "tail", h, 1);
}

int print_double_list_from_tail(hashtable* h) {
	return print_double_list("tail", "head", h, -1);
}

int main(int argc, char* argv[]) {
	int return_value;
	
	//main program, tag counter
	if(argc != 2 || argv[1] == NULL) {
		fprintf(stderr, "Please specify a file to open.\n");
		return 0;
	} else {
		return_value = parse_file(argv[1]); 
	}

	//extra credit part
	printf("Extra credit:\n");
	hashtable* h = initiate();
	printf("Add one through five\n");
	put("one", h);
	put("two", h);
	put("three", h);
	put("four", h);
	put("five", h);
	print_double_list_from_head(h);
	print_double_list_from_tail(h);
	remove_node("four", h);
	printf("remove four\n");
	print_double_list_from_head(h);
	print_double_list_from_tail(h);
}

