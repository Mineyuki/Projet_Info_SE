#include "queue.h"

/*
 * Allouer un maillon
 */
chain *new_chain(passenger *passenger1)
{
    chain *chain1 = malloc(sizeof(chain)); // Alloue la taille d'un maillon
    if(chain1 == NULL)
    {
        fprintf(stderr, "Erreur queue.c : Impossible d'allouer un maillon.\n");
        exit(EXIT_FAILURE);
    }
    chain1->data = passenger1; // Affecte a la donnee du maillon, un passager
    return chain1; // Retourne le maillon
}

/*
 * Allouer une file
 */
queue *new_queue()
{
    queue *queue1 = malloc(sizeof(queue)); // Alloue la taille d'une file
    if(queue1 == NULL)
    {
        fprintf(stderr, "Erreur queue.c : Impossible d'allouer une file.\n");
        exit(EXIT_FAILURE);
    }
    queue1->size = 0; // La taille est a 0
    queue1->head = NULL; // Aucun maillon encore
    queue1->tail = NULL; // Aucun maillon encore
    return queue1; // Retourne la file
}

/*
 * Verifie si la file est vide
 */
_Bool is_empty(queue *queue1)
{
    return queue1->size == 0; // Retourne true si c'est vrai
}

/*
 * Ajout d'elements dans la file
 * On ajoute forcement en queue de file
 */
void push(queue *queue1, passenger *passenger1)
{
    chain *chain1 = new_chain(passenger1); // Cree un nouveau maillon
    chain1->next = NULL; // Prochain maillon est a null
    if(queue1->size > 0)
    { // S'il y a au moins un element dans la file
        queue1->tail->next = chain1; // Devient le prochain maillon du maillon de la queue de la file
    }
    else
    { // S'il n'existe aucun element dans la file
        queue1->head = chain1; // Devient la tete du maillon de la file
    }
    queue1->tail = chain1; // Devient la queue du maillon de la file

    queue1->size += 1; // Ajoute 1 a la taille de la file
}

/*
 * Suppression d'elements dans la file
 * On retire forcement les elements en tete de file
 */
passenger *pop(queue *queue1)
{
    chain *chain1 = queue1->head; // Recupere la tete du maillon
    passenger *passenger1 = chain1->data; // Recupere le passager

    queue1->head = queue1->head->next; // La tete de la file devient la tete de la file suivante
    free(chain1); // Libere la memoire du maillon
    queue1->size -= 1; // Decremente la taille de la file

    if(queue1->size == 0)
    { // Si la taille de la file est a 0
        queue1->tail = NULL; // La queue de la file devient null
    }

    return passenger1;
}

/*
 * Supprime une file
 */
void delete_queue(queue *queue1)
{
    passenger *passenger1;

    while(!is_empty(queue1))
    { // Tant que la file n'est pas vide
        passenger1 = pop(queue1); // Recupere la donnee en tete
        free(passenger1); // Libere l'espace de la donnee
    }

    free(queue1);
}

