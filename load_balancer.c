/* Copyright 2023 <Alexandra Titoc> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

/* Structura load_balancer contine:
 * nr_servers = nr de servere din loader
 * servers = vector de pointeri la serverele din sistem
 * hash_servers = vector la structura hash_server_t
 * hash_ring = vector de etichete id/adresa
 */
struct load_balancer {
    int nr_servers;
    server_memory **servers;
    hash_ring_t *hash_ring;
    hash_servers_t *hash_servers;
};

/* Functie de hash pentru servere */
unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

/* Functie de hash pentu cheile care trebuie stocate in servere */
unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

/* Functie care initializeaza memorie pentru load balancer si retuneaza
 * un pointer la structura load_balancer
 */
load_balancer *init_load_balancer() {
    /* Se aloca memorie pentru loader */
    load_balancer *loader = malloc(sizeof(load_balancer));
    DIE(!loader, "malloc failed\n");

    /* Se aloca memorie pentru vectorul de pointeri la serverele din sistem */
    loader->servers = malloc(sizeof(server_memory *));
    DIE(!loader->servers, "malloc failed\n");

    /* Se aloca memorie pentru vectorul circular de etichete */
    loader->hash_ring = malloc(sizeof(hash_ring_t));
    DIE(!loader->hash_ring, "malloc failed\n");

    loader->hash_servers = malloc(sizeof(hash_servers_t));
    DIE(!loader->hash_servers, "malloc failed\n");

    /* Se initializeaza numarul de servere existente in sistem cu 0 */
    loader->nr_servers = 0;

    return loader;
}

/* Functia binary_search_key este folosita pentru a afla serverele de pe
 * hashring in care trebuie introduse noi obiecte.
 * Functia e folosita atat pentru a adauga un nou obiect in sistem, cat
 * si pentru a remapa obiectele la alte servere.
 */
int binary_search_key(load_balancer *main, char *key,
                  unsigned int (*hash_function)(void*))
{
    int left =  0, right = 3 * main->nr_servers - 1, mid = 0;

    while (left <= right) {
        mid = left + (right - left) / 2;

        if (main->hash_ring[mid].hash_address >= hash_function(key))
            right = mid - 1;
        else
            left = mid + 1;
    }
    if (left == 3 * main->nr_servers)
        return 0;
    return left;
}

/* Functia binary_search_servers e folosita pentru a afla pozitia pe care
 * trebuie pusa o noua eticheta a unui server. 
 */
int binary_search_servers(load_balancer *main, int server_id,
                  unsigned int (*hash_function)(void*), int param)
{
    int left =  0, right = 3 * main->nr_servers + param - 1, mid;

    while (left < right) {
        mid = left + (right - left) / 2;

        if (main->hash_ring[mid].hash_address >= hash_function(&server_id))
            right = mid;
        else
            left = mid + 1;
    }
    if (left == 3 * main->nr_servers + param)
        return 0;
    if (left == 3 * main->nr_servers + param - 1 &&
       main->hash_ring[left].hash_address > hash_function(&server_id))
        return left;
    if (left == 3 * main->nr_servers + param - 1)
        return left + 1;

    return left;
}

/* Functie in care se realoca memorie pentru vectorul de servere.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_server_memory_vector(server_memory ***servers, int nr)
{
    *servers = realloc(*servers, nr * sizeof(server_memory*));
    DIE(!*servers, "malloc failed\n");
}

/* Functie in care se realoca memorie pentru vectorul de tip hash_ring_t.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_hash_ring(hash_ring_t **hash_ring, int nr)
{
    *hash_ring = realloc(*hash_ring, nr * sizeof(hashtable_t));
    DIE(!*hash_ring, "malloc failed\n");
}

/* Functie in care se realoca memorie pentru vectorul de tip hash_servers_t.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_hash_servers(hash_servers_t **hash_servers, int nr)
{
    *hash_servers = realloc(*hash_servers, nr * sizeof(hash_servers_t));
    DIE(!*hash_servers, "malloc failed\n");
}

/* Functia returneaza 1 daca cheia de pe serverul de dupa cel nou adaugat
 * trebuie remapata la cel nou si 0 in caz contrar.
 */
int check_remap(char *key, unsigned int add1, unsigned int add2, int pos)
{
    /* add1 e adresa labelului nou adaugat pe hash-ring, add2 e adresa
     * labelului de dinaintea cel nou in sens orar
     * pos e pozitia pe hash_ring a noului label adaugat
     */
    if (pos == 0) {
        if (hash_function_key(key) > add2 || hash_function_key(key) < add1)
            return 1;
    } else {
        if (hash_function_key(key) > add2 && hash_function_key(key) < add1)
            return 1;
    }
    return 0;
}

/* Functia afla pozitia pe care trebuie adaugata o eticheta pe hash_ring
 * si muta eventualele obiecte in noul server adaugat.
 * ord = ordinul etichetei (0, 1, 2)
 */
void add_server_id(load_balancer *main, int id, int ord)
{
    /* Verificam daca se introduce prima eticheta a primului server introdus
     *in sistem */
    if (main->nr_servers == 0 && ord == 0) {
        unsigned int hash_server = hash_function_servers(&id);
        main->hash_ring[0].id = id;
        main->hash_ring[0].hash_address = hash_server;
        return;
    }

    unsigned int hash_server = hash_function_servers(&id);

    /* pos = pozitia pe care se va stoca noul label */
    int pos = binary_search_servers(main, id, hash_function_servers, ord);

    /* Shiftez labelurile de pe hashring pentru a "face loc" noului label */
    for (int i = 3 * main->nr_servers + ord; i > pos; i--)
        main->hash_ring[i] = main->hash_ring[i - 1];
    main->hash_ring[pos].id = id % LABEL_ID_PARAM;
    main->hash_ring[pos].hash_address = hash_server;

    /* Gasesc pozitia serverului de dupa si de dinainte de noul label */
    int pos_next, pos_prev;
    if (pos == 3 * main->nr_servers + ord) {
        pos_next = 0;
        pos_prev = pos - 1;
    } else {
        pos_next = pos + 1;
        if (pos == 0)
            pos_prev = 3 * main->nr_servers + ord;
        else
            pos_prev = pos - 1;
    }

    /* Gasesc pozitia din vectorul de servere a serverului din care este
     * posibil sa trebuiasca sa mutam obiecte-labelul de pe pozitia pos_next) 
     */
    int pos_server_next = -1;
    if (main->hash_ring[pos_next].id % LABEL_ID_PARAM !=
       main->hash_ring[pos].id % LABEL_ID_PARAM) {
        for (int i = 0; i < main->nr_servers; i++) {
            if (main->hash_servers[i].id ==
               main->hash_ring[pos_next].id % LABEL_ID_PARAM) {
                pos_server_next = i;
                break;
            }
        }
    }

    /* Fac verificarea daca obiectele de pe serverul de dupa cel nou trebuie
     * remapate */
    if (pos_server_next != -1) {
        server_memory *s = main->servers[pos_server_next];
        hashtable_t *ht = s->ht;
        for (int i = 0; i < MAX_BUCKETS; i++) {
            ll_node_t *curr = ht->buckets[i]->head;
            while (curr) {
                struct info *info = curr->data;
                char *key = (char *)info->key;
                char *value = (char *)(info->value);

                /* Verific daca trebuie sa mut respectivul obiect pe noul
                 * server adaugat */
                if (check_remap(key, hash_server,
                    main->hash_ring[pos_prev].hash_address, pos)) {
                    server_store(main->servers[main->nr_servers], key, value);
                    curr = curr->next;
                    server_remove(main->servers[pos_server_next], key);
                } else {
                    curr = curr->next;
                }
            }
        }
    }
}

void loader_add_server(load_balancer *main, int server_id)
{
    /* Realoc memorie pentu toti vectorii din sistem pentru a face loc
     * unui nou server */
    resize_server_memory_vector(&main->servers, main->nr_servers + 1);
    resize_hash_servers(&main->hash_servers, main->nr_servers + 1);
    resize_hash_ring(&main->hash_ring, 3 * main->nr_servers + 3);

    /* Aflu id-ul tuturor celor 3 labeluri */
    int id0 = server_id;
    int id1 = LABEL_ID_PARAM + server_id;
    int id2 = 2 * LABEL_ID_PARAM + server_id;

    /* Adaug un nou server pe ultima pozitie */
    main->servers[main->nr_servers] = init_server_memory();
    main->hash_servers[main->nr_servers].id = server_id;
    main->hash_servers[main->nr_servers].hash_address =
    hash_function_servers(&server_id);

    /* adaugare server cu id0 */
    add_server_id(main, id0, 0);

    /* adaugare server cu id1 */
    add_server_id(main, id1, 1);

    /* adaugare server cu id2 */
    add_server_id(main, id2, 2);

    /* Cresc numarul de servere */
    main->nr_servers++;
}

void loader_remove_server(load_balancer *main, int server_id)
{
    /* Verific daca trebuie sters unicul server (in acest caz se sterg
     * definitiv si informatiile din acesta) */
    if (main->nr_servers == 1) {
        free_server_memory(main->servers[0]);
        main->nr_servers = 0;
        return;
    }

    /* Aflu indexul unde este stocat serverul cu id-ul server_id */
    int pos;
    for (int i = 0; i < main->nr_servers; i++)
        if (server_id == (main->hash_servers[i].id)) {
            pos = i;
            break;
        }

    /* Fac shiftarea la stanga a tuturor labelurilor de fiecare data cand
     * gasesc un label al serverului pe care vreau sa il scot */
    int nr = 3 * main->nr_servers;
    for (int i = 0; i < nr; i++) {
        if (main->hash_ring[i].id % LABEL_ID_PARAM == server_id) {
            nr--;
            for (int k = i; k < nr; k++)
                main->hash_ring[k] = main->hash_ring[k + 1];
            i--;
        }
    }

    /* Realoc vectorul cu labeluri de pe hash_ring */
    resize_hash_ring(&main->hash_ring, 3 * main->nr_servers - 3);
    main->nr_servers--;

    /* Remapez toate obiectele (cheie-valoare) din serverul pe care vreau sa il
     * sterg pe celelalte servere de pe hashring */
    server_memory *s = main->servers[pos];
    hashtable_t *ht = s->ht;

    /* Parcurg toate buckets-urile din hashtable-ul asociat serverului */
    for (int i = 0; i < MAX_BUCKETS; i++) {
        ll_node_t *curr;
        curr = ht->buckets[i]->head;
        while (curr) {
            struct info *info = curr->data;
            char *key = (char *)info->key;
            char *value = (char *)(info->value);

            /* Functia loader_store se ocupa atat de gasirea serverului pe care
             * trebuie remapat obiectul prin cautarea binara, cat si de remaparea
             * prorpiu-zisa */
            loader_store(main, key, value, &server_id);

            curr = curr->next;
        }
    }

    /* Dealloc toata memoria serverului */
    free_server_memory(main->servers[pos]);

    /* Elimin serverul din vectorul de servere si cel de hash_servers */
    for (int i = pos; i < main->nr_servers; i++) {
        main->servers[i] = main->servers[i + 1];
        main->hash_servers[i] = main->hash_servers[i + 1];
    }

    /* Realoc memorie pentru cei doi vectori */
    resize_server_memory_vector(&main->servers, main->nr_servers);
    resize_hash_servers(&main->hash_servers, main->nr_servers);
}


void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
    /* Pozitia din hash_ring a serverului unde se va stoca cheia */
    int position = binary_search_key(main, key, hash_function_key);

    /* ID-ul serverului in care se va adauga cheia */
    *server_id = main->hash_ring[position].id % LABEL_ID_PARAM;

    /* Se introduce cheia pe serverul respectiv */
    for (int i = 0; i <= main->nr_servers; i++)
        if (main->hash_servers[i].id == *server_id) {
            server_store(main->servers[i], key, value);
            return;
        }
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    /* Pozitia din hash_ring a serverului unde se afla cheia */
    int position = binary_search_key(main, key, hash_function_key);

    /* ID-ul serverului in care se afla cheia */
    *server_id = main->hash_ring[position].id % LABEL_ID_PARAM;

    /* Se preia cheia de pe serverul respectiv */
    for (int i = 0; i < main->nr_servers; i++)
        if (main->hash_servers[i].id == *server_id)
            return server_retrieve(main->servers[i], key);

    return NULL;
}

void free_load_balancer(load_balancer *main)
{
    for (int i = 0; i < main->nr_servers; i++)
        free_server_memory(main->servers[i]);
    free(main->servers);
    free(main->hash_ring);
    free(main->hash_servers);
    free(main);
}

