#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashtable.h"

int main() {
	hashtable* h1 = initiate();
	assert(h1 != NULL);
	if(h1 == NULL) {
		return 0;
	}
	
	assert(put("dog", h1) == 1);
	assert(put("cat", h1) == 1);
	assert(put("dog", h1) == 2);
	assert(put("", h1) == 1);
	assert(get("", h1) == 1);
	
	hashtable* h2 = initiate();
	assert(h2 != NULL);
	if(h2 == NULL) {
		return 0;
	}
	
	assert(put("dog", h2) == 1);
	assert(get("dog", h2) == 1);
	assert(get("cat", h2) == 0);
	free_all(h1);
	free_all(h2);
}