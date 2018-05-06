#include "processus.h"

uint32_t number_passenger; // Nombre de passager
queue *arrived_passenger; // Passager arrive

/*
 * Lit un passager dans le fichier passe en parametre et retourne ce passager
 */
passenger *read_passenger(FILE *file)
{
    // Alloue un espace pour un passager
    passenger* passenger1 = malloc(sizeof(passenger));
    if(passenger1 == NULL)
    {
        fprintf(stderr, "Function read_passenger : Impossible d'allouer un espace pour un passager.\n");
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

/*
 * Supprime un maillon d'une liste de passager
 * Retourne le passager et le maillon suivant
 */
passenger *remove_chain(queue *queue1, chain **chain_remove)
{
    chain *chain_temp;
    passenger *passenger1;

    if((*chain_remove) == NULL)
    { // Erreur : on demande a supprimer dans une chaine inexistante
        fprintf(stderr, "Erreur main.c : Impossible de supprimer le passager inexistant\n");
        exit(EXIT_FAILURE);
    }
    else if ((*chain_remove) == queue1->head)
    { // Si le maillon est en tete
        passenger1 = pop(queue1); // Retourne la tete de la liste

        (*chain_remove) = queue1->head; // Recupere la tete de la liste

        return passenger1;
    }
    else
    { // Sinon
        chain_temp = (*chain_remove)->next; // Recupere le maillon suivant du maillon a supprimer
        passenger1 = (*chain_remove)->data; // Recupere le passager

        if(queue1->tail == (*chain_remove))
        { // Si le maillon a supprimer est a la queue de la liste
            queue1->tail = chain_temp; // La nouvelle queue devient le maillon
        }

        free(*chain_remove); // Supprime le maillon a supprimer

        queue1->size -= 1; // Reduit la taille de la liste
        (*chain_remove) = chain_temp;

        return passenger1; // Retourne le maillon suivant
    }
}

/*
 * Fonction thread pour autobus
 */
void *thread_bus(queue **table_passenger)
{
    uint32_t count_station = 0; // Compteur de la station
    chain *chain_bus;
    passenger *passenger_bus;
    queue *bus_passenger_list = new_queue(); // Creation d'une liste de passager

    while(number_passenger > 0)
    {
        // Incremente un compteur de station
        count_station = count_station + 1;
        if(count_station == MAX_STATION_BUS)
        { // Si on atteint le maximum de station, on remet le compteur a 0
            count_station = 0;
        }

        chain_bus = bus_passenger_list->head; // On recupere la tete de la liste de passager du bus

        while(chain_bus != NULL)
        { // Parcours de la liste de passager du bus

            if (chain_bus->data->station_end == count_station)
            { // Verifier dans la liste de passager du bus si un passager est arrive a destination
                printf("[bus] : [debarque] le passager %u\n", chain_bus->data->identification_number);

                // Il assure le debarquement du passager en question en ajustant sa liste de passagers
                passenger_bus = remove_chain(bus_passenger_list, &chain_bus);
                push(arrived_passenger, passenger_bus); // Passager arrive a sa station
                number_passenger -= 1;
            }
            else if((count_station == 0) && (chain_bus->data->transfert == 1))
            { // Verifie le transfert des passagers vers la queue de la station de metro
                printf("[bus] : transfert passager %u vers station %u\n",
                       chain_bus->data->identification_number,
                       MAX_STATION_BUS);
                // Transfere le passager vers la queue des stations 0 a 5
                passenger_bus = remove_chain(bus_passenger_list, &chain_bus);
                push(table_passenger[MAX_STATION_BUS], passenger_bus);
            }
            else
            { // Dans les autres cas, on passe au passager suivant
                chain_bus = chain_bus->next;
            }
        }

        /*
         * Faire l'embarquement des nouveaux passagers en examinant la queue qui correspond au compteur de station
         * Attention a la capacite maximale du vehicule
         */
        while(bus_passenger_list->size < MAX_CAPACITY_BUS)
        { // Rempli le bus jusqu'a sa limite
            if(table_passenger[count_station]->size > 0)
            { // Si il y a des passagers dans la file d'attente
                passenger_bus = pop(table_passenger[count_station]); // Recupere un passager de la file d'attente
                push(bus_passenger_list, passenger_bus); // Rajoute le passager dans la liste du bus
                printf("[bus] : [embarque] le passager %u\n", passenger_bus->identification_number);
            }
        }

        /*
         * Donner le controle au verificateur une fois l'execution termine via un rendez-vous bilateral. Chaque cycle
         * du thread autobus s'execute en concurrence avec un cycle du thread metro. Ces deux cycles sont toujours
         * suivis d'un cycle du thread verificateur
         */
    }

    delete_queue(bus_passenger_list);
    pthread_exit(NULL);
}

/*
 * Fonction thread pour metro
 */
void *thread_subway(queue **table_passenger)
{
    uint32_t count_station = MAX_STATION_BUS; // Compteur de la station
    _Bool increment = true; // Test pour verifier si on incremente le compteur ou decremente
    chain *chain_subway;
    passenger *passenger_subway;
    queue *subway_passenger_list = new_queue(); // Liste des

    while(number_passenger > 0)
    {
        if(increment)
        { // Incremente un compteur de station dans la direction originel
            count_station += 1;
        }
        else
        { // Decremente un compteur de station dans la direction opposee
            count_station -= 1;
        }

        if(count_station == MAX_STATION_BUS)
        { // Si on est a la station 5, on incrementera
            increment = true;
        }
        else if (count_station == MAX_STATION)
        { // Sinon, on decrementera
            increment = false;
        }

        chain_subway = subway_passenger_list->head; // On recupere la tete de la liste de passager du metro

        while(chain_subway != NULL)
        { // Parcours de la liste de passager du metro

            if(chain_subway->data->station_end == count_station)
            { // Verifier dans la liste de passager du bus si un passager est arrive a destination
                printf("[metro] : [debarque] le passager %u\n", chain_subway->data->identification_number);

                // Il assure le debarquement du passager en question en ajustant sa liste des passagers
                passenger_subway = remove_chain(subway_passenger_list, &chain_subway);
                push(arrived_passenger, passenger_subway);
                number_passenger -= 1;
            }
            else if((count_station == 0) && (chain_subway->data->transfert == 1))
            { // Verifie le transfert des passagers vers la queue de la station de bus
                printf("[metro] : transfert passager %u vers station %u\n",
                       chain_subway->data->identification_number,
                       0);
                // Tranfere le passager vers la queue des stations 5 a 0
                passenger_subway = remove_chain(subway_passenger_list, &chain_subway);
                push(table_passenger[0], passenger_subway);
            }
            else
            { // Dans les autres cas, on passe au passager suivant
                chain_subway = chain_subway->next;
            }
        }

        /*
         * Faire l'embarquement des nouveaux passagers en examinant la queue qui correspond au compteur de station
         * Attention a la capacite maximale du vehicule
         */
        while(subway_passenger_list->size < MAX_CAPACITY_SUBWAY)
        { // Rempli le metro jusqu'a sa limite
            if(table_passenger[count_station]->size > 0)
            { // Si il y a des passagers dans la file d'attente
                passenger_subway = pop(table_passenger[count_station]); // Recupere un passager de la file d'attente
                push(subway_passenger_list, passenger_subway); // Rajoute le passager dans la liste du metro
                printf("[metro] : [embarque] le passager %u\n", passenger_subway->identification_number);
            }
        }

        /*
         * Donner le controle au verificateur une fois l'execution termine via un rendez-vous bilateral. Chaque cycle
         * du thread autobus s'execute en concurrence avec un cycle du thread metro. Ces deux cycles sont toujours
         * suivis d'un cycle du thread verificateur
         */
    }

    delete_queue(subway_passenger_list);
    pthread_exit(NULL);
}

/*
 * Fonction pour le thread verificateur
 */
void *thread_check(queue** table_passenger)
{
    char *myfifo = "communication.fifo";
    int fd, index;
    chain *chain_passenger;
    passenger *passenger_check;

    while(number_passenger > 0)
    {
        for(index = 0; index < MAX_STATION; index++)
        { // Parcours toutes les stations
            chain_passenger = table_passenger[index]->head; // On recupere la tete de la file d'attente de passager

            while(chain_passenger != NULL)
            { // Parcours de la file d'attente de passager
                chain_passenger->data->wait_time_past += 1; // Incremente le temps d'attente du passager

                // Compare le temps d'attente de chaque passager avec son temps d'attente maximale
                if(chain_passenger->data->wait_time_past == chain_passenger->data->wait_time_maximum)
                {
                    if((fd = open(myfifo, O_WRONLY)) == -1)
                    { // Ouverture du pipe nomme
                        fprintf(stderr, "Erreur main.c : Impossible d'ouvrir l'entree du tube nomme.\n");
                        exit(EXIT_FAILURE);
                    }

                    passenger_check = remove_chain(table_passenger[index], &chain_passenger); // Recuperation du passager
                    /*
                     * Transferer le passager de la queue autobus/metro vers le tube nomme memorisant les passagers en
                     * attente de taxis.
                     */
                    write(fd, &passenger_check, sizeof(passenger));

                    // A COMPLETER ************************************************************************************

                    printf("verificateur : transfert du passager %u vers le taxi",
                           passenger_check->identification_number);

                    close(fd); // Fermeture du pipe nomme
                }
                else
                {
                    chain_passenger = chain_passenger->next;
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{

/*
 ***********************************************************************************************************************
 * Lecture et repartition des passagers dans les files d'attente des stations
 ***********************************************************************************************************************
 */
    // Creation de la file pour metro et bus
    FILE *file = fopen(argv[1], "rt"); // On recupere le fichier passe en parametre
    if(file == NULL)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier.\n");
        exit(EXIT_FAILURE);
    }

    // Tableau de file d'attente
    queue **table_passenger = malloc(MAX_STATION * sizeof(queue));
    passenger *passenger1;

    for(int index = 0; index < MAX_STATION; index++)
    { // Creation des files FIFO
        table_passenger[index] = new_queue();
    }

    fscanf(file, "%u\n", &number_passenger); // Recupere le nombre de passager total

    while(!feof(file))
    { // Tant que l'on n'a pas atteint la fin du fichier
        passenger1 = read_passenger(file); // Recupere un passager
        push(table_passenger[passenger1->station_start], passenger1); // Ajoute a la file d'attente
    }

    fclose(file); // Fermeture du fichier

/*
 ***********************************************************************************************************************
 * Creation du tube de communication nomme
 ***********************************************************************************************************************
 */
    int32_t fd;
    char *myfifo = "communication.fifo";

    // Creation d'un tube nomme avec permission : READ, WRITE, EXECUTE/SEARCH by OWNER
    if(mkfifo((myfifo), S_IRWXU) == -1)
    {
        fprintf(stderr, "Erreur lors de la creation du tube.\n");
        exit(EXIT_FAILURE);
    }

/*
 ***********************************************************************************************************************
 * Creation du processus taxis
 ***********************************************************************************************************************
 */

    pthread_t pthread_id[3];
    arrived_passenger = new_queue();

    if(fork())
    {
/*
 * *********************************************************************************************************************
 * Creation des threads
 * *********************************************************************************************************************
 */
        if(pthread_create(pthread_id+0, NULL, thread_bus, table_passenger) == -1)
        { // Creation du thread autobus
            fprintf(stderr, "Impossible de creer le thread bus");
            exit(EXIT_FAILURE);

        }

        if(pthread_create(pthread_id+1, NULL, thread_subway, table_passenger) == -1)
        { // Creation du thread metro
            fprintf(stderr, "Impossible de creer le thread metro");
            exit(EXIT_FAILURE);
        }

        if(pthread_create(pthread_id+2, NULL, thread_check, table_passenger) == -1)
        { // Creation du thread verificateur
            fprintf(stderr, "Impossible de creer le thread verificateur");
            exit(EXIT_FAILURE);
        }

        close(fd);

        for(int index = 0; index < 3; index++)
        {
            pthread_join(pthread_id[index], NULL);
        }
    }
    else
    {
        if((fd = open(myfifo, O_RDONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }

        close(fd);
    }

/*
 ***********************************************************************************************************************
 * Liberation de la memoire
 ***********************************************************************************************************************
 */

    unlink(myfifo);

    for(int index = 0; index < MAX_STATION; index++)
    { // Supprimer des files FIFO
        delete_queue(table_passenger[index]);
    }

    delete_queue(arrived_passenger);

    free(table_passenger);

    return EXIT_SUCCESS;
}

/*
 * Passage structure dans un pipe nomme
int main()
{
    int fd;
    char *myfifo = "test.fifo";
    passenger *passenger1 = malloc(sizeof(passenger));

    passenger1->station_start = 1;
    passenger1->station_end = 2;
    passenger1->transfert = 0;
    passenger1->wait_time_maximum = 15;
    passenger1->wait_time_past = 10;
    passenger1->identification_number = 10;



    // Creation d'un tube nomme avec permission : READ, WRITE, EXECUTE/SEARCH by OWNER
    if(mkfifo((myfifo), S_IRWXU) == -1)
    {
        fprintf(stderr, "Erreur lors de la creation du tube.\n");
        exit(EXIT_FAILURE);
    }

    if(fork())
    {
        if((fd = open(myfifo, O_WRONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }

        write(fd, &passenger1, sizeof(passenger));

        printf("#%u %hhu %hhu %u %hhu %u\n",
               passenger1->identification_number,
               passenger1->station_start,
               passenger1->station_end,
               passenger1->wait_time_past,
               passenger1->transfert,
               passenger1->wait_time_maximum);

        close(fd);

        if((fd = open(myfifo, O_RDONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        passenger *passenger2 = malloc(sizeof(passenger));
        if((fd = open(myfifo, O_RDONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }

        read(fd, &passenger2, sizeof(int32_t));

        printf("#%u %hhu %hhu %u %hhu %u\n",
               passenger2->identification_number,
               passenger2->station_start,
               passenger2->station_end,
               passenger2->wait_time_past,
               passenger2->transfert,
               passenger2->wait_time_maximum);

        close(fd);
        free(passenger2);

    }

    unlink(myfifo);

    free(passenger1);


    return EXIT_SUCCESS;
}
 */