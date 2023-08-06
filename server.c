/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

server_memory *init_server_memory()
{
	/* Alocare memorie pentru un server */
	server_memory *server = calloc(1, sizeof(server_memory));
	DIE(!server, "calloc failed\n");

	/* Alocam un nou hashtable - folosesc 100 de bucketuri */
	server->ht = ht_create(MAX_BUCKETS, hash_function_string,
						   compare_function_strings);

	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	/* Se stocheaza un nou element in server prin functia ht_put */
	ht_put(server->ht, key, MAX_KEY_LENGTH, value, MAX_VALUE_LENGTH);
}

char *server_retrieve(server_memory *server, char *key) {
	/* Se returneaza valoarea care are cheia "key" (daca aceasta exista) */
	return (char *)ht_get(server->ht, key);
}

void server_remove(server_memory *server, char *key) {
	/* Se elimina elementul cu cheia key (daca acesta exista) */
	if(ht_has_key(server->ht, key))
		ht_remove_entry(server->ht, key);
}

void free_server_memory(server_memory *server) {
	/* Se elibereaza memoria intregului server */
	ht_free(server->ht);
	free(server);
}
