#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "col3-bibtp/communCOL3-TP.h"
#include "clientCOL3.h"

/* variables globales */

niveau_log NIVEAULOG=none;						// mode debug
int NCURSE =0;						// affichage avec NCURSE

/*  programme principal du client CoL3  */
int main(int argc, char *argv[])
{
	int i=1 ;
	int help=0;
	int test=0;
	
	/* --- cette zone est à modifier à l'issue du jeu --- */
	strcpy(MONTOKEN, "IC:22:NC:4:LS:1-10-6-12-13-18-11-19-20-8-4-17-5-3-16-9-2-7");
	strcpy(NOMDUCLAN, "25");
	strcpy(ADRESSE, "192.168.184.12");
	PORT = 8080;
    /* --- cette zone est à modifier à l'issue du jeu --- */

	
	/* boucle de traitements des arguments */
    while(i < argc && help==0) {

		/* si demande d'aide */
		if ((0 == strcmp("-h",argv[i])) || (0 == strcmp("--help",argv[i]))){
			help=1;
        } 
		else if (0 == strcmp("--test",argv[i])){
			test=1;
        } 
		else if (0 == strcmp("--full",argv[i])){
			NIVEAULOG=full;
        } 
		else if (0 == strcmp("--debug",argv[i])){
			NIVEAULOG=debug;
        } 
		else if (0 == strcmp("--info",argv[i])){
			NIVEAULOG=info;
        } 
		else if (0 == strcmp("--error",argv[i])){
			NIVEAULOG=error;
        } 
        else if (0 == strcmp("-p",argv[i])){
			if (argv[i+1]!= NULL) PORT  = (int) strtol(argv[i+1], (char **)NULL, 10);
		} 
		else if (0 == strcmp("-n",argv[i])){
            if (argv[i+1]!= NULL)  strcpy(NOMDUCLAN,argv[i+1]);
        }
		else if (0 == strcmp("-t",argv[i])){
            if (argv[i+1]!= NULL)  strcpy(MONTOKEN,argv[i+1]);
		}
		else if (0 == strcmp("-a",argv[i])){
            if (argv[i+1]!= NULL)  strcpy(ADRESSE ,argv[i+1]);
        }		
        i++;
    }

	/* en cas d'aide : affichage des options */
	if (help == 1)
	{
		printf(" Description : pgm client Clash of L3 \n");
		printf(" Aide : [-h | --help] \n");
		printf(" Usage : col3-client-etu -a [adresseIP] -p [port] -n [nomduclan] -t [token]   \n");
		printf("	        	 [--test]\n");
		printf("	        	 [--full | --debug | --info | --error] \n");
		printf("  -p [port] : port d'accès du serveur CLAN (valeur par defaut = 8080) \n");
		printf("  -a [adresseIP] : adresse IP du serveur  (valeur par defaut = 127.0.0.1) \n");
		printf("  -n [nomduclan] : nom du clan) \n");
		printf("  -t [token] : token du clan) \n");
		printf("  --test  : lance le test de connexion avec le serveur CoL3\n");
		printf("  --full | --debug | --info | --error : lance le client avec un niveau de log \n\n");
	}
	/* sinon je lance la fonction principale */
	else
	{
		printf("\n  *** Clash Of L3 ****\n");

		printf("\n%s %s", "nom du Clan = ",NOMDUCLAN);
		printf("\n%s %s", "token du Clan = ",MONTOKEN);
		printf("\n%s %s", "adresse IP du serveur= ",ADRESSE);
		printf("\n%s %d", "port du serveur = ",PORT);
		printf("\n");

		if (test==1){
			testServeur(ADRESSE,PORT,MSG_TEST,NOMDUCLAN);
		}
		else
		{
			// Initialisation mutex lecteur/rédacteur
			if (pthread_mutex_init(&lect, NULL) != 0) {
				logClientCOL3(error, "main", "init mutex lect [FAIL]");
				exit(EXIT_FAILURE);
			}
			if (pthread_mutex_init(&red, NULL) != 0) {
				logClientCOL3(error, "main", "init mutex red [FAIL]");
				exit(EXIT_FAILURE);
			}

			// Initialisation sémaphores producteur/consommateur
			sem_init(&plein, 0, TMP_MAX);
			sem_init(&vide, 0, 0);
			sem_init(&mutex, 0, 1);

			// Récupération de la capacite_clan
			CAPACITE_CLAN = recupSiteExtraction();
			if (CAPACITE_CLAN == NULL) {
				logClientCOL3(error, "main", "Erreur récupération capacite_clan");
				exit(EXIT_FAILURE);
			}

			// Lecture de la hutte
			readHutte();
			
			// Gestion approvisionnement (chariots)
			gestionAppro();

			// Lance les forges
			lanceForges();

			// Initialisation de l'armée
			ARMEE = malloc(sizeof(armee));
			ARMEE->nbbaliste = 0;

			// Lance le fils du clan
			lanceFilsClan();
		}
	}
		
    return 0;
}







