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

int nbLecteur = 0;
int indP = 0;
int indC = 0;

void *chariotFn(void *arg) {
	// Récupération de la socket passée en paramètre
	int socket = connexionServeurCOL3(ADRESSE, PORT, MONTOKEN, NOMDUCLAN);

	// Si la socket est valide
	if (socket != INVALID_SOCKET) {
		logClientCOL3(info, "chariotFn", "Socket validé [OK]");

		// Indéfiniment
		while (1)
		{
			// Envoie au serveur qu'on est un chariot
			char msg1[100] = "";
			sprintf(msg1, "%s%s%s", MSG_CHARIOT, MSG_DELIMINTER, MSG_QUEST);
			if (envoiMessageCOL3_s(socket, msg1) == -1) {
				logClientCOL3(error, "chariotFn", "envoie message 1 [FAIL]");
				exit(EXIT_FAILURE);
			}
			logClientCOL3(info, "chariotFn", "envoie message 1 [OK]");

			// Récupère la réponse du serveur
			char reponse1[100];
			lireMessageCOL3_s(socket, reponse1);
			if (strcmp(reponse1, MSG_CHARIOT_OK) != 0)
			{
				logClientCOL3(error, "chariotFn", "chariot vérif [FAIL]");
				exit(EXIT_FAILURE);
			}
			logClientCOL3(info, "chariotFn", "chariot vérif [OK]");
			
			// Envoie du chariot sur le bon site d'extraction
			char msg2[100] = "";
			// Stratégie de choix du site d'extraction
			int i = 0;
			while (CAPACITE_CLAN->sitesAccessibles[i++].quantite == 0 && i < MAX_SITE_EXTRACTION);
			// Envoie du message
			sprintf(msg2, "%s%s%d", MSG_CHARIOT, MSG_DELIMINTER, CAPACITE_CLAN->sitesAccessibles[i].idSite);
			if (envoiMessageCOL3_s(socket, msg2) == -1) {
				logClientCOL3(error, "chariotFn", "envoie message 2 [FAIL]");
				exit(EXIT_FAILURE);
			}
			logClientCOL3(info, "chariotFn", "envoie message 2 [OK]");

			// Récupère la réponse du serveur
			char reponse2[100];
			lireMessageCOL3_s(socket, reponse2);
			if (strcmp(reponse2, MSG_STOP) == 0) {
				logClientCOL3(error, "chariotFn", "chariot trajet [FAIL]");
				exit(EXIT_FAILURE);
			}
			logClientCOL3(info, "chariotFn", "chariot trajet [OK]");

			// Récupération des valeurs dans le message
			strtok(reponse2, MSG_DELIMINTER);				   // MSG_MATIERE
			int matiere = atoi(strtok(NULL, MSG_DELIMINTER));  // matière
			strtok(NULL, MSG_DELIMINTER);					   // MSG_QUANTITE
			int quantite = atoi(strtok(NULL, MSG_DELIMINTER)); // quantité

			// Lis la hutte
			readHutte();

			// Actualiste les valeurs
			pthread_mutex_lock(&red);

			HUTTE->stock[matiere] += quantite;
			time_t now;
			time(&now);
			HUTTE->tps_fin = now;

			pthread_mutex_unlock(&red);

			// Sauvegarde les valeurs
			saveHutte();
		}
	} else {
		logClientCOL3(error, "chariotFn", "Erreur socket [FAIL]");
	}
}


/* ===================================
     fonction d'extraction des sites 
       (a completer)
  ==================================*/
		   
capacite_clan* recupSiteExtraction() {
	// Connexion au serveur et création de la socket
	int socket = connexionServeurCOL3(ADRESSE, PORT, MONTOKEN, NOMDUCLAN);

	if (socket != INVALID_SOCKET) {
		logClientCOL3(debug, "connexionGetCapacite", "Socket validé [OK]");

		// Création du message
		char msg[100] = "";
		sprintf(msg, "%s%s%s", MSG_SITE, MSG_DELIMINTER, MSG_QUEST);
		logClientCOL3(info, "connexionGetCapacite", "Envoie message '%s'", msg);

		// Envoie du message
		if (envoiMessageCOL3_s(socket, msg) == -1) {
			logClientCOL3(error, "connexionGetCapacite", "Message envoyé [FAIL]");
			return NULL;
		} else {
			logClientCOL3(debug, "connexionGetCapacite", "Message envoyé [OK]");
		}

		// Récupération de la capacité
		capacite_clan *capacite = malloc(sizeof(capacite_clan));
		if (lireStructureCOL3_s(socket, capacite, sizeof(capacite_clan)) == 0) {
			logClientCOL3(error, "connexionGetCapacite", "Lecture structure [FAIL]");
			return NULL;
		}
		afficheCapaciteDuClan(*capacite);

		return capacite;
	} else {
		logClientCOL3(error, "connexionGetCapacite", "Erreur socket [fail]");

		return NULL;
	}
}


/* ===================================
     fonction de recupération de MP 
       (a completer)
  ==================================*/
		   
void gestionAppro()
{
	// nbChariot threads
	pthread_t thread[CAPACITE_CLAN->nbChariotDisponible];

	for (int i = 0; i < CAPACITE_CLAN->nbChariotDisponible; i++) {
		// Création de touts les threads chariots
		if (pthread_create(&thread[i], NULL, (void *) chariotFn, NULL) < 0) {
			logClientCOL3(error, "envoieChariots", "Création thread %d [FAIL]", i);
			exit(EXIT_FAILURE);
		}
	}
}


/**
 * Sauvegarde la hutte dans un fichier
 */
void saveHutte() {
	pthread_mutex_lock(&red);

	FILE *file;
	file = fopen("save.txt", "w");

	if (file) {
		logClientCOL3(info, "saveHutte", "Ouverture fichier [OK]");

		fprintf(file, "%s\n", HUTTE->nomClanHutte);

		fprintf(file, "%d\n", HUTTE->stock[0]);
		fprintf(file, "%d\n", HUTTE->stock[1]);
		fprintf(file, "%d\n", HUTTE->stock[2]);
		fprintf(file, "%d\n", HUTTE->stock[3]);
		fprintf(file, "%d\n", HUTTE->stock[4]);
		fprintf(file, "%d\n", HUTTE->stock[5]);

		fprintf(file, "%ld\n", HUTTE->tps_debut);
		fprintf(file, "%ld\n", HUTTE->tps_fin);

		logClientCOL3(info, "saveHutte", "Ecriture fichier [OK]");

		close(file);

		logClientCOL3(info, "saveHutte", "Fermeture fichier [OK]");
	} else {
		logClientCOL3(error, "saveHutte", "Ouverture fichier [FAIL]");
	}

	pthread_mutex_unlock(&red);
}

void readHutte() {
	// Avant de lire
	pthread_mutex_lock(&lect);
	nbLecteur++;
	if (nbLecteur == 1) {
		pthread_mutex_lock(&red);
	}
	pthread_mutex_unlock(&lect);

	// Lecture
	FILE *file = fopen("save.txt", "r");
	if (fscanf(file, "%s", HUTTE->nomClanHutte) == EOF) {
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[0]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[1]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[2]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[3]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[4]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%d", &HUTTE->stock[5]) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%ld", &HUTTE->tps_debut) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}
	if (fscanf(file, "%ld", &HUTTE->tps_fin) == EOF) {
		HUTTE = malloc(sizeof(hutte));
		return NULL;
	}

	// Après avoir lu
	pthread_mutex_lock(&lect);
	nbLecteur--;
	if (nbLecteur == 0) {
		pthread_mutex_unlock(&red);
	}
	pthread_mutex_unlock(&lect);
}

void *forgeBle(void *arg) {
	int n = *(int *) arg;

	// Indéfiniment
	while (1) {
		// On récupère la hutte
		readHutte();
		bool ok;

		for (int i = 0; i < 6; i++) {
			ok = ok && HUTTE->stock[i] - MATERIAUX_BALISTE[(int)BLE][i] >= 0;
		}

		if (ok) {
			pthread_mutex_lock(&red);
			
			for (int i = 0; i < 6; i++) {
				HUTTE->stock[i] -= MATERIAUX_BALISTE[(int) BLE][i];
			}

			pthread_mutex_unlock(&red);

			saveHutte();

			// Construction de la baliste
			sleep(TPS_FAB_BLE);

			// Ajout de la baliste dans le buffer
			sem_wait(&plein);
			sem_wait(&mutex);

			baliste *b = malloc(sizeof(baliste));
			b->baliste = BLE;
			b->date = time(NULL);
			b->forge = n;
			b->pVie = 6;

			buffer[indP] = b;
			logClientCOL3(info, "forgeBle", "Baliste légère produite à la forge %d", n);
			indP = (indP + 1) % TMP_MAX;
			
			sem_post(&mutex);
			sem_post(&vide);
		}
	}
}

void *forgeBlo(void *arg) {
	int n = *(int *) arg;

	// Indéfiniment
	while (1) {
		// On récupère la hutte
		readHutte();
		bool ok;

		for (int i = 0; i < 6; i++) {
			ok = ok && HUTTE->stock[i] - MATERIAUX_BALISTE[(int)BLO][i] >= 0;
		}

		if (ok) {
			pthread_mutex_lock(&red);

			for (int i = 0; i < 6; i++) {
				HUTTE->stock[i] -= MATERIAUX_BALISTE[(int)BLO][i];
			}

			pthread_mutex_unlock(&red);

			saveHutte();

			// Construction de la baliste
			sleep(TPS_FAB_BLO);

			// Ajout de la baliste dans le buffer
			sem_wait(&plein);
			sem_wait(&mutex);

			baliste *b = malloc(sizeof(baliste));
			b->baliste = BLO;
			b->date = time(NULL);
			b->forge = n;
			b->pVie = 10;

			buffer[indP] = b;
			logClientCOL3(info, "forgeBlo", "Baliste lourde produite à la forge %d", n);
			indP = (indP + 1) % TMP_MAX;

			sem_post(&mutex);
			sem_post(&vide);
		}
	}
}

void lanceForges() {
	// Forges légères
	pthread_t forge_ble[3];
	int ble[3] = {0, 1, 2};
	// Forges lourdes
	pthread_t forge_blo[2];
	int blo[2] = {0, 1};

	// Lancement des forges légères
	pthread_create(&forge_ble[0], NULL, (void *) forgeBle, (void *) ble[0]);
	pthread_create(&forge_ble[1], NULL, (void *) forgeBle, (void *) ble[1]);
	pthread_create(&forge_ble[2], NULL, (void *) forgeBle, (void *) ble[2]);

	// Lancement des forges lourdes
	pthread_create(&forge_blo[0], NULL, (void *) forgeBlo, (void *) blo[0]);
	pthread_create(&forge_blo[1], NULL, (void *) forgeBlo, (void *) blo[1]);
}

void *filsClan() {
	while (1) {
		sem_wait(&vide);
		sem_wait(&mutex);

		if (ARMEE->nbbaliste < TAILLE_MAX_ARMEE) {
			baliste *b = buffer[indC];
			indC = (indC + 1) % TMP_MAX;

			ARMEE->baliste[ARMEE->nbbaliste] = b->baliste;
			ARMEE->forge[ARMEE->nbbaliste] = b->forge;
			ARMEE->pvie[ARMEE->nbbaliste] = b->pVie;
			ARMEE->nbbaliste++;

			buffer[indC] = NULL;

			sem_post(&mutex);
			sem_post(&plein);
		}
	}
	
}

void lanceFilsClan() {
	pthread_t filsClan;

	pthread_create(&filsClan, NULL, (void *) filsClan, NULL);
}

	/*  ======================================
	  fonction de test d'échange initiale 
      avec le serveur
    =====================================*/

	int testServeur(const char *adresseip, int port, const char *tokenduclan, const char *nomduclan)
{
	int socket;

	logClientCOL3(info, "test", "le clan[%s] crée une socket pour tester le serveur", nomduclan);
	
	/* -----------------------------
	   ECHANGE 1 : envoi du token de test 
	   ----------------------------- */

	/* creation et connexion au serveur de socket */
	socket = connexionServeurCOL3(adresseip, port, tokenduclan, nomduclan); // on met MSG_TEST à la place du TOKEN


	/* -----------------------------
	   ECHANGE 2 : valildation echange  
	   ----------------------------- */
	if (socket != INVALID_SOCKET) {
		logClientCOL3(info, "test", "le clan[%s] a validé son test de connexion  %b ", nomduclan, debug_ok); 
		close(socket);
	} else {
		logClientCOL3(error, "test", "le clan[%s] n'a pas validé son test de connexion  %b ", nomduclan,debug_nok); 
	}

	return socket;
}
