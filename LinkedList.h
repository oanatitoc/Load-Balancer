/* Copyright 2023 <Alexandra Titoc> */
#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_


typedef struct ll_node_t
{
    void* data;
    struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
} linked_list_t;

/* Functie care creeaza lista (aloca memorie pentru lista) */
linked_list_t *ll_create(unsigned int data_size);

/* Functie care adauga un nod pe pozitia n in lista */
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* data);

/* Functie care elimina nodul de pe pozitia n din lista */
ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n);

/* Functie care returneaza nodul de pe pozitia n din lista */
ll_node_t *ll_get_nth_node(linked_list_t* list, unsigned int n);

/* Functia returneaza dimensiunea listei */
unsigned int ll_get_size(linked_list_t* list);

/* Functia elibereaza intreaga memorie a listei */
void ll_free(linked_list_t** pp_list);

#endif  // LINKEDLIST_H_
