#ifndef QUEUE_H
#define QUEUE_H

#include "processus.h"

/*
 * Declaration du maillon
 */
typedef struct _chain
{
    passenger *data; //
    struct _chain *next;
}chain;

/*
 * Declaration de la file
 */
typedef struct
{
    uint64_t size;
    chain *head;
    chain *tail;
}queue;

chain *new_chain(passenger*); // Allouer un maillon
queue *new_queue(); // Allouer une liste
_Bool is_empty(queue *); // Fonction de test sur la liste
void push(queue *, passenger*); // Ajout d'elements dans la file
passenger *pop(queue *); // Suppression d'element dans la file
chain *find_chain(queue *, uint64_t); // Trouve un maillon a une position particuliere
passenger *remove_position(queue *, uint64_t);// Suppression d'une donnee a une position particuliere

#endif
