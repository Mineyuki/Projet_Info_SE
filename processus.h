#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h" // Librairie personnelle : file FIFO de type passenger


#define MAX_STATION 8 // Nombre de station
#define MAX_STATION_BUS 5 // Nombre de station maximal de bus
#define MAX_TAXI 3 // Nombre maximale de taxi
#define MAX_CAPACITY_BUS 5 // Capacite maximale d'un bus
#define MAX_CAPACITY_SUBWAY 8 // Capacite maximale d'un train

passenger *read_passenger(FILE *); // Lit un passager dans le fichier passe en parametre et retourne ce passager
void *thread_bus(queue **); // Thread du bus
void *thread_subway(queue **); // Thread du metro
void *thread_check(queue **); // Thread verificateur
void *thread_taxi(void *); // Thread taxi

#endif
