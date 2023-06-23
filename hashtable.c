/* Copyright 2023 <Cretu Mihnea Tudor> */
#include "hashtable.h"
#include "utils.h"

#define MAX_STRING_SIZE 256
#define HMAX 10

linked_list_t *ll_create(unsigned int data_size) {
  linked_list_t *ll;

  ll = malloc(sizeof(*ll));

  ll->head = NULL;
  ll->data_size = data_size;
  ll->size = 0;

  return ll;
}

void ll_add_nth_node(linked_list_t *list, unsigned int n,
                     const void *new_data) {
  ll_node_t *prev, *curr;
  ll_node_t *new_node;

  if (!list) {
    return;
  }

  /* n >= list->size means adding the new node at the end of the list */
  if (n > list->size) {
    n = list->size;
  }

  curr = list->head;
  prev = NULL;
  while (n > 0) {
    prev = curr;
    curr = curr->next;
    --n;
  }

  new_node = malloc(sizeof(*new_node));
  new_node->data = malloc(list->data_size);
  memcpy(new_node->data, new_data, list->data_size);

  new_node->next = curr;
  if (prev == NULL) {
    /* means n == 0*/
    list->head = new_node;
  } else {
    prev->next = new_node;
  }

  list->size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n) {
  ll_node_t *prev, *curr;

  if (!list || !list->head) {
    return NULL;
  }

  /* n >= list->size - 1 means removing the node from the end of the list */
  if (n > list->size - 1) {
    n = list->size - 1;
  }

  curr = list->head;
  prev = NULL;
  while (n > 0) {
    prev = curr;
    curr = curr->next;
    --n;
  }

  if (prev == NULL) {
    /* n == 0 */
    list->head = curr->next;
  } else {
    prev->next = curr->next;
  }

  list->size--;

  return curr;
}

unsigned int ll_get_size(linked_list_t *list) {
  if (!list) {
    return -1;
  }

  return list->size;
}

void ll_free(linked_list_t **pp_list) {
  ll_node_t *currNode;

  if (!pp_list || !*pp_list) {
    return;
  }

  while (ll_get_size(*pp_list) > 0) {
    currNode = ll_remove_nth_node(*pp_list, 0);
    free(currNode->data);
    currNode->data = NULL;
    free(currNode);
    currNode = NULL;
  }

  free(*pp_list);
  *pp_list = NULL;
}

int compare_function_ints(void *a, void *b) {
  int int_a = *((int *)a);
  int int_b = *((int *)b);

  if (int_a == int_b) {
    return 0;
  } else if (int_a < int_b) {
    return -1;
  } else {
    return 1;
  }
}

int compare_function_strings(void *a, void *b) {
  char *str_a = (char *)a;
  char *str_b = (char *)b;

  return strcmp(str_a, str_b);
}

unsigned int hash_function_int(void *a) {
  /*
   * Credits: https://stackoverflow.com/a/12996028/7883884
   */
  unsigned int uint_a = *((unsigned int *)a);

  uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
  uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
  uint_a = (uint_a >> 16u) ^ uint_a;
  return uint_a;
}

unsigned int hash_function_string(void *a) {
  /*
   * Credits: http://www.cse.yorku.ca/~oz/hash.html
   */
  unsigned char *puchar_a = (unsigned char *)a;
  unsigned long hash = 5381;
  int c;

  while ((c = *puchar_a++))
    hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

  return hash;
}

void key_val_free_function(void *data) {
  info *data_info = (info *)data;
  free(data_info->key);
  free(data_info->value);
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t* ht = malloc(sizeof(*ht));
	DIE(!ht, "malloc failed\n");

	ht->buckets = malloc(sizeof(linked_list_t *) * hmax);
	DIE(!ht->buckets, "malloc failed\n");

	for (unsigned int i = 0; i < hmax; i++)
		ht->buckets[i] = ll_create(sizeof(info));

	ht->size = 0;
	ht->hmax = hmax;

	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;

	return ht;
}

int ht_has_key(hashtable_t *ht, void *key) {
  if (!ht || !key) {
    return -1;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  while (node != NULL) {
    info *data_info = (info *)node->data;
    if (!ht->compare_function(data_info->key, key)) {
      return 1;
    }
    node = node->next;
  }

  return 0;
}

void *ht_get(hashtable_t *ht, void *key) {
  if (!ht || !key || ht_has_key(ht, key) != 1) {
    return NULL;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  while (node != NULL) {
    info *data_info = (info *)node->data;
    if (!ht->compare_function(data_info->key, key)) {
      return data_info->value;
    }
    node = node->next;
  }

  return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
            unsigned int value_size) {
  if (!ht || !key || !value) {
    return;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;

  if (ht_has_key(ht, key) == 1) {
    ll_node_t *node = ht->buckets[hash_index]->head;
    while (node != NULL) {
      info *data_info = node->data;

      if (!ht->compare_function(data_info->key, key)) {
        free(data_info->value);

        data_info->value = malloc(value_size);

        memcpy(data_info->value, value, value_size);
        return;
      }

      node = node->next;
    }
  }

  info *data_info = malloc(sizeof(info));

  data_info->key = malloc(key_size);
  data_info->value = malloc(value_size);

  memcpy(data_info->key, key, key_size);
  memcpy(data_info->value, value, value_size);
  ll_add_nth_node(ht->buckets[hash_index], 0, data_info);
  ht->size++;
  free(data_info);
}

void ht_remove_entry(hashtable_t *ht, void *key) {
  if (!ht || !key || ht_has_key(ht, key) != 1) {
    return;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  unsigned int node_nr = 0;

  while (node != NULL) {
    info *data_info = (info *)node->data;

    if (!ht->compare_function(data_info->key, key)) {
      ht->key_val_free_function(data_info);
      free(data_info);

      ll_node_t *deleted_node =
          ll_remove_nth_node(ht->buckets[hash_index], node_nr);
      free(deleted_node);

      ht->size--;
      return;
    }

    node = node->next;
    node_nr++;
  }
}

void ht_free(hashtable_t *ht)
{
	ll_node_t *aux;
	for (unsigned int i = 0; i < ht->hmax; i++) {
		if (ht->buckets[i]->head) {
			aux = ht->buckets[i]->head;
			while (aux) {
				free((*(info*)aux->data).key);
				free((*(info*)aux->data).value);
				aux = aux->next;
			}
		}
		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}
