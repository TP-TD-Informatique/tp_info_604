#ifndef CLIENTCOL3_H_INCLUDED
#define CLIENTCOL3_H_INCLUDED

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "col3-bibtp/communCOL3-TP.h"

char NOMDUCLAN[TAILLE_MAX_NOM_CLAN]; // nom du clan

int PORT;           // port de connexion au serveur
int PORTBATAILLE;   // port de connexion pour la bataille
char ADRESSE[30];   // adresse IP du serveur
char MONTOKEN[100]; // token unique du clan

capacite_clan *CAPACITE_CLAN;
hutte *HUTTE;
armee *ARMEE;

pthread_mutex_t lect,red;
sem_t plein, vide, mutex;

baliste *buffer[TMP_MAX];

capacite_clan* recupSiteExtraction();
void gestionAppro();
void saveHutte();
void readHutte();
void lanceForges();
void lanceFilsClan();

#endif // CLIENTCOL3_H_INCLUDED
