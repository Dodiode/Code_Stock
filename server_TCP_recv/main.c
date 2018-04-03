#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MYPORT 12340		/* le port de connection pour les utilisateurs */
#define BACKLOG 2     	/* Le nombre maxi de connections en attente  */
#define MAXBUFLEN 512	/* taille max du buffer de réception des données */
#define LINE printf("%i\n",__LINE__);

/* Variables globales */
volatile sig_atomic_t ctrlc = 0;


/* Prototypes de fonctions */
int create_socket_TCP(struct sockaddr_in* my_addr);
static void ctrl_c(int signo);


int main(int argc, char *argv[])
{
	FILE *f = fopen("val.txt", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	
	int sockfd, new_fd;		/* Écouter sur sock_fd, nouvelle connection sur new_fd */
	struct sockaddr_in my_addr;	/* information d'adresse */
	struct sockaddr* their_addr;	/* information d'adresse du client */
	int sin_size, ret, on, numbytes;
	char buf[MAXBUFLEN];
	
	my_addr.sin_family = AF_INET;		 /* host byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-remplissage avec mon IP */
	my_addr.sin_port = htons(MYPORT);	 /* short, network byte order */
	memset(&(my_addr.sin_zero), '\0', 8); /* zero pour le reste de struct */
	
	sockfd = create_socket_TCP(&my_addr);
	
	printf("-- Serveur TCP, appuyer sur Ctrl-C pour arrêter le programme --\n");
	
	printf("Attente d'une connexion...\n");
	if(listen(sockfd, BACKLOG)){ /* écoute sur le socket pour nouvelle connexion */
		perror("listen failed");
		exit(1);
	}
	sin_size = sizeof(struct sockaddr_in);
	new_fd = accept(sockfd, their_addr, &sin_size);	/* accepte connexion */
	close(sockfd);
	printf("Client connecté !\n");
	
	signal(SIGINT, ctrl_c); /* Permet de capturer le signal Ctrl-C */
	
	char msg[MAXBUFLEN];
	int len = 0, len_tot = 0;

	while(1){
	
	  do{
	  	memset(msg, 0, MAXBUFLEN);
	  	if((len = recv(new_fd, msg, sizeof(msg), 0)) >= 0){
	  		len_tot += len;
	  		fprintf(f,"%s",msg);
	  	}
	  }
	  while (len > 0 && len < sizeof(msg) -1 );

		if(ctrlc){
			close(new_fd);
			return 0;
		}
	}	
	close(sockfd);
}

/* Handler du signal "Ctrl-C", permet de fermer la connexion TCP */
static void ctrl_c(int signo){
	ctrlc = 1;
}

/* Crée un socket et bind avec l'adresse en paramètre, retourne l'identifiant du socket */
int create_socket_TCP(struct sockaddr_in* my_addr){
	int sockfd, ret, on;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* création du socket */
		perror("socket failed");
		exit(1);
	}
	
	/* Enable address reuse */
	on = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))){
		perror("setsockopt failed");
		exit(1);
	}

	/* bind entre l'adresse et le socket créé */
	if (bind(sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind failed");
		exit(1);
	}
	
	return sockfd;
}
