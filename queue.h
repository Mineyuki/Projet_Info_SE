#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Les passagers sont representes par une structure contenant les infomations suivantes
 */
typedef struct
{
    uint32_t identification_number; // Numero d'identification unique
    uint32_t station_start; // Station de depart
    uint32_t station_end; // Station d'arrivee
    uint32_t wait_time_past; // Temps d'attente ecoule
    uint8_t transfert; // Transfert entre le circuit de metro et d'autobus est requis
    uint32_t wait_time_maximum; // Temps d'attente maximal
}passenger;


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
queue *new_queue(); // Allouer une file
_Bool is_empty(queue *); // Fonction de test sur la file
void push(queue *, passenger*); // Ajout d'elements dans la file
passenger *pop(queue *); // Suppression d'element dans la file
void delete_queue(queue *); // Suppression d'une file
chain *find_chain(queue *, uint64_t); // Trouve un maillon a une position particuliere
uint64_t find_passenger_position(queue *, chain *); // Trouve la position d'un passager selon son numero d'identification unique
passenger *remove_chain(queue *, uint64_t); // Supprime une donnée a une position particuliere

#endif
