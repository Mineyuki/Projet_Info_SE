#include "processus.h"
#include "queue.h"

uint32_t profit = 0;
uint32_t number_passenger; // Nombre de passager

sem_t rendez_vous_bus, rendez_vous_subway, rendez_vous_check_bus, rendez_vous_check_subway;

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
    fscanf(file, "# %u %hhu %hhu %u %hhu %u\n",
           &passenger1->identification_number,
           &passenger1->station_start,
           &passenger1->station_end,
           &passenger1->wait_time_past,
           &passenger1->transfert,
           &passenger1->wait_time_maximum);

    return passenger1; // Renvoie le passager
}

/*
 * Fonction recuperant le passager du maillon et changeant le maillon actuel par le suivant
 */
passenger *change_chain(queue *queue1, chain **return_chain)
{
    uint64_t passenger_position;
    passenger_position = find_passenger_position(queue1, (*return_chain));
    (*return_chain) = (*return_chain)->next;
    return remove_chain(queue1, passenger_position);
}

/*
 * Fonction thread pour autobus
 */
void *thread_bus(queue **table_passenger)
{
    uint64_t passenger_position = 0; // Position du passager
    uint32_t count_station = 0; // Compteur de la station
    chain *chain_bus;
    passenger *passenger_bus;
    queue *bus_passenger_list = new_queue(); // Creation d'une liste de passager

    while(number_passenger > 0)
    {
        count_station = count_station + 1; // Incremente un compteur de station
        if(count_station == MAX_STATION_BUS)
        { // Si on atteint le maximum de station, on remet le compteur a 0
            count_station = 0;
        }

        chain_bus = bus_passenger_list->head; // On recupere la tete de la liste de passager du bus

        while(chain_bus != NULL)
        { // Parcours de la liste de passager du bus
            if((chain_bus->data->station_end == count_station) ||
               (chain_bus->data->station_end == MAX_STATION_BUS))
            { // Verifier dans la liste de passager du bus si un passager est arrive a destination
                printf("[bus] : [debarque] le passager %u\n", chain_bus->data->identification_number);

                passenger_position = find_passenger_position(bus_passenger_list, chain_bus);
                chain_bus = chain_bus->next;
                // Il assure le debarquement du passager en question en ajustant sa liste de passagers
                passenger_bus = remove_chain(bus_passenger_list, passenger_position);
                free(passenger_bus);

                number_passenger = number_passenger - 1; // Decremente d'un passager
            }
            else if((count_station == 0) && (chain_bus->data->transfert == 1))
            { // Verifie le transfert des passagers vers la queue de la station de metro
                printf("[bus] : transfert passager %u vers station %u\n",
                       chain_bus->data->identification_number,
                       MAX_STATION_BUS);

                passenger_position = find_passenger_position(bus_passenger_list, chain_bus);
                chain_bus = chain_bus->next;

                // Transfere le passager vers la queue des stations 0 a 5
                passenger_bus = remove_chain(bus_passenger_list, passenger_position);
                push(table_passenger[MAX_STATION_BUS], passenger_bus); // Ajoute a la liste d'attente du metro

                profit = profit + 1; // Debourse 1$
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
                profit = profit + 1; // Debourse 1$
            }
            else
            {
                break;
            }
        }

        /*
         * Donner le controle au verificateur une fois l'execution termine via un rendez-vous bilateral. Chaque cycle
         * du thread autobus s'execute en concurrence avec un cycle du thread metro. Ces deux cycles sont toujours
         * suivis d'un cycle du thread verificateur
         */
        sem_post(&rendez_vous_check_bus); // Libere le verrou pose au verificateur
        sem_wait(&rendez_vous_bus); // Pose un verrou
    }

    delete_queue(bus_passenger_list); // Supprime la liste des passagers
    pthread_exit(NULL); // Termine le thread
}

/*
 * Fonction thread pour metro
 */
void *thread_subway(queue **table_passenger)
{
    uint64_t passenger_position = 0; // Position du passager
    uint32_t count_station = MAX_STATION_BUS; // Compteur de la station
    _Bool increment = true; // Test pour verifier si on incremente le compteur ou decremente
    chain *chain_subway;
    passenger *passenger_subway;
    queue *subway_passenger_list = new_queue(); // Liste des passagers

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
            count_station -= 2;
        }

        chain_subway = subway_passenger_list->head; // On recupere la tete de la liste de passager du metro

        while(chain_subway != NULL)
        { // Parcours de la liste de passager du metro

            if((chain_subway->data->station_end == count_station) ||
               (chain_subway->data->station_end == 0))
            { // Verifier dans la liste de passager du bus si un passager est arrive a destination
                printf("[metro] : [debarque] le passager %u\n", chain_subway->data->identification_number);

                passenger_position = find_passenger_position(subway_passenger_list, chain_subway);
                chain_subway = chain_subway->next;
                // Il assure le debarquement du passager en question en ajustant sa liste des passagers
                passenger_subway = remove_chain(subway_passenger_list, passenger_position);
                free(passenger_subway);

                number_passenger = number_passenger - 1; // Decremente d'un passager
            }
            else if((count_station == MAX_STATION_BUS) && (chain_subway->data->transfert == 1))
            { // Verifie le transfert des passagers vers la queue de la station de bus
                printf("[metro] : transfert passager %u vers station %u\n",
                       chain_subway->data->identification_number,
                       0);

                passenger_position = find_passenger_position(subway_passenger_list, chain_subway);
                chain_subway = chain_subway->next;
                // Tranfere le passager vers la queue des stations 5 a 0
                passenger_subway = remove_chain(subway_passenger_list, passenger_position);

                push(table_passenger[0], passenger_subway); // Ajoute a la liste d'attente du bus

                profit = profit + 1; // Debourse 1$
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
                profit = profit + 1; // Debourse 1$
            }
            else
            {
                break;
            }
        }

        /*
         * Donner le controle au verificateur une fois l'execution termine via un rendez-vous bilateral. Chaque cycle
         * du thread autobus s'execute en concurrence avec un cycle du thread metro. Ces deux cycles sont toujours
         * suivis d'un cycle du thread verificateur
         */
        sem_post(&rendez_vous_check_subway); // Libere le verrou du verificateur
        sem_wait(&rendez_vous_subway); // Pose un verrou
    }

    delete_queue(subway_passenger_list); // Supprime la liste des passager du metro
    pthread_exit(NULL); // Fin du thread
}

/*
 * Fonction pour le thread verificateur
 */
void *thread_check(queue** table_passenger)
{
    uint64_t passenger_position;
    char *myfifo = "communication.fifo";
    int fd, index;
    chain *chain_passenger;
    passenger *passenger_check;

    if((fd = open(myfifo, O_WRONLY)) == -1)
    { // Ouverture du pipe nomme
        fprintf(stderr, "Erreur main.c : Impossible d'ouvrir l'entree du tube nomme.\n");
        exit(EXIT_FAILURE);
    }

    while(number_passenger > 0)
    {
        sem_wait(&rendez_vous_check_bus); // Pose d'un verrou
        sem_wait(&rendez_vous_check_subway); // Pose d'un second verrou

        for(index = 0; index < MAX_STATION; index++)
        { // Parcours toutes les stations
            chain_passenger = table_passenger[index]->head; // On recupere la tete de la file d'attente de passager

            while(chain_passenger != NULL)
            { // Parcours de la file d'attente de passager
                chain_passenger->data->wait_time_past += 1; // Incremente le temps d'attente du passager

                // Compare le temps d'attente de chaque passager avec son temps d'attente maximale
                if(chain_passenger->data->wait_time_past == chain_passenger->data->wait_time_maximum)
                {
                    passenger_position = find_passenger_position(table_passenger[index], chain_passenger);
                    chain_passenger = chain_passenger->next;
                    passenger_check = remove_chain(table_passenger[index], passenger_position); // Recuperation du passager
                    /*
                     * Transferer le passager de la queue autobus/metro vers le tube nomme memorisant les passagers en
                     * attente de taxis.
                     */
                    write(fd, &passenger_check, sizeof(passenger));

                    printf("verificateur : transfert du passager %u vers le taxi\n",
                           passenger_check->identification_number);

                    profit = profit + 3; // Debourse 3$
                    number_passenger = number_passenger - 1; // Decremente d'un passager
                }
                else
                {
                    chain_passenger = chain_passenger->next;
                }
            }
        }

        sem_post(&rendez_vous_bus); // Liberation du verrou du bus
        sem_post(&rendez_vous_subway); // Liberation du verrou du metro
    }

    close(fd); // Fermeture du pipe nomme
    pthread_exit(NULL); // Fin du thread
}

/*
 * Fonction pour le thread du taxi
 */
void *thread_taxi(void *args)
{
    int fd;
    char *myfifo = "communication.fifo";
    passenger *passenger_taxi;

    if((fd = open(myfifo, O_RDONLY)) == -1)
    { // Ouverture du pipe nomme
        fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
        exit(EXIT_FAILURE);
    }

    while(number_passenger > 0)
    {
        read(fd, &passenger_taxi, sizeof(passenger)); // Recuperer les demandes du pipe (lecture bloquante)

        usleep(10); // Simule l'action de reconduire un passager

        printf("taxi#%d : passager %u est rendu a la station %hhu\n",
               pthread_self(),
               passenger_taxi->identification_number,
               passenger_taxi->station_end);

        free(passenger_taxi); // Liberation de la memoire

        number_passenger = number_passenger - 1; // Diminution d'un passager
    }

    close(fd); // Fermeture du pipe
    pthread_exit(NULL); // Fin du thread
}

int main(int argc, char* argv[])
{
    uint32_t index;
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

    for(index = 0; index < MAX_STATION; index++)
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

    sem_init(&rendez_vous_bus, 0, 1); // Initialisation du semaphore a 1
    sem_init(&rendez_vous_subway, 0, 1); // Initialisation du semaphore a 1
    sem_init(&rendez_vous_check_bus, 0, 0); // Initialisation du semaphore a 0
    sem_init(&rendez_vous_check_subway, 0, 0); // Initialisation du semaphore a 0

    if(fork())
    { // Creation des threads
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

        for(index = 0; index < 3; index++)
        { // Attente de la fin de chaque taxi
            pthread_join(pthread_id[index], NULL);
        }
    }
    else
    {
        pthread_t *pthread_id_taxi = malloc(MAX_TAXI * sizeof(pthread_t));
        if(pthread_id_taxi == NULL)
        {
            fprintf(stderr, "Erreur main.c : erreur lors de l'allocation memoire taxi\n");
            exit(EXIT_FAILURE);
        }

        for(index = 0; index < MAX_TAXI; index++)
        { // Creation de tous les taxis
            if(pthread_create(pthread_id+index, NULL, thread_taxi, NULL) == -1)
            { // Creation du thread taxi
                fprintf(stderr, "Impossible de creer le thread taxi\n");
                exit(EXIT_FAILURE);
            }
        }

        for(index = 0; index < MAX_TAXI; index++)
        { // Attente de la fin de chaque taxi
            pthread_join(pthread_id[index], NULL);
        }

        free(pthread_id_taxi); // Liberation du tableau de thread
    }

/*
 ***********************************************************************************************************************
 * Liberation de la memoire
 ***********************************************************************************************************************
 */

    unlink(myfifo);

    sem_destroy(&rendez_vous_bus);
    sem_destroy(&rendez_vous_subway);
    sem_destroy(&rendez_vous_check_bus);
    sem_destroy(&rendez_vous_check_subway);

    for(index = 0; index < MAX_STATION; index++)
    { // Supprimer des files FIFO
        delete_queue(table_passenger[index]);
    }

    free(table_passenger);

    printf("Profit de la journee : %u $\n", profit);

    return EXIT_SUCCESS;
}