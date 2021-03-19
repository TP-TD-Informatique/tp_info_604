#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "col3-bibtp/communCOL3-TP.h"
#include "clientCOL3.h"


void *chariotFn(void *arg) {
	capacite_clan *capacite = (capacite_clan *) arg;

	int socket = connexionServeurCOL3("srv1.col3.learninglab.eu", 8080, "IC:22:NC:4:LS:1-10-6-12-13-18-11-19-20-8-4-17-5-3-16-9-2-7", "25");

	if (socket != INVALID_SOCKET) {
		logClientCOL3(info, "chariotFn", "Socket validé [OK]");
		
		char msg1[100] = "";
		strcat(msg1, MSG_CHARIOT);
		strcat(msg1, MSG_DELIMINTER);
		strcat(msg1, MSG_QUEST);
		if (envoiMessageCOL3_s(socket, msg1) == -1) {
			logClientCOL3(error, "chariotFn", "envoie message 1 [FAIL]");
			exit(EXIT_FAILURE);
		}
		logClientCOL3(info, "chariotFn", "envoie message 1 [OK]");

		char reponse1[100];
		lireMessageCOL3_s(socket, reponse1);
		if (strcmp(reponse1, MSG_CHARIOT_OK) != 0) {
			logClientCOL3(error, "chariotFn", "chariot vérif [FAIL]");
			exit(EXIT_FAILURE);
		}
		logClientCOL3(info, "chariotFn", "chariot vérif [OK]");

		char msg2[100] = "";
		int i = 0;
		while (capacite->sitesAccessibles[i++].quantite == 0 && i < MAX_SITE_EXTRACTION);
		sprintf(msg2, "%s%s%d", MSG_CHARIOT, MSG_DELIMINTER, capacite->sitesAccessibles[i].idSite);
		if (envoiMessageCOL3_s(socket, msg2) == -1) {
			logClientCOL3(error, "chariotFn", "envoie message 2 [FAIL]");
			exit(EXIT_FAILURE);
		}
		logClientCOL3(info, "chariotFn", "envoie message 2 [OK]");

		char reponse2[100];
		lireMessageCOL3_s(socket, reponse2);
		if (strcmp(reponse2, MSG_STOP) == 0) {
			logClientCOL3(error, "chariotFn", "chariot trajet [FAIL]");
			exit(EXIT_FAILURE);
		}
		logClientCOL3(info, "chariotFn", "chariot trajet [OK]");

		// Sauvegarde dans la hutte
		strtok(reponse2, MSG_DELIMINTER); // MSG_MATIERE
		int matiere = atoi(strtok(NULL, MSG_DELIMINTER)); // matière
		strtok(NULL, MSG_DELIMINTER); // MSG_QUANTITE
		int quantite = atoi(strtok(NULL, MSG_DELIMINTER)); // quantité

		HUTTECLAN.stock[matiere] += quantite;
		time_t now;
		time(&now);
		HUTTECLAN.tps_fin = now;

		close(socket);
	} else
		logClientCOL3(error, "chariotFn", "Erreur socket [FAIL]");


	return NULL;
}


/* ===================================
     fonction d'extraction des sites 
       (a completer)
  ==================================*/
		   
void recupSiteExtraction()
{
	int socket = connexionServeurCOL3("srv1.col3.learninglab.eu", 8080, "IC:22:NC:4:LS:1-10-6-12-13-18-11-19-20-8-4-17-5-3-16-9-2-7", "25");

	if (socket != INVALID_SOCKET) {
		logClientCOL3(debug, "connexionGetCapacite", "Socket validé [OK]");

		char msg[100] = "";
		//msg[0] = '\0';
		strcat(msg, MSG_SITE);
		strcat(msg, MSG_DELIMINTER);
		strcat(msg, MSG_QUEST);
		logClientCOL3(info, "connexionGetCapacite", "Envoie message '%s'", msg);
		if (envoiMessageCOL3_s(socket, msg) == -1)
			logClientCOL3(error, "connexionGetCapacite", "Message envoyé [FAIL]");
		else
			logClientCOL3(debug, "connexionGetCapacite", "Message envoyé [OK]");

		if (lireStructureCOL3_s(socket, &CAPACITECLAN, sizeof(capacite_clan)) == 0)
			logClientCOL3(error, "connexionGetCapacite", "Lecture structure [FAIL]");

		close(socket);
	}
	else
		logClientCOL3(error, "connexionGetCapacite", "Erreur socket [fail]");
}


/* ===================================
     fonction de recupération de MP 
       (a completer)
  ==================================*/
		   
void gestionAppro()
{
	time_t now;
	time(&now);
	HUTTECLAN.tps_debut = now; 

	pthread_t thread[CAPACITECLAN.nbChariotDisponible];

	for (int i = 0; i < CAPACITECLAN.nbChariotDisponible; i++)
	{
		if (pthread_create(&thread[i], NULL, chariotFn, &CAPACITECLAN) < 0)
		{
			logClientCOL3(error, "envoieChariots", "Création thread %d [FAIL]", i);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < CAPACITECLAN.nbChariotDisponible; i++)
		pthread_join(thread[i], NULL);
}


/**
 * Sauvegarde la hutte dans un fichier
 */
void saveHutte() {
	pthread_mutex_lock(&mutex);

	FILE *file;
	file = fopen("save.txt", "w");

	if (file) {
		logClientCOL3(info, "saveHutte", "Ouverture fichier [OK]");

		fprintf(file, "%s\n", HUTTECLAN.nomClanHutte);

		fprintf(file, "%d\n", HUTTECLAN.stock[0]);
		fprintf(file, "%d\n", HUTTECLAN.stock[1]);
		fprintf(file, "%d\n", HUTTECLAN.stock[2]);
		fprintf(file, "%d\n", HUTTECLAN.stock[3]);
		fprintf(file, "%d\n", HUTTECLAN.stock[4]);
		fprintf(file, "%d\n", HUTTECLAN.stock[5]);

		fprintf(file, "%ld\n", HUTTECLAN.tps_debut);
		fprintf(file, "%ld\n", HUTTECLAN.tps_fin);

		logClientCOL3(info, "saveHutte", "Ecriture fichier [OK]");

		close(file);

		logClientCOL3(info, "saveHutte", "Fermeture fichier [OK]");
	} else
		logClientCOL3(error, "saveHutte", "Ouverture fichier [FAIL]");

	pthread_mutex_unlock(&mutex);
}

	/*  ======================================
	  fonction de test d'échange initiale 
      avec le serveur
    =====================================*/

	int testServeur(const char *adresseip, int port, const char *tokenduclan, const char *nomduclan)
{
	int socket;


	logClientCOL3(info,"test", 
					  "le clan[%s] crée une socket pour tester le serveur",
					  nomduclan);
	
	/* -----------------------------
	   ECHANGE 1 : envoi du token de test 
	   ----------------------------- */

	/* creation et connexion au serveur de socket */
	socket = connexionServeurCOL3(adresseip,port,tokenduclan,nomduclan); // on met MSG_TEST à la place du TOKEN


	/* -----------------------------
	   ECHANGE 2 : valildation echange  
	   ----------------------------- */
	if (socket != INVALID_SOCKET) {

		logClientCOL3(info,"test", 
					  "le clan[%s] a validé son test de connexion  %b ",
					  nomduclan,debug_ok); 
		close(socket);
			
	} 
	else {
				logClientCOL3(error,"test", 
					  "le clan[%s] n'a pas validé son test de connexion  %b ",
					  nomduclan,debug_nok); 
	}
	


	return socket;
}

