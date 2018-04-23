#ifndef PROJET_INFO_SE_PROCESSUS_H
#define PROJET_INFO_SE_PROCESSUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_STATION_BUS 5 // Nombre de station maximal de bus
#define MAX_STATION_SUBWAY 3 // Nombre de station maximal de metro
#define MAX_TAXI 3 // Nombre maximale de taxi
#define MAX_CAPACITY_BUS 5 // Capacite maximale d'un bus
#define MAX_CAPACITY_SUBWAY 8 // Capacite maximale d'un train
#define MAX_CAPACITY_TAXI 1 // Capacite maximale d'un taxi

/*
 * Les passagers sont representes par une structure contenant les infomations suivantes
 */
typedef struct{
    uint32_t identification_number; // Numero d'identification unique
    uint8_t station_start; // Station de depart
    uint8_t station_end; // Station d'arrivee
    uint32_t wait_time_past; // Temps d'attente ecoule
    bool transfert; // Transfert entre le circuit de metro et d'autobus est requis
    uint32_t wait_time_maximum; // Temps d'attente maximal
}passenger;


#endif
