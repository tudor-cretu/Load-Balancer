/* Copyright 2023 <Cretu Mihnea Tudor> */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_SIZE	256
#define HMAX 10

typedef struct ll_node_t {
  void *data;
  struct ll_node_t *next;
} ll_node_t;

typedef struct linked_list_t {
  ll_node_t *head;
  unsigned int data_size;
  unsigned int size;
} linked_list_t;

typedef struct info info;
struct info {
  void *key;
  void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
  linked_list_t **buckets; /* Array de liste simplu-inlantuite. */
  /* Nr. total de noduri existente curent in toate bucket-urile. */
  unsigned int size;
  unsigned int hmax; /* Nr. de bucket-uri. */
  /* (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor.
   */
  unsigned int (*hash_function)(void *);
  /* (Pointer la) Functie pentru a compara doua chei. */
  int (*compare_function)(void *, void *);
  /* (Pointer la) Functie pentru a elibera memoria ocupata de cheie si
   * valoare. */
  void (*key_val_free_function)(void *);
};

linked_list_t *ll_create(unsigned int data_size);

/*
  * Based on the data sent through the new_data pointer, a new node is created
  * and added at position n of the list represented by the list pointer. Positions
  * in the list are indexed starting at 0 (i.e. the first node in the list is at
  * position n=0). If n >= no_of_nodes (number of nodes), the new node is added at the
  * end of the list. If n < 0, an error occurs.
*/
void ll_add_nth_node(linked_list_t *list, unsigned int n, const void *new_data);

/*
  * Remove the node at position n from the list whose pointer is sent as a parameter.
  * Positions in the list are indexed starting at 0 (i.e. the first node in the list
  * is at position n=0). If n >= no_of_nodes - 1, the node at theend of the list is removed. 
  * If n < 0, an error occurs. The function returns a pointerto the node that was just removed 
  * from the list. It is the responsibility of the caller to free the memory of this node.
*/
ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n);

/*
 * Function returns the number of nodes from the list
*/
unsigned int ll_get_size(linked_list_t *list);

/*
  * The procedure frees the memory used by all the nodes in the list, and finally,
  * frees the memory used by the list structure itself, and updates the value of
  * the pointer passed as argument to NULL (the argument is a pointer to a pointer).
*/
void ll_free(linked_list_t **pp_list);

/*
 * Functions for comparing keys:
*/
int compare_function_ints(void *a, void *b);
int compare_function_strings(void *a, void *b);

/*
 * Hashing functions:
*/
unsigned int hash_function_int(void *a);
unsigned int hash_function_string(void *a);

/*
  * This function is called to free the memory occupied by the key and value
  * of a pair in the hashtable. If the key or value contains complex data types, 
  * make sure to free the memory considering this aspect.
*/
void key_val_free_function(void *data);

/*
  * Function called after allocating a hashtable in order to initialize it.
  * Linked lists also need to be allocated and initialized.
*/
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void *),
                       int (*compare_function)(void *, void *),
                       void (*key_val_free_function)(void *));

/*
 * Function which returns: 
 * 1, if for the key given has been found its respective value
 * 0, otherwise
*/                       
int ht_has_key(hashtable_t *ht, void *key);

/*
  * Function which returns the value associated with the key given as argument.
  * If the key is not found, the function returns NULL.
*/
void *ht_get(hashtable_t *ht, void *key);

/*
  * Adding a new key-value pair to the hashtable.
  * If the key already exists, the value is replaced with the new one.
  * If the key does not exist, a new pair is created.
  * If the hashtable is full, it is resized.
*/
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

/*
  * Function which removes the entry associated with the key given as argument
*/
void ht_remove_entry(hashtable_t *ht, void *key);

/*
  * A procedure that frees the memory used by all the entries in the hashtable,
  * and then frees the memory used to store the hashtable structure.
*/
void ht_free(hashtable_t *ht);

unsigned int ht_get_size(hashtable_t *ht);

#endif /* HASHTABLE_H_ */
