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
	printf("\n----------------------------------------------------------------\n");
	printf("\nMenú de selecció:\n");
	printf("____________________\n\n");
	printf("1: Mostra més antiga (ºC)\n");
	printf("2: Mostra màxima (ºC)\n");
	printf("3: Mostra mínima (ºC)\n");
	printf("4: Reset mostres màxima i mínima\n");
	printf("5: Mostres emmagatzemades\n");
	printf("6: Posada en marxa\n");
	printf("7: Aturada adquisició\n");
	printf("0: Sortir\n");
	printf("____________________\n\n");
}


int main(int argc, char *argv[]){
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; 								//Adreça IP on està el servidor
	int			sockAddrSize;
	int			sFd;
	int 		indic = 0;													//Variable per a identificar si s'ha executat default en el switch case del menú de selecció
	int 		result;
	char		buffer[256];
	char 		input;
	
	INICI:																	//Punt INICI, usat per a poder ser executar cíclicament
	indic = 0;																//S'assegura que la variable s'inicii a 0 
	
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
	printf("\n\n\n\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
	
	ImprimirMenu();
	input = getchar();

		switch (input)																		//Switch case pel menu de selecció
		{
			case '0':
				strcpy (buffer,"{M0}");														//En cas de tancar el client, envia un missatge de parada al servidor abans d'aturar l'execució
				result = write(sFd, buffer, strlen(buffer));
				return 0;
				 
			case '1':
				printf("\nHeu seleccionat l'opció 1: Mostra més antiga\n\n");				//Mostra a l'usuari l'opció que ha triat
				strcpy (buffer,"{U}");														//Es posa en buffer el missatge que volem enviar, seguint el protocol de comunicació establert
				result = write(sFd, buffer, strlen(buffer));								//S'envia el contingut del buffer
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                             
				break;
				
			case '2':
				printf("\nHeu seleccionat l'opció 2: Mostra màxima\n\n");					//Ídem cas '1'
				strcpy (buffer,"{X}");														//		"
				result = write(sFd, buffer, strlen(buffer));								//		"
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                            
				break;
				
			case '3':
				printf("\nHeu seleccionat l'opció 3: Mostra mínima\n\n");					//Ídem cas '1'
				strcpy (buffer,"{Y}");														//		"
				result = write(sFd, buffer, strlen(buffer));								//		"
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);	                            
				break;
				
			case '4':
				printf("\nHeu seleccionat l'opció 4: Reset mostres màxima i mínima\n\n");	//Ídem cas '1'
				strcpy (buffer,"{R}");														//		"
				result = write(sFd, buffer, strlen(buffer));								//		"
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                          
				break;
				
			case '5':
				printf("\nHeu seleccionat l'opció 5: Mostres emmagatzemades\n\n");			//Ídem cas '1'
				strcpy (buffer,"{B}");														//		"
				result = write(sFd, buffer, strlen(buffer));								//		"
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                         
				break;
				
			case '6':
				printf("\nHeu seleccionat l'opció 6: Posada en marxa\n");					//Ídem cas '1'
				Param_Marxa();																//Es crida la funció Param_marxa
				strcpy (buffer,param);														//Es posa en buffer els paràmetres establerts a través de la funció anterior
				result = write(sFd, buffer, strlen(buffer));								//S'envia el contingut del buffer
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                           
				break;
				
			case '7':
				printf("\nHeu seleccionat l'opció 7: Aturada adquisició\n\n");				//Ídem cas '1'
				strcpy (buffer,"{M000Z}");													//S'envia el codi per a parar el mostreig.
				result = write(sFd, buffer, strlen(buffer));								//		"
				//printf("Missatge enviat (bytes %d): %s\n", result, buffer);                         
				break;
				                            
			case 0x0a: 																		//Envia els 0x0a (line feed) que s'envia quan li donem al Enter
				break;
				
			default:
				printf("\nOpció incorrecta\n");												//En cas d'entrar un nombre no considerat en el switch case, mostra missatge d'error per pantalla
				indic = 1;																	//Indicdor per identificar si es vé de defalut. Si indic == 1, no haurà d'executar la funció result (es va provar a executar i el programa s'hi quedava encallat)
				
		}
	
		input = getchar();
	
	if (indic!=1)																			
	{
		/*Rebre*/
		result = read(sFd, buffer, 256);
		//printf("Missatge rebut del servidor(bytes %d): %s\n", result, buffer);

		if (buffer[2]=='0'){																	//En cas que el codi retorn c sigui igual a 0 (cap error), procedeix a mostrar la informació pertinent de cada cas
			switch (buffer[1])
			{
				case 'U':
               		printf("La mostra més antiga és: %c%c%c%c%c ℃\n",buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
					break;
               	
				case 'X':
			    	printf("La mostra màxima és: %c%c%c%c%c ℃\n",buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
			    	break;
			    
				case 'Y':
			    	printf("La mostra mínima és: %c%c%c%c%c ℃\n",buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
		            break;
		        
				case 'B':
		            printf("El nombre de mostres emmagatzemades és: %c%c%c%c \n",buffer[3],buffer[4],buffer[5],buffer[6]);
		        	break;
		        
				default:
					printf("Acció realitzada correctament");								//En cas que l'acció demanada per part del client sigui Reset max i min o bé marxa
		        	break;																	// i el codi de retorn c sigui igual a 0 (cap error), mostra el missatge en pantalla
				}
			}	
			else if(buffer[2]=='1') 															//En cas que c==1, mostra per pantalla 'ERROR PROTOCOL'
			{
				printf("ERROR PROTOCOL");
			}
			else 																			//En cas que c==2, mostra per pantalla 'ERROR PARÀMETRES'
			{
				printf("ERROR PARÀMETRES");
			}
	}
	
		close(sFd);	
		goto INICI;																			//Retorna al punt INICI

	return 0;
	}
