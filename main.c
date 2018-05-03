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
 * Fonction thread pour autobus
 */

void * thread_autobus(queue * arg)
{

    uint32_t compteur_station = 0;
    chain *chain1;
    while(1)
    {
        while(arg[compteur_station].head->next !=NULL)
        {
            chain1 = arg[compteur_station].head;
            if ( arg[compteur_station].head->data->station_end == compteur_station)
            {
                printf("[Bus] debarque le passager %d", arg[compteur_station].head->data->identification_number);
            }
            arg[compteur_station].head = arg[compteur_station].head->next;
        }
        compteur_station = compteur_station + 1;
    }
}

/*
 * Fonction thread pour metro
 */

void * thread_metro(queue * arg)
{
    uint32_t compteur_station = 5;
    queue **metro_passenger = malloc(2*sizeof(queue));
    metro_passenger[0] = new_queue();
    metro_passenger[0]->size = 8;


    while(1)
    {
        if(compteur_station > 8)
        {

        }
        while (arg[compteur_station].head->next != NULL && metro_passenger[0]->size <= 8)
        {
            push(metro_passenger, arg[compteur_station].head);
            if(metro_passenger[0]->head->data->station_end == compteur_station)
            {
                printf("[Metro] debarque le passager %d", metro_passenger[0]->head->data->identification_number);
            }
            arg[compteur_station].head = arg[compteur_station].head->next;


        }
        compteur_station = compteur_station + 1;

    }

}

/*
 * Fonction pour le thread verificateur
 */

void * thread_verificateur(void * arg)
{

}

int main(int argc, char* argv[])
{

/*
 ***********************************************************************************************************************
 * Lecture et repartition des passagers dans les files d'attente des stations
 ***********************************************************************************************************************
 */
    pid_t taxi;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    FILE *file = fopen(argv[1], "rt"); // On recupere le fichier passe en parametre
    if(file == NULL)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier.\n");
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

    if((taxi = fork()) == 0)
    {
        if((fd = open(myfifo, O_WRONLY)) == -1)
        {
            fprintf(stderr, "Impossible d'ouvrir l'entree du tube nomme.\n");
            exit(EXIT_FAILURE);
        }

        close(fd);
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
 * *********************************************************************************************************************
 * Creation des threads
 * *********************************************************************************************************************
 */
    if( pthread_create(&thread1, NULL, thread_autobus, NULL ) == -1)
    {
        fprintf(stderr, "Impossible de creer le thread bus");
        exit(EXIT_FAILURE);

    }

    if(pthread_create(&thread2, NULL, thread_metro, increment_table_passenger)== -1)
    {
        fprintf(stderr, "Impossible de creer le thread metro");
        exit(EXIT_FAILURE);
    }

    if ( pthread_create(&thread3, NULL, thread_verificateur, increment_table_passenger) == -1)
    {
        fprintf(stderr, "Impossible de creer le thread verificateur");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);


/*
 ***********************************************************************************************************************
 * Liberation de la memoire
 ***********************************************************************************************************************
 */

    unlink(myfifo);

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