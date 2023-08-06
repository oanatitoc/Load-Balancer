/* Copyright 2023 <> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"
#include "utils.h"
#define LABEL_ID_PARAM 100000

struct load_balancer;
typedef struct load_balancer load_balancer;

/* Structura hash_ring_t contine:
 * id = id-ul fiecarui label(eticheta) de pe hashring
 * hash_address = adresa de hash a fiecarui label de pe hashring
 * Va exista mereu un vector de tip hash_ring_t care stocheaza 3 * nr_servers
 * labeluri.
 */
typedef struct {
    int id;
    unsigned int hash_address;
} hash_ring_t;

/* Structura hash_servers_t stocheaza nr_servers servere in ordinea in care
 * acestea au fost citite din fisier.
 * id = id-ul unui server
 * hash_address = adresa de hash a unui server
 * Va exista mereu un vector de tip hash_servers_t care stocheaza nr_servers
 * servere.
 */
typedef struct {
    int id;
    unsigned int hash_address;
} hash_servers_t;


/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/* Functia binary_search_key este folosita pentru a afla serverele de pe
 * hashring in care trebuie introduse noi obiecte.
 * Functia e folosita atat pentru a adauga un nou obiect in sistem, cat
 * si pentru a remapa obiectele la alte servere.
 */
int binary_search_key(load_balancer *main, char *key,
                  unsigned int (*hash_function)(void*));

/* Functia binary_search_servers e folosita pentru a afla pozitia pe care
 * trebuie pusa o noua eticheta a unui server. 
 */
int binary_search_servers(load_balancer *main, int server_id,
                  unsigned int (*hash_function)(void*), int param);

/* Functie in care se realoca memorie pentru vectorul de servere.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_server_memory_vector(server_memory ***servers, int nr);

/* Functie in care se realoca memorie pentru vectorul de tip hash_ring_t.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_hash_ring(hash_ring_t **hash_ring, int nr);

/* Functie in care se realoca memorie pentru vectorul de tip hash_servers_t.
 * Se foloseste atat la adaugarea cat si la stergerea unui server din sistem.
 */
void resize_hash_servers(hash_servers_t **hash_servers, int nr);

/* Functia returneaza 1 daca cheia de pe serverul de dupa cel nou adaugat
 * trebuie remapata la cel nou si 0 in caz contrar.
 */
int check_remap(char *key, unsigned int add1, unsigned int add2, int pos);


/* Functie ajutatoare pentru loader_add_server */
void add_server_id(load_balancer *main, int id, int ord);
/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

#endif /* LOAD_BALANCER_H_ */
