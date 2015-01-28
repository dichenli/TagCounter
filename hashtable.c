#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "hashtable.h"

/*
 * For the given string refered to by the pointer "string", 
 * calculate the hashcode and update the variable pointed to by "value".
 * 
 * Return 1 if successful, and 0 if an error occurred 
 * (e.g. if the string is null).
 */
int hash(char* string, unsigned long* value)
{
	unsigned long hash_code = 0;
	unsigned int len = 0;
	unsigned int i;
	
	//illegal arguments
	if(string == NULL || value == NULL) {
		fprintf(stderr, "Illegal arguments to hash()\n");
		return 0;//exception
	}
	
	len = strlen(string);
	//special cases
	if(len == 0) { //empty string, assign hash code as 31
		*value = 31;
		return 1;
	}
	
	for(i = 0; i < len; i++) {
		hash_code += ((int) pow(31, i)) * ((int) string[i]);
	}
	*value = hash_code;
	return 1;
}

/*
 * find the correct bucket by calculating hashcode of string
 */
node** find_bucket(char* string, hashtable* h, node** bucket_ptr) {
	unsigned long* hash_code = NULL;
	hash_code = malloc(sizeof(unsigned long));
	if(hash_code == NULL) {
		fprintf(stderr, "dynamic memory allocation for hash code error\n");
		return NULL;
	}
	if(hash(string, hash_code) != 1) {
		free(hash_code);
		return NULL; //error from calling hash()
	}
	
	//finally get valid hash code, assign bucket address
	bucket_ptr = &(h->list[(*hash_code) % CAPACITY]);
	free(hash_code);
	return bucket_ptr;
}

/* 
 * return pointer to a node if a given linked list (the bucket) 
 * has the given string, NULL if not
 */
node* find_node(node* head, char* string) {
	if(string == NULL) {
		fprintf(stderr, "Illegal arguments to find_node()\n");
		return NULL;
	}
	while(head != NULL) {
		if(strcmp(head->value, string) == 0) {
			return head;
		}
		head = head->next;
	}
	return NULL;
}

/* given a string and the hashtable used, return a pointer to the node found
*/
node* get_node(char* string, hashtable* h) {
	node** bucket_ptr = NULL;
	if(h == NULL || string == NULL) {
		fprintf(stderr, "Illegal arguments to get_node()\n");
		return NULL;
	}
	
	//find the bucket
	bucket_ptr = find_bucket(string, h, bucket_ptr);
	if(*bucket_ptr == NULL) { //bucket empty, so string not in hashatable
		// fprintf(stderr, "Node not found!\n");
		return NULL;
	}

	return find_node(*bucket_ptr, string);
}

/*
 * Create a new node with the string as the value, return the pointer to the new node. Return null if any error
 */
node* create_node(char* string, unsigned int count, hashtable* h) {
	// node* temp;

	if(string == NULL) {
		fprintf(stderr, "Illegal arguments to find_node()\n");
		return NULL;
	}
	
	node* new_node = NULL;
	new_node = malloc(sizeof(node));
	if(new_node == NULL) {
		fprintf(stderr, "dynamic memory allocation for new node error\n");
		return NULL;
	}
	(new_node->value) = malloc(strlen(string) + 1);
	if(new_node->value == NULL) {
		fprintf(stderr, "dynamic memory allocation for new_node string error\n");
		free(new_node);
		return NULL;
	}
	(new_node->count) = count;
	strcpy(new_node->value, string);
	
	//create double linked list
	// temp = h->tail;
	new_node->next_list = NULL;
	new_node->prev_list = h->tail;
	if(h->tail != NULL) { //the hashtable is not empty
		h->tail->next_list = new_node;
	} else { // the hashtable is empty
		h->head = new_node;
	}
	h->tail = new_node;
	return new_node;
}

/* given a string and hashtable, find the node with the string, 
 * delete the node from the hashtable
 */
int remove_node(char* string, hashtable* h) {
	if(string == NULL || h == NULL) {
		fprintf(stderr, "Illegal arguments for remove()\n");
	}

	node* node_removed = get_node(string, h);
	if(node_removed == NULL) {
		fprintf(stderr, "The string %s is not in the hashtable!\n", string);
		return 0;
	}

	if(h->head == node_removed) {
		h->head = node_removed->next_list;
	}
	if(h->tail == node_removed) {
		h->tail = node_removed->prev_list;
	}
	if(node_removed->prev_list != NULL) {
		node_removed->prev_list->next_list = node_removed->next_list;
	}
	if(node_removed->next_list != NULL) {
		node_removed->next_list->prev_list = node_removed->prev_list;
	}

	free(node_removed->value);
	free(node_removed);
	return 1;
}

/*
 * Add the string to the hashtable in the appropriate "bucket".
 * 
 * Return the count of node after new node inserted. Return 0 if failed
 */
int put(char* string, hashtable* h)
{
	node** bucket_ptr = NULL;
	node* new_node = NULL;

	if(h == NULL || string == NULL) {
		fprintf(stderr, "Illegal arguments to put()\n");
		return 0;
	}

	//find the correct bucket to work on
	bucket_ptr = find_bucket(string, h, bucket_ptr);

	//check if the string is already in the hashtable
	new_node = find_node(*bucket_ptr, string);
	if(new_node != NULL) {
		return (new_node->count) += 1;
	}
	
	//create new node
	new_node = NULL;
	new_node = create_node(string, 1, h);
	if(new_node == NULL) {
		return 0;
	}
	new_node->next = *bucket_ptr;
	*bucket_ptr = new_node;
	return 1;
}


/*
 * return the number of counts of a tag in hashtable. 0 if not found
 */
int get(char* string, hashtable* h)
{
	node* node_found = get_node(string, h);
	if(node_found == NULL) {
		return 0;
	}
	return node_found->count;
}

/*
 * create a new hashtable
 */
hashtable* initiate() {
	int i;
	hashtable* h = malloc(sizeof(hashtable));
	if(h == NULL) {
		fprintf(stderr, "dynamic memory allocation for hashtable error\n");
		return NULL;
	}
	for(i = 0; i < CAPACITY; i++) {
		h->list[i] = NULL;
	}
	h->head = NULL;
	h->tail = NULL;
	return h;
}

/*
 * free all memory of a created hashtable
 */
int free_all(hashtable* h) {
	int i;
	node* temp;
	for(i = 0; i < CAPACITY; i++) {
		for(temp = h->list[i]; temp != NULL; temp = h->list[i]) {
			h->list[i] = temp->next;
			free(temp->value);
			free(temp);
		}
	}
	
	free(h);
	return 0;
}

