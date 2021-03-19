#ifndef CLIENTCOL3_H_INCLUDED
#define CLIENTCOL3_H_INCLUDED

#include <pthread.h>
#include "col3-bibtp/communCOL3-TP.h"


extern hutte HUTTECLAN;
extern capacite_clan CAPACITECLAN;

pthread_mutex_t mutex;

void recupSiteExtraction();
void gestionAppro();
void saveHutte();
capacite_clan *connexionGetCapacite(const char *adresse, int port, const char *token, const char *clan);

#endif // CLIENTCOL3_H_INCLUDED
