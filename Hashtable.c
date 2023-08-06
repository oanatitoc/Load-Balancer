/* Copyright 2023 <Alexandra Titoc> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int hash_function_string(void *a)
{
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

void key_val_free_function(void *data) {
	free(((info*)data)->key);
	free(((info*)data)->value);
    free(data);
}

hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
          int (*compare_function)(void*, void*))
{
	hashtable_t *ht = malloc(sizeof(*ht));
    DIE(ht == NULL, "malloc failed");

    ht->buckets = malloc(hmax *sizeof(*ht->buckets));
    DIE(ht->buckets == NULL, "malloc failed");

    ht->size = 0;

    ht->hmax = hmax;

	ht->hash_function = hash_function;
	ht->compare_function = compare_function;

    for (unsigned int i = 0; i < hmax; i++) {
        ht->buckets[i] = ll_create(sizeof(info));
    }
    return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
    /* indexul la care am putea gasi key-ul */
	int index = ht->hash_function(key) % ht->hmax;

    /* lista din acel bucket */
    linked_list_t *bucket = ht->buckets[index];

    ll_node_t *curr = bucket->head;

	for (unsigned int i = 0; i < bucket->size; i++) {
        if (ht->compare_function(key, ((info *)curr->data)->key) == 0)
            return 1;
        curr = curr->next;
    }

	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	/* indexul la care am putea gasi key-ul */
	int index = ht->hash_function(key) % ht->hmax;

    /* lista din acel bucket */
    linked_list_t *bucket = ht->buckets[index];

    ll_node_t *curr = bucket->head;

	for (unsigned int i = 0; i < bucket->size; i++) {
        if (ht->compare_function(key, ((info *)curr->data)->key) == 0)
            return ((info *)curr->data)->value;
        curr = curr->next;
    }

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	        void *value, unsigned int value_size)
{
	ht->size++;
	info data;
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[index];
	int pos = 0;
	ll_node_t *curr = bucket->head;

    while (curr != NULL && ht->compare_function(key,
		   ((info *)curr->data)->key) != 0) {
        curr = curr->next;
        pos++;
    }

	if (curr == NULL) {
		data.key = malloc(key_size);
		DIE(NULL == data.key, "malloc failed");
		memcpy(data.key, key, key_size);

		data.value = malloc(value_size);
		DIE(NULL == data.value, "malloc failed");
		memcpy(data.value, value, value_size);

		ll_add_nth_node(bucket, pos, &data);

	} else {
		memcpy(((info *)curr->data)->value, value, value_size);
	}
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *curr = ht->buckets[index]->head;
	unsigned int count = 0;

	while (curr) {
		if (ht->compare_function(((info *)curr->data)->key, key) == 0) {
			ll_node_t *remove = ll_remove_nth_node(ht->buckets[index], count);
			key_val_free_function(remove->data);
			free(remove);

			ht->size--;
			return;
		}

		curr = curr->next;
		count++;
	}
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}

void ht_free(hashtable_t *ht)
{
	ll_node_t *curr;
	for (unsigned int i = 0; i < ht->hmax; i++) {
		if (ht->buckets[i]->head != NULL) {
			curr = ht->buckets[i]->head;
			while (curr != NULL) {
				free(((info *)curr->data)->value);
				free(((info *)curr->data)->key);
				curr = curr->next;
			}
		}
		ll_free(&ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}
