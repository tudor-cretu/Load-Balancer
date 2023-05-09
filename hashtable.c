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

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
 * adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
 * lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
 * pozitia n=0). Daca n >= nr_noduri, noul nod se adauga la finalul listei. Daca
 * n < 0, eroare.
 */
void ll_add_nth_node(linked_list_t *list, unsigned int n,
                     const void *new_data) {
  ll_node_t *prev, *curr;
  ll_node_t *new_node;

  if (!list) {
    return;
  }

  /* n >= list->size inseamna adaugarea unui nou nod la finalul listei. */
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
    /* Adica n == 0. */
    list->head = new_node;
  } else {
    prev->next = new_node;
  }

  list->size++;
}

/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca
 * parametru. Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din
 * lista se afla pe pozitia n=0). Daca n >= nr_noduri - 1, se elimina nodul de
 * la finalul listei. Daca n < 0, eroare. Functia intoarce un pointer spre acest
 * nod proaspat eliminat din lista. Este responsabilitatea apelantului sa
 * elibereze memoria acestui nod.
 */
ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n) {
  ll_node_t *prev, *curr;

  if (!list || !list->head) {
    return NULL;
  }

  /* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei.
   */
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
    /* Adica n == 0. */
    list->head = curr->next;
  } else {
    prev->next = curr->next;
  }

  list->size--;

  return curr;
}

/*
 * Functia intoarce numarul de noduri din lista al carei pointer este trimis ca
 * parametru.
 */
unsigned int ll_get_size(linked_list_t *list) {
  if (!list) {
    return -1;
  }

  return list->size;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista si actualizeaza la
 * NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
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

/*
 * Functii de comparare a cheilor:
 */
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

/*
 * Functii de hashing:
 */
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

/*
 * Functie apelata pentru a elibera memoria ocupata de cheia si valoarea unei
 * perechi din hashtable. Daca cheia sau valoarea contin tipuri de date complexe
 * aveti grija sa eliberati memoria luand in considerare acest aspect.
 */
void key_val_free_function(void *data) {
  info *data_info = (info *)data;
  free(data_info->key);
  free(data_info->value);
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
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

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable
 * folosind functia put 0, altfel.
 */
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

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune
 * tipul ei), in momentul in care se creeaza o noua intrare in hashtable (in
 * cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie a
 * valorii la care pointeaza key si adresa acestei copii trebuie salvata in
 * structura info asociata intrarii din ht. Pentru a sti cati octeti trebuie
 * alocati si copiati, folositi parametrul key_size_bytes.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel
 * put(ht, key_actual, value_actual), valoarea la care pointeaza key_actual
 * poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia
 * unei intrari din hashtable. Nu ne dorim acest lucru, fiindca exista riscul sa
 * ajungem in situatia in care nu mai stim la ce cheie este inregistrata o
 * anumita valoare.
 */
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

  ll_add_nth_node(
      ht->buckets[hash_index], 0,
      data_info);  // adaugam fiecare nod nou pe prima pozitie a listei
  ht->size++;

  free(data_info);
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o
 * intrare din hashtable (adica memoria pentru copia lui key --vezi observatia
 * de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
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
      free(data_info);  // eliberam memoria la care pointeaza
                        // membrul data din nod

      ll_node_t *deleted_node =
          ll_remove_nth_node(ht->buckets[hash_index],
                             node_nr);  // captam nodul care trebuie eliberat
      free(deleted_node);               // eliberam memoria la care pointeaza
                                        // nodul ce a fost exclus din lista

      ht->size--;
      return;
    }

    node = node->next;
    node_nr++;
  }
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable,
 * dupa care elibereaza si memoria folosita pentru a stoca structura hashtable.
 */
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

unsigned int ht_get_size(hashtable_t *ht) {
  if (ht == NULL) return 0;

  return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht) {
  if (ht == NULL) return 0;

  return ht->hmax;
}