/* Copyright 2023 <> */
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
    // Alocam memorie pentru structura load_balancer
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
    // Returnam adresa de memorie alocata
    return lb;
}

void loader_add_server(load_balancer *main, int server_id) 
{
    if (main->capacity < main->servers_count + NUMBER_OF_REPLICAS) {
		main->capacity *= 2;
		main->servers = realloc(main->servers,
								  main->capacity * sizeof(server_memory *));
		DIE(!main->servers, "load_add_server(): realloc() failed\n");
	}
    int position = 0; 
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
        else for(int j = 0; j < main->servers_count; j++)
        {
            if(main->servers[j] != NULL && main->servers[j]->hash < server->hash)
            {
                position = j;
                okay = 1;
                break;
            }
            else if (main->servers[j] != NULL && main->servers[j]->id < server->id)
            {
                position = j;
                okay = 1;
                break;
            }
        }
        if (!okay) position = 0;
        server_memory *aux = main->servers[position + 1];
        main->servers[position + 1] = server;
        for(int j = position + 2; j < main->servers_count; j++)
        {
            server_memory *aux2 = main->servers[j];
            main->servers[j] = aux;
            aux = aux2;
        }
    }
    loader_servers_sort(main);
}

void loader_remove_server(load_balancer *main, int server_id) {
    for(int i = 0; i < main->servers_count; i++)
    {   
        if(i == main->servers_count - 1 && main->servers[i]->id == server_id)
            move_objects(main->servers[i], main->servers[0]);
        else if(main->servers[i]->id == server_id)
            move_objects(main->servers[i], main->servers[i+1]);
    }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) 
{    
    unsigned int hash = hash_function_key(key);
    short okay = 0;
    for (int i = 0; i < main->servers_count; i ++)
    {
        if(main->servers[i]->hash > hash)
        {
            *server_id = main->servers[i]->id;
            server_store(main->servers[i], key, value);
            okay = 1;
            break;
        }
    }
    if(!okay)
    {
        *server_id = main->servers[0]->id;
        server_store(main->servers[0], key, value);
    } 
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) 
{   
    server_memory* server = NULL; 
    char *value = NULL;
    short okay = 0;
    unsigned int hash = hash_function_key(key);
    for (int i = 0; i < main->servers_count; i ++)
    {
        if (main->servers[i] != NULL)
        {
            server = main->servers[i];
            if(server->hash > hash)
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
    for(int i = 0; i < main->servers_count; i ++)
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

void move_objects(server_memory *server_from, server_memory *server_to)
{
	short okay = 0;
	for (unsigned int i = 0; i < server_from->ht->hmax; i++)
	{   
        if (server_from->ht->buckets[i]->head != NULL)
        {
            ll_node_t* curr = server_from->ht->buckets[i]->head;
            info* data = curr->data;
            while(curr != NULL)
            {
                if (hash_function_key(data->key) < hash_function_servers(&server_to->label))
                {
                    server_store(server_to, data->key, data->value);
                    server_remove(server_from, data->key);
                    okay = 1;
                }
                if(okay)
                    break;
                curr = curr->next;
            }
        }
	}
}

int loader_search_position(load_balancer *main, server_memory *server)
{
    int left = 0;
    int right = main->servers_count - 1;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (main->servers[mid]->hash < server->hash)
            left = mid + 1;
        else if (main->servers[mid]->hash > server->hash)
            right = mid - 1;
        else if (main->servers[mid]->id < server->id)
            left = mid + 1;
        else 
            right = mid - 1;
    }
    return left;
}

