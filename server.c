/* Copyright 2023 <Cretu Mihnea Tudor> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

server_memory *init_server_memory()
{
	/* allocating the memory for the server */
	server_memory *sm = (server_memory *)malloc(sizeof(server_memory));
    DIE(!sm, "malloc server_memory failed");

    /* creating the hashtable for the server */
	sm->ht = ht_create(HMAX, hash_function_string,
                compare_function_strings, key_val_free_function);

    return sm;
}

void server_store(server_memory *server, char *key, char *value) {
	if (!key || !value)
		return;

	/* storing a key-value pair in the server's hashtable */
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	char *value = NULL;
	if(!ht_get(server->ht, key))
		return NULL;
	else
	{
		/* retrieveing the value associated with the key given from a server*/
		value = ((char*)ht_get(server->ht, key));
		return value;
	}
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server->ht, key);
}

void free_server_memory(server_memory *server) {
	ht_free(server->ht);
	free(server);
}

int label_formula(int server_id, int pos)
{
	return server_id + pos * 100000;
}
