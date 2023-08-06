/* Copyright 2023 <Alexandra Titoc> */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "LinkedList.h"

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
	/* (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor. */
	unsigned int (*hash_function)(void*);
	/* (Pointer la) Functie pentru a compara doua chei. */
	int (*compare_function)(void*, void*);
};

/* Functia de comparare pentru hashtable */
int compare_function_strings(void *a, void *b);

/* Functia de hash pentru stringuri */
unsigned int hash_function_string(void *a);

/* FUnctie care elibereaza memoria datelor cheie-valoare */
void key_val_free_function(void *data);

/* Functie care creeaza un hashtable cu hmax buckets-uri */
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
          int (*compare_function)(void*, void*));

/* Functie care verifica daca exista o anumita cheie in hashtable */
int ht_has_key(hashtable_t *ht, void *key);

/* Functie care returneaza un element din hashtable in functie de cheie*/
void *ht_get(hashtable_t *ht, void *key);

/* Functie care adauga un element in hashtable */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	        void *value, unsigned int value_size);

/* Functie prin care se elimina un element din hashtable */
void ht_remove_entry(hashtable_t *ht, void *key);

/* Functie care returneaza size-ul hashtable-ului */
unsigned int ht_get_size(hashtable_t *ht);

/* Functia returneaza numarul maxim de buckets-uri ale hashtable-ului */
unsigned int ht_get_hmax(hashtable_t *ht);

/* Functia elibereaza toata memoria hashtable-ului */
void ht_free(hashtable_t *ht);

#endif  // HASHTABLE_H_
