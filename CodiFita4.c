#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <termios.h>       
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>

#define SERVER_PORT_NUM	5001
#define SERVER_MAX_CONNECTIONS 4
#define REQUEST_MSG_SIZE 1024
#define SIZE_OF_BUFFER 100				//Tamany buffer circular.
#define BAUDRATE B9600                  //Velocitat comunicació sèrie.                           
//#define MODEMDEVICE "/dev/ttyS0"   	//Conexió IGEP - Arduino.
#define MODEMDEVICE "/dev/ttyACM0"      //Conexió directa PC(Linux) - Arduino.
#define _POSIX_SOURCE 1 				// POSIX compliant source.

pthread_mutex_t lock;					//Hablilitem el poder fer Mutex en el tracte de variables de l'array.

bool modemarxa = true;					//Variable booleana que ens indica si l'Arduino està llegint dades.
int fd, i = 0, res, res1, res2;         //Variables usades per saber el nombre de bytes que ens comunica l'Arduino.                                                
char buf[255];							//Array de chars on s'emmagatzema el valor rebut de l'Arduino.
char missatge[255];						//Array de chars que inclou el missatge que s'envia a l'Arduino.
union{
	long int i;
	char	c[4];
}conversio;
int bytes;
float arraycircular[SIZE_OF_BUFFER] = { 0 };	//Definim el tamany del buffer.
int indexLlegir = 0;							//Índex al punter de lectura.
int indexEscriure = 0;							//Índex al punter d'escriptura.
int numeroceldas = 0;							//Nombre de valors en el buffer circular.
float max=0;									//Variable que emmagatzema el valor màxim de temperatura mesurat.
float min=70;									//Variable que emmagatzema el valor mínim de temperatura mesurat.
struct termios oldtio,newtio;   				//Estructura del codi per comunicar amb l'arduino.
int temps = 0;									//Temps de mostratge rebut del client.
int mostres = 0;								//Nombre de mostres per fer la mitjana rebut del client.


/*******************************Prototipus de les funcions***********************************************************/

void Param_Marxa();				//Funció que envia l'ordre de marxa.
void buffer(float); 			//Funció que emmagatzema les temperatures registrades.
int ConfigurarSerie(void);		//Funció que configura el port sèrie.
void TancarSerie(int);			//Funció que tanca el port sèrie.
void demanarmostra(int);		//Funció que demana la mostra llegida per l'Arduino.
void registremax_min(float);	//Funció que emmagatzema els valors màxims i mínims.
int tiempo();					//Funció que compta segons.
void blinkingled();				//Funció que fa que el LED 13 s'encengui i s'apagui cada cop que es realitza una lectura.
/********************************************************************************************************************/




void* codi_fill(void* parametre){ 										// Codi thread fill.

/***************************************************************************** EXECUCIÓ THREAD FILL ********************************************************/

	printf("Thread fill PID(%d), executant-se\n",getpid());
	//sleep(2); // Retard de 2 segons

		Param_Marxa(temps, mostres);							//Aquest és el "main" de la Fita 3.


/***********************************************************************************************************************************************************/

	pthread_exit(NULL);
	return NULL;
}


int main(int argc, char *argv[]){
	pthread_t thread;	
	

/***************************************************************************** EXECUCIÓ SERVIDOR *********************************************************/
		
	printf("Proces pare PID(%d), executant-se\n",getpid());
	//sleep(1); // Retard de 1 segons
		
		struct sockaddr_in	serverAddr;
		struct sockaddr_in	clientAddr;
		unsigned int sockAddrSize;
		int	sFd;
		int	newFd;
		int result;
		char buffer[256];
		char missatge[] = "#(2)(0,23.3)(10,23.3)";
		int bufferlen = 0;
		int desenes;
		int unitats;
		
		/*Preparar l'adreça local*/
		sockAddrSize=sizeof(struct sockaddr_in);
		bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(SERVER_PORT_NUM);
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		/*Crear un socket*/
		sFd=socket(AF_INET, SOCK_STREAM, 0);
		
		/*Nominalitzar el socket*/
		result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
		
		/*Crear una cua per les peticions de connexió*/
		result = listen(sFd, SERVER_MAX_CONNECTIONS);
		
		/*Bucle s'acceptació de connexions*/
		while(1){
			printf("\nServidor esperant connexions\n");

			/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
			newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
			printf("Connexió acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			/*Rebre*/
			memset( buffer, 0, 256 );
			result = read(newFd, buffer, 256);
			bufferlen = strlen(buffer);
			
			printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
				switch (buffer[1]){
					case 'M':
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){															//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));																		//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));																	//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{M1}\n");																				//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																				//Escrivim el que haviem posat al array de char "missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);														//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
							
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							
							switch(buffer[2]){
								case '0':																	//Client vol PARADA
									modemarxa = false;														//Deshabilitem la variable de mode MARXA.
									memset(buffer,0,strlen(buffer));										//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
									memset(missatge,0,strlen(missatge));									//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
									strcpy(missatge,"{M0}\n");												//Escrivim el que volem retornar al client al char "missatge".
									strcpy(buffer,missatge);												//Escrivim el que haviem posat al array de char "missatge" al array "buffer".
									result = write(newFd, buffer, strlen(buffer)+1); 						//+1 per enviar el 0 final de cadena.
									printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
									result = close(newFd);
								break;
								case '1':																									//Client vol MARXA
									desenes = buffer[3]-'0';																				//Convertim els valors de temps rebuts a enters.
									unitats = buffer[4]-'0';
									temps = desenes*10 + unitats;
									mostres = buffer[5]-'0';																				//Convertim el nombre de mostres rebudes a enter.
																	
									if(temps < 1 || temps > 20 || mostres < 1 || mostres > 9){												//Mirem que no hi hagi un ERROR de tipus 2.
										memset(buffer,0,strlen(buffer));																	//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
										memset(missatge,0,strlen(missatge));																//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
										strcpy(missatge,"{M2}\n");																			//Escrivim el missatge d'error que volem retornar al client al char "missatge".
										strcpy(buffer,missatge);																			//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
										result = write(newFd, buffer, strlen(buffer)+1);													//+1 per enviar el 0 final de cadena
										printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
										result = close(newFd);
									} else{
										printf("El temps de mostratge és %d, i el nombre de mostres per la mitjana és %d.\n",temps, mostres);	//Treiem per la cònsola la informació rebuda del client.
										modemarxa = true;																						//Establim que estem en mode 'MARXA'.
										//printf("Proces pare 1 PID(%d) \n",getpid() );
										pthread_create(&thread, NULL, codi_fill, 0); 															//Creem el thread fill.
										
										pthread_mutex_init(&lock, NULL);																		//Inicialitzem el Mutex.
										
										//printf("Proces pare 2 PID(%d) \n",getpid() );	// Proces Pare.
										
										memset(buffer,0,strlen(buffer));																		//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
										memset(missatge,0,strlen(missatge));																	//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
										strcpy(missatge,"{M0}\n");																				//Escrivim el que volem retornar al client al char "missatge".
										strcpy(buffer,missatge);																				//Escrivim el que haviem posat al array de char "missatge" al array "buffer".
										result = write(newFd, buffer, strlen(buffer)+1);														//+1 per enviar el 0 final de cadena.
										printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
										result = close(newFd);
									}
									
								break;
								}
							}
					break;
					case 'U':  																							//Client vol consultar la MOSTRA MÉS ANTIGA.
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){												//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{U1}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							float mostramesantiga = arraycircular[indexLlegir];
							if(indexLlegir < SIZE_OF_BUFFER){
								indexLlegir++;
							}else if(indexLlegir == SIZE_OF_BUFFER){
								indexLlegir = 0;
							}
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							char str[20];
							sprintf(str, "{U0%05.1f}\n",mostramesantiga);												//La mostra més antiga la treiem en format de 5 dígits enters i 1 decimal.
							strcpy(missatge,str);																	
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}
					break;
					case 'X':  																							//Client vol la TEMPERATURA MÀXIMA.
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){												//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{X1}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							char str[20];
							sprintf(str, "{X0%05.1f}\n",max);															//La mostra la treiem en format de 5 dígits enters i 1 decimal.
							strcpy(missatge,str);																		//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}
					break;
					case 'Y': 																							//Client vol la TEMPERATURA MÍNIMA.
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){												//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{Y1}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							char str[20];
							sprintf(str, "{Y0%05.1f}\n",min);															//La mostra més antiga la treiem en format de 5 dígits enters i 1 decimal.
							strcpy(missatge,str);																		//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena
							printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}
					break;
					case 'R':  																							//Client vol fer RESET MÁXIM I MÍNIM.
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){												//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{R1}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							min = 70;
							max = 0;
							strcpy(missatge,"{R0}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}
					break;
					case 'B':  																							//Client vol el NOMBRE DE MOSTRES EMMAGATZEMADES.
						if(buffer[0] != '{' || buffer[bufferlen-1] != '}'){												//Mirem que no hi hagi un ERROR de tipus 1.
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							strcpy(missatge,"{B1}\n");																	//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Error de paràmetres, missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}else if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
							memset(buffer,0,strlen(buffer));															//Posem un '0' a totes per posicions de l'array de chars anomenat "buffer".
							memset(missatge,0,strlen(missatge));														//Posem un '0' a totes per posicions de l'array de chars anomenat "missatge".
							char str[20];
							sprintf(str, "{B0%04d}\n",numeroceldas);													//El nombre de mostres emmagatzemades el treiem en format de 4 nombres.
							strcpy(missatge,str);																		//Escrivim el missatge d'error que volem retornar al client al char "missatge".
							strcpy(buffer,missatge);																	//Escrivim el que haviem posat al array de chars"missatge" al array "buffer".
							result = write(newFd, buffer, strlen(buffer)+1);											//+1 per enviar el 0 final de cadena.
							printf("Missatge enviat a client(bytes %d): %s\n",result, buffer);
							result = close(newFd);
						}
					break;
					default:
						strcpy(buffer, "{E1}\n");																		//En cas de no haver rebut una comanda clara del client, informem d'un error.
						result = write(newFd, buffer, strlen(buffer) + 1); 												//+1 per enviar el 0 final de cadena.
						printf("L'instruccio rebuda no es reconeix\n");
						strcpy(missatge, "Error instrucció\n");
						printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
						result = close(newFd);
						break;
						}

				}
			
			/*Tancar el socket fill*/
			result = close(newFd);
		





/************************************************************************************************************************************************************/

	pthread_join(thread, NULL);
	pthread_mutex_destroy(&lock);																						//Destruïm el Mutex que abans hem inicialitzat.
	//printf("Proces pare PID(%d)\n",getpid());
	return EXIT_SUCCESS;
}



int	ConfigurarSerie(void){
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );                             
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }
	tcgetattr(fd,&oldtio); /* save current port settings */
	bzero(&newtio, sizeof(newtio));                                         
	//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;
	/* set input mode (non-canonical, no echo,...) */                       
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */         
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);	
	sleep(2); //Per donar temps a que l'Arduino es recuperi del RESET	
	return fd;
}               

void TancarSerie(fd){
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}

void Param_Marxa(int tmostr, int mitjana){
 
/************************* CODI COMUNICACIÓ RASPBERRY PI AMB ARDUINO*************************************/	
	fd = ConfigurarSerie();

	sprintf(missatge,"AM1%.2dZ\n",tmostr/2);									//Ordre de marxa, amb temps de mostratge la meitat del entrat per consola.

	res = write(fd,missatge,strlen(missatge));

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

	printf("Enviats %d bytes: ",res);
	
	for (i = 0; i < res; i++){
		printf("%c",missatge[i]);
	}
	
	printf("\n");
	
		
	while(modemarxa==true){				//Sempre que estiguem en mode marxa, cridem la funció "tiempo()", que compta segons.
		int segons = tiempo();			
		if(segons == tmostr){
			demanarmostra(tmostr);		//Un cop la funció "tiempo()" hagi comptat els segons que han estat entrats per l'usuari, cridem la funció que envia l'ordre "ACZ".
		}
	}
/********************************************************************************************************/	
	
}

void buffer(float variableperguardar){
	

	float suma=0;															//Variable que emmagatzema la suma de les "X" temperatures de les quals volem fer la mitjana.

/*******************************************CONFIGURACIÓ BUFFER CIRCULAR ********************************/
	pthread_mutex_lock(&lock);												//Bloquejem amb un Mutex l'accés a la variable de l'array circular que s'està emmagatzemant en aquest moment, així evitem inconsistències de dades.
	arraycircular[indexEscriure]=variableperguardar;
	indexEscriure++;
	if(numeroceldas != SIZE_OF_BUFFER){
		numeroceldas++;
	}
	
	printf("Actualment hi ha %d mostres guardades de 100, mostra guardada és %0.2fºC\n",numeroceldas, arraycircular[indexEscriure-1]);	//Es notifica a l'usuari el nombre de mostres emmagatzemades i quina ha estat la última.
	blinkingled();
	
	if(indexEscriure == SIZE_OF_BUFFER){
		indexEscriure = 0;
	}
	pthread_mutex_unlock(&lock);											//Un cop la variable ja ha estat emmagatzemada al array circular, desbloquejem el Mutex.
/********************************************************************************************************/

		for(int i = indexEscriure -1; i >=(indexEscriure-mostres); i--){
			suma=suma + arraycircular[i];												//Fem la suma de les "X" últimes mostres.
		}
		float mitja = suma/mostres;														//Dividim la variable "suma" entre el nombre de temperatures sumades, fent així la mitjana.
		printf("La mitjana de les últimes %d mostres és %0.2fºC\n", mostres, mitja);	//Treiem la informació per pantalla.



}



void demanarmostra(int temps){

/************************* CODI COMUNICACIÓ RASPBERRY PI AMB ARDUINO*************************************/	
	fd = ConfigurarSerie();

	sprintf(missatge,"ACZ\n");																		//Enviem l'ordre de l'Operació 'C'.

	res1 = write(fd,missatge,strlen(missatge));

	if (res1 <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

		
	sleep(1);
	res1 = read(fd,buf,1); 																			//Cal rebre d'un byte en un byte perquè newtio.c_cc[VMIN] = 1 A ConfigurarSerie(void)
	res1 = res1 + read(fd,buf+1,1);																	//Rebem de l'Arduino 7 bytes.
	res1 = res1 + read(fd,buf+2,1);
	res1 = res1 + read(fd,buf+3,1);
	res1 = res1 + read(fd,buf+4,1);
	res1 = res1 + read(fd,buf+5,1);
	res1 = res1 + read(fd,buf+6,1);

		
		
	int milers = buf[3]-'0';																		//Convertim les dades comunicades de char a integer.
	int centenes = buf[4]-'0';
	int desenes = buf[5]-'0';
	int unitats = buf[6]-'0';
	
	int temperatura = milers*1000+centenes*100+desenes*10+unitats;									//Formem una mateixa dada, que va de 0 a 1023.
	
	if(temperatura<1024 && temperatura>-1){
		float temperaturaengraus = (temperatura*70)/1023;											//Convertim la dada rebuda de l'Arduino a graus centígrads.
		printf("\nHem rebut %d, que correspòn a  %0.2f ºC\n",temperatura, temperaturaengraus);
		buffer(temperaturaengraus);																	//La temperatura, ja convertida a graus centígrads s'envia a la funció "buffer()" per emmagatzemar-la.
		registremax_min(temperaturaengraus);														//La temperatura, ja convertida a graus centígrads s'envia a la funció "registremax_min", per actualitzar, si cal, els regístres de temperatura màxima i mínima.
		TancarSerie(fd);
	}else{
		printf("\nHi ha hagut un error en la comunicació sèrie, ho tornem a intentar, disculpi les molèsties\n");
		demanarmostra(temps);
	}
}

void registremax_min(float temperatura){

	
	if (temperatura>max){							//En cas de ser la nova temperatura superior a la que fins ara havia estat considerada com a màxima, esdevé la nova temperatura màxima.
		max=temperatura;
		printf("El nou màxim és %0.2fºC\n", max);		
	}
	if (temperatura<min){							//En cas de ser la nova temperatura inferior a la que fins ara havia estat considerada com a mínima, esdevé la nova temperatura mínima.
		min=temperatura;
		printf("El nou mínim és %0.2fºC\n", min);
	}
}

int tiempo(){									//Aquesta funció compta segons, està configurada per treure per pantalla el nombre de segons que va comptant, quan arriba al temps estipulat per "tmostr",
										//finalitza la seva execució, permetent així l'execució de la funció "demanarmostra()".
	clock_t t,ts;
	int segundos=0;
	ts=clock()+CLOCKS_PER_SEC;
	for(;;){
		if((t=clock())>=ts){
		  //printf("%d\n",++segundos);
		  segundos++;
		  ts=t+CLOCKS_PER_SEC;
		  if(segundos == temps){
			break;
		  }
		}
	}
	return segundos;
}

void blinkingled(){
	/******************************* FEM QUE EL LED FACI UNA PULSACIÓ ****************************************/
	fd = ConfigurarSerie();

	sprintf(missatge,"AS131Z\n");													//Enviem l'ordre de l'Operació 'S' per encendre el LED 13.

	res1 = write(fd,missatge,strlen(missatge));

	if (res1 <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }


	
	sprintf(missatge,"AS130Z\n");													//Enviem l'ordre de l'Operació 'S' per a que el LED 13 s'apagui.
	res1 = write(fd,missatge,strlen(missatge));
	if (res1 <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
	
	TancarSerie(fd);
/********************************************************************************************************/
}
