#include "processus.h"
#include "queue.h"

/*
 * Variable globale
 */
queue * metro_passenger;
queue * bus_passenger;

uint32_t number_passenger; // Nombre de passager

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

        free((*chain_remove)); // Supprime le maillon a supprimer

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
    bus_passenger = new_queue(); // Creation d'une liste de passager
    uint32_t count_station = 0; // Compteur de la station
    chain *chain_bus;
    passenger *passenger_bus;

    while(number_passenger > 0)
    {
        // Incremente un compteur de station
        count_station = count_station + 1;
        if(count_station == MAX_STATION_BUS)
        { // Si on atteint le maximum de station, on remet le compteur a 0
            count_station = 0;
        }

        chain_bus = bus_passenger->head; // On recupere la tete de la liste de passager du bus

        while(chain_bus != NULL)
        { // Parcours de la liste de passager du bus

            if (chain_bus->data->station_end == count_station)
            { // Verifier dans la liste de passager du bus si un passager est arrive a destination
                printf("[bus] : [debarque] le passager %u", chain_bus->data->identification_number);

                // Il assure le debarquement du passager en question en ajustant sa liste de passagers
                passenger_bus = remove_chain(bus_passenger, &chain_bus);
                free(passenger_bus);
            }
            else if((count_station == 0) && (chain_bus->data->transfert == 1))
            { // Verifie le transfert des passagers vers la queue de la station de metro
                printf("[bus] : transfert passager %u vers station %u",
                       chain_bus->data->identification_number,
                       MAX_STATION_BUS);
                // Transfere le passager vers la queue des stations 0 a 5
                passenger_bus = remove_chain(bus_passenger, &chain_bus);
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
        while(bus_passenger->size < MAX_CAPACITY_BUS)
        { // Test si la capacité du bus est atteinte
            passenger_bus = pop(table_passenger[count_station]); // Recupere un passager de la file d'attente
            push(bus_passenger, passenger_bus); // Rajoute le passager dans la liste du bus
        }

        /*
         * Donner le controle au verificateur une fois l'execution termine via un rendez-vous bilateral. Chaque cycle
         * du thread autobus s'execute en concurrence avec un cycle du thread metro. Ces deux cycles sont toujours
         * suivis d'un cycle du thread verificateur
         */
    }
}

/*
 * Fonction thread pour metro
 */
void *thread_subway(queue **arg)
{
    uint32_t compteur_station = 5;
    metro_passenger = new_queue();


    while(number_passenger > 0)
    {
        if(compteur_station > 8)
        {

        }
        while (arg[compteur_station]->head->next != NULL && metro_passenger->size <= 8)  // Parcours de files
        {
            if (metro_passenger->size > MAX_CAPACITY_SUBWAY)
                printf("Capacite maximale du metro atteinte");
            else
                push(metro_passenger, arg[compteur_station]->head->data); // Rajoute le voyageur dans la file du metro
            printf("[metro] transfert du passager %d vers station %d", metro_passenger->head->data->identification_number, metro_passenger->head->data->station_end);
            while(metro_passenger->head->next != NULL)
            {
                if (metro_passenger->head->data->station_end == compteur_station)
                {
                    printf("[Metro] debarque le passager %d", metro_passenger->head->data->identification_number);
                    pop(metro_passenger);
                }
                metro_passenger->head = metro_passenger->head->next;
            }
            arg[compteur_station]->head = arg[compteur_station]->head->next;


        }
        compteur_station = compteur_station + 1;

    }

}

/*
 * Fonction pour le thread verificateur
 */

void *thread_check(queue** arg)
{
    char * myfifo = "communication.fifo";
    int fd;
    while(number_passenger > 0)
    {
        for (int i = 0 ; i < MAX_STATION ; i ++)
        {
            while(arg[i]->head->next != NULL)
            {
                arg[i]->head->data->wait_time_past ++;
                if(arg[i]->head->data->wait_time_past == arg[i]->head->data->wait_time_maximum)
                {
                    printf("Le passager %d a depasse son temps d'attente\n", arg[i]->head->data->identification_number);
                    if((fd =open(myfifo, O_WRONLY)) == -1)
                    {
                        printf("Erreur d'ouverture du pipe nomme\n");
                    }
                    else
                    {
                        write(fd,arg[i]->head, sizeof(arg[i]->head));
                        printf("verificateur : transfert du passager %d vers le taxi \n",arg[i]->head->data->identification_number );
                        close(fd);
                    }

                }
                arg[i]->head = arg[i]->head->next;

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

    fscanf(file, "%u", &number_passenger); // Recupere le nombre de passager total

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

    pthread_t id_thread_bus;
    pthread_t id_thread_subway;
    pthread_t id_thread_check;

    if(fork())
    {
        if((fd = open(myfifo, O_WRONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }

/*
 * *********************************************************************************************************************
 * Creation des threads
 * *********************************************************************************************************************
 */
        if(pthread_create(&id_thread_bus, NULL, thread_bus, table_passenger) == -1)
        { // Creation du thread autobus
            fprintf(stderr, "Impossible de creer le thread bus");
            exit(EXIT_FAILURE);

        }

        if(pthread_create(&id_thread_subway, NULL, thread_subway, table_passenger) == -1)
        { // Creation du thread metro
            fprintf(stderr, "Impossible de creer le thread metro");
            exit(EXIT_FAILURE);
        }

        if(pthread_create(&id_thread_check, NULL, thread_check, table_passenger) == -1)
        { // Creation du thread verificateur
            fprintf(stderr, "Impossible de creer le thread verificateur");
            exit(EXIT_FAILURE);
        }

        close(fd);
        pthread_exit(NULL); // Attend la fin des autres threads
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