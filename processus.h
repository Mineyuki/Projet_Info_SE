#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h" // Librairie personnelle : file FIFO de type passenger


#define MAX_STATION 8 // Nombre de station
#define MAX_STATION_BUS 5 // Nombre de station maximal de bus
#define MAX_STATION_SUBWAY 3 // Nombre de station maximal de metro
#define MAX_TAXI 3 // Nombre maximale de taxi
#define MAX_CAPACITY_BUS 5 // Capacite maximale d'un bus
#define MAX_CAPACITY_SUBWAY 8 // Capacite maximale d'un train
#define MAX_CAPACITY_TAXI 1 // Capacite maximale d'un taxi

/*
 * Les passagers sont representes par une structure contenant les infomations suivantes
 */
typedef struct
{
    uint32_t identification_number; // Numero d'identification unique
    uint8_t station_start; // Station de depart
    uint8_t station_end; // Station d'arrivee
    uint32_t wait_time_past; // Temps d'attente ecoule
    uint8_t transfert; // Transfert entre le circuit de metro et d'autobus est requis
    uint32_t wait_time_maximum; // Temps d'attente maximal
}passenger;

passenger *read_passenger(FILE*); // Lit un passager dans le fichier passe en parametre et retourne ce passager
passenger *remove_chain(queue *, chain **); // Supprime un maillon d'une liste de passager et retourne le passager et le maillon suivant
void *thread_bus(queue**); // Thread du bus

#endif
