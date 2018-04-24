#include "processus.h"
#include "queue.h" // Librairie personnelle : file FIFO de type passenger

/*
 * Lit un passager dans le fichier passe en parametre et retourne ce passager
 */
passenger* read_passenger(FILE *file)
{
    // Alloue un espace pour un passager
    passenger* passenger1 = malloc(sizeof(passenger));
    if(passenger1 == NULL)
    {
        printf("Function read_passenger : Impossible d'allouer un espace pour un passager.\n");
        exit(EXIT_FAILURE);
    }

    // Lecture des differents parametres du passager
    fscanf(file, "#%u %hhu %hhu %u %hhu %u",
           &passenger1->identification_number,
           &passenger1->station_start,
           &passenger1->station_end,
           &passenger1->wait_time_past,
           &passenger1->transfert,
           &passenger1->wait_time_maximum);

    return passenger1; // Renvoie le passager
}

int main(int argc, char* argv[])
{
    EXEMEPLE
    return EXIT_SUCCESS;
}