#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		5001

static int 		tmostr;
static int		nmostr;
static char 	param[10];

void Param_Marxa(void)														//Funció per a demanar a l'usuari que entri els paràmetres de marxa abans d'enviar-ho al servidor
{
	printf("\nEntra el temps de mostreig (1-20s):");
	scanf("%d",&tmostr);													//L'usuari entra el temps de mostreig
	while (tmostr>20)														//Comprovar si el valor entrat supera el límit estipulat per l'enunciat. En cas que el superi:
	 {
		printf("*Valor erroni!*\nSiusplau, torni-ho a intentar: ");			//Notificar a l'usuari que el valor entrat és erroni i se li demana que el torni a entrar
		scanf("%d",&tmostr);											
	 }
	printf("\nEntra el nombre de mostres mitjana (1-9):");
	scanf("%d",&nmostr);													//L'usuari entra el nombre de valors per a fer la mitjana
	while (nmostr>9)														//Comprovar si el valor entrat supera el límit estipulat per l'enunciat. En cas que el superi:
	 {
		printf("*Valor erroni!*\nSiusplau, torni-ho a intentar: ");			//Notificar a l'usuari que el valor entrat és erroni i se li demana que el torni a entrar
		scanf("%d",&nmostr);
	 }
	sprintf (param,"{M1%.2d%d}",tmostr,nmostr);								//Es concatenen els valors entrats per l'usuari en el format exigit { M v Temps Temps Num }
}

void ImprimirMenu(void)														//Funció que imprimeix el menú de selecció
{
	printf("\n\nMenú de selecció:\n");
	printf("--------------------\n");
	printf("1: Mostra més antiga (ºC)\n");
	printf("2: Mostra màxima (ºC)\n");
	printf("3: Mostra mínima (ºC)\n");
	printf("4: Reset mostres màxima i mínima\n");
	printf("5: Mostres emmagatzemades\n");
	printf("6: Posada en marxa\n");
	printf("0: Sortir\n");
	printf("--------------------\n");
}


int main(int argc, char *argv[]){
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; 								//Adreça IP on està el servidor
	int			sockAddrSize;
	int			sFd;
	int 		indic = 0;
	int 		result;
	char		buffer[256];
	char 		input;
	
	INICI:
	indic = 0;
	
	/*Crear el socket*/
	sFd=socket(AF_INET,SOCK_STREAM,0);

	/*Construir l'adreça*/
	sockAddrSize = sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); 								//Posar l'estructura a zero
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons (SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr(serverName);

	/*Connexió*/
	result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	if (result < 0)
	{
		printf("Error en establir la connexió\n");
		exit(-1);
	}
	printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
	
	ImprimirMenu();
	input = getchar();

		switch (input)															//Switch case pel menu de selecció
		{
			case '0':
				strcpy (buffer,"{M0}");
				result = write(sFd, buffer, strlen(buffer));
				return 0;
				 
			case '1':
				printf("\nHeu seleccionat l'opció 1: Mostra més antiga\n\n");
				strcpy (buffer,"{U}");
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);                             
				break;
				
			case '2':
				printf("\nHeu seleccionat l'opció 2: Mostra màxima\n\n");
				strcpy (buffer,"{X}");
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);                            
				break;
				
			case '3':
				printf("\nHeu seleccionat l'opció 3: Mostra mínima\n\n");
				strcpy (buffer,"{Y}");
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);	                            
				break;
				
			case '4':
				printf("\nHeu seleccionat l'opció 4: Reset mostres màxima i mínima\n\n");
				strcpy (buffer,"{R}");
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);                          
				break;
				
			case '5':
				printf("\nHeu seleccionat l'opció 5: Mostres emmagatzemades\n\n");	
				strcpy (buffer,"{B}");
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);                         
				break;
				
			case '6':
				printf("\nHeu seleccionat l'opció 6: Posada en marxa\n\n");
				Param_Marxa();
				strcpy (buffer,param);
				result = write(sFd, buffer, strlen(buffer));
				printf("Missatge enviat (bytes %d): %s\n", result, buffer);                           
				break;
				                            
			case 0x0a: 														//Envia els 0x0a (line feed) que s'envia quan li donem al Enter
				break;
				
			default:
				printf("Opció incorrecta\n");
				strcpy (buffer,"ERROR");
				indic = 1;
				
		}
	
		input = getchar();
	
	if (indic!=1)
	{
		/*Rebre*/
		result = read(sFd, buffer, 256);
		printf("Missatge rebut del servidor(bytes %d): %s\n", result, buffer);

	}
	
		close(sFd);
		goto INICI;

	return 0;
	}
