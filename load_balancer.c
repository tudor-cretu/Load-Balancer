/* Copyright 2023 <Cretu Mihnea Tudor> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

unsigned int hash_function_servers(void *a)
{
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer *init_load_balancer()
{
    /* Allocating memory for the load_balancer structure */
    load_balancer *lb = malloc(sizeof(load_balancer));
    DIE(!lb, "malloc load_balancer failed");
    if (lb)
    {
        lb->servers = malloc(sizeof(server_memory*) * NUMBER_OF_REPLICAS);
        DIE(!lb->servers, "malloc servers failed");

        for (int i = 0; i < NUMBER_OF_REPLICAS; i++)
            lb->servers[i] = NULL;
        lb->capacity = NUMBER_OF_REPLICAS;
        lb->servers_count = 0;
    }

    return lb;
}

void loader_add_server(load_balancer *main, int server_id)
{
    /* reallocating the servers memory in case
     the hashring gets full after the adding*/
    if (main->capacity < main->servers_count + NUMBER_OF_REPLICAS)
    {
		main->capacity *= 2;
		main->servers = realloc(main->servers,
								  main->capacity * sizeof(server_memory *));
		DIE(!main->servers, "load_add_server(): realloc() failed\n");

        memset(main->servers + main->servers_count, 0,
            NUMBER_OF_REPLICAS * sizeof(server_memory *));

        for (int i = main->servers_count + NUMBER_OF_REPLICAS;
            i < main->capacity; i++)
            main->servers[i] = NULL;
	}
    int position = 0;
    /* adding the server and its 2 replicas */
    for (int i = 0; i < NUMBER_OF_REPLICAS; i ++)
    {
        short okay = 0;
        main->servers_count++;
        server_memory *server = init_server_memory();
        server->id = server_id;
        server->label = label_formula(server_id, i);
        server->hash = hash_function_servers(&server->label);

        if (main->servers_count == 1)
        {
            main->servers[0] = server;
            break;
        }
        else
        {
            for (int j = 0; j < main->servers_count; j++)
            {
                if (main->servers[j] != NULL)
                {
                    if (main->servers[j]->hash < server->hash)
                    {
                        position = j;
                        okay = 1;
                        break;
                    }
                    else if (main->servers[j]->id < server->id)
                    {
                        position = j;
                        okay = 1;
                        break;
                    }
                }
            }
        }
        /* inserting the new server to the hash ring */
        if (!okay) position = 0;
        server_memory *aux = main->servers[position + 1];
        main->servers[position + 1] = server;
        for (int j = position + 2; j < main->servers_count; j++)
        {
            server_memory *aux2 = main->servers[j];
            main->servers[j] = aux;
            aux = aux2;
        }
    }

    loader_servers_sort(main);
    /* redirecting the key-value pair of 
    corresponding servers to the new one*/
    for (int j = 0; j < main->servers_count; j++)
    {
        if (main->servers[j] != NULL)
        {
            if (server_id == main->servers[j]->id)
                position = j + 1;
            else if (position == main->servers_count - 1)
                position = 0;
            if (main->servers[position] != NULL)
            {
                unsigned int size = ht_get_size(main->servers[position]->ht);
                char **keys = NULL;
                char **values = NULL;

                keys = get_keys(main->servers[position]);
                values = get_values(main->servers[position]);
                for (unsigned int k = 0; k < size; k++)
                    server_remove(main->servers[position], keys[k]);
                for (unsigned k = 0; k < size; k++)
                {
                    int garbage = -1;
                    loader_store(main, keys[k], values[k], &garbage);
                }

                for (unsigned int k = 0; k < size; k++)
                {
                    free(keys[k]);
                    free(values[k]);
                }
                free(keys);
                free(values);
            }
        }
    }
}

void loader_remove_server(load_balancer *main, int server_id)
{
    int position = 0;
    for (int i = 0; i < main->servers_count; i++)
    {
        /* redirecting the key-value pair of 
        the removed server to the next one*/
        int okay = 0;
        if (i == main->servers_count - 1 && main->servers[i]->id == server_id)
        {
            move_objects_for_remove(main->servers[i], main->servers[0]);
            position = i;
            okay = 1;
        }
        else if (main->servers[i]->id == server_id)
        {
            move_objects_for_remove(main->servers[i], main->servers[i+1]);
            position = i;
            okay = 1;
        }
        if (okay == 1)
        {
            /* removing the server from the hashring */
            free_server_memory(main->servers[position]);
            main->servers[position] = NULL;
            for (int j = position; j < main->servers_count - 1; j++)
                main->servers[j] = main->servers[j+1];
            main->servers_count--;
            i--;
        }
    }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
    unsigned int hash = hash_function_key(key);
    for (int i = 0; i < main->servers_count; i++)
    {
        if (main->servers[i] != NULL)
        {
            if (main->servers[i]->hash >= hash)
            {
                *server_id = main->servers[i]->id;
                server_store(main->servers[i], key, value);
                return;
            }
        }
    }
    *server_id = main->servers[0]->id;
    server_store(main->servers[0], key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
    server_memory* server = NULL;
    char *value = NULL;
    short okay = 0;
    unsigned int hash = hash_function_key(key);
    for (int i = 0; i < main->servers_count; i ++)
    {
        /* retrieves the value associated with
        the given key from the hashring */
        if (main->servers[i] != NULL)
        {
            server = main->servers[i];
            if (server->hash > hash)
            {
                *server_id = server->id;
                value = server_retrieve(server, key);
                okay = 1;
                break;
            }
        }
    }
    if (!okay)
    {
        *server_id = main->servers[0]->id;
        value = server_retrieve(main->servers[0], key);
    }
    return value;
}

void free_load_balancer(load_balancer *main)
{
    for (int i = 0; i < main->servers_count; i ++)
        free_server_memory(main->servers[i]);
    free(main->servers);
    free(main);
}

void loader_servers_sort(load_balancer *main)
{
    for (int i = 0; i < main->servers_count - 1; i ++)
        for (int j = i + 1; j < main->servers_count; j ++)
            if (main->servers[i]->hash > main->servers[j]->hash)
            {
                server_memory *aux = main->servers[i];
                main->servers[i] = main->servers[j];
                main->servers[j] = aux;
            }
}

void move_objects_for_remove(server_memory *server_from,
     server_memory *server_to)
{
    /* 
     * parsing through the hashtable of the source server
     * and stores the key-value pair with the corresponding hash 
     * into the destination server 
    */
    info* data = NULL;
	for (unsigned int i = 0; i < server_from->ht->hmax; i++)
	{
        if (server_from->ht->buckets[i]->head != NULL)
        {
            ll_node_t* curr = server_from->ht->buckets[i]->head;
            while (curr != NULL)
            {
                data = curr->data;
                ht_put(server_to->ht, data->key, strlen(data->key)+1,
                     data->value, strlen(data->value)+1);
                curr = curr->next;
            }
        }
	}
}

char **get_keys(server_memory *server_from)
{
    char **keys = NULL;
    info* data = NULL;
    unsigned int size = ht_get_size(server_from->ht);
    keys = malloc(size * sizeof(char *));
    DIE(!keys, "malloc keys failed");
    int j = 0;

    for (unsigned int i = 0; i < server_from->ht->hmax; i++)
    {
        if (server_from->ht->buckets[i]->head != NULL)
        {
            ll_node_t* curr = server_from->ht->buckets[i]->head;
            while (curr != NULL)
            {
                /* storing all the keys from a given server
                into a vector */
                data = (info *)curr->data;
                keys[j] = malloc(strlen(data->key) + 1);
                strcpy(keys[j], data->key);
                j++;
                curr = curr->next;
            }
        }
    }

    return keys;
}

char **get_values(server_memory *server_from)
{
    char **values = NULL;
    info* data = NULL;
    unsigned int size = ht_get_size(server_from->ht);
    values = malloc(size * sizeof(char *));
    DIE(!values, "malloc values failed");
    int j = 0;

    for (unsigned int i = 0; i < server_from->ht->hmax; i++)
    {
        if (server_from->ht->buckets[i]->head != NULL)
        {
            ll_node_t* curr = server_from->ht->buckets[i]->head;
            while (curr != NULL)
            {
                 /* storing all the values from a given server
                into a vector */
                data = (info *)curr->data;
                values[j] = malloc(strlen(data->value) + 1);
                strcpy(values[j], data->value);
                j++;
                curr = curr->next;
            }
        }
    }

    return values;
}
