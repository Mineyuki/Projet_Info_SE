#include "processus.h"
#include "queue.h" // Librairie personnelle : file FIFO de type passenger

/*
 * Lit un passager dans le fichier passe en parametre et retourne ce passager
 */
passenger *read_passenger(FILE *file)
{
    // Alloue un espace pour un passager
    passenger* passenger1 = malloc(sizeof(passenger));
    if(passenger1 == NULL)
    {
        printf("Function read_passenger : Impossible d'allouer un espace pour un passager.\n");
        exit(EXIT_FAILURE);
    }

    // Lecture des differents parametres du passager
    fscanf(file, "#%u %hhu %hhu %u %hhu %u\n",
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
    FILE *file = fopen(argv[1], "rt"); // On recupere le fichier passe en parametre
    if(file == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier.\n");
        exit(EXIT_FAILURE);
    }

    // Tableau de file d'attente
    queue **increment_table_passenger = malloc(MAX_STATION * sizeof(queue));
    // Tableau de file d'attente du sens contraire horaire du metro
    queue **decrement_table_passenger = malloc(MAX_STATION * sizeof(queue));
    passenger *passenger1;

    for(int index = 0; index < MAX_STATION; index++)
    { // Creation des files FIFO
        increment_table_passenger[index] = new_queue();
        if(index >= MAX_STATION_BUS)
        { // Tableau de files d'attente pour le metro
            decrement_table_passenger[index] = new_queue();
        }
    }

    while(!feof(file))
    { // Tant que l'on n'a pas atteint la fin du fichier
        passenger1 = read_passenger(file); // Recupere un passager

        if(passenger1->station_start > passenger1->station_end
           && passenger1->station_start >= MAX_STATION_BUS)
        { // Ajoute a la file d'attente du sens contraire horaire du metro
            push(decrement_table_passenger[passenger1->station_start], passenger1);
        }
        else
        { // Ajoute a la file d'attente
            push(increment_table_passenger[passenger1->station_start], passenger1);
        }
    }

    fclose(file); // Fermeture du fichier

    for(int index = 0; index < MAX_STATION; index++)
    { // Supprimer des files FIFO
        delete_queue(increment_table_passenger[index]);
        if(index >= MAX_STATION_BUS)
        { // Tableau de files d'attente pour le metro
            delete_queue(decrement_table_passenger[index]);
        }
    }

    free(increment_table_passenger);
    free(decrement_table_passenger);

    return EXIT_SUCCESS;
}