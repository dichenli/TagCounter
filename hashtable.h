// the initial capacity (number of buckets)
#define CAPACITY 100

typedef struct Node node;
struct Node {
  char* value;
  unsigned int count;
  node* next; // next node in the list
  node* prev_list; //a doubly linked list in the order of which nodes are added
  node* next_list;
};

typedef struct Hashtable hashtable;
struct Hashtable {
  node* list[CAPACITY]; // "buckets" of linked lists
  node* head; //head and tail of doubly linked list
  node* tail;
};

/* Function prototypes */
int hash(char*, unsigned long*);
int put(char*, hashtable*);
int get(char*, hashtable*);
hashtable* initiate();
int free_all(hashtable*);
int remove_node(char*, hashtable*);