/***************************************************************************
                          main.c  -  server
                             -------------------
    begin                : lun feb  4 15:30:41 CET 2002
    copyright            : (C) 2002 by A. Moreno
    copyright            : (C) 2020 by A. Fontquerni
    email                : amoreno@euss.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024


/************************
*
*
* tcpServidor
*
*
*/

int main(int argc, char *argv[])
{
    struct sockaddr_in	serverAddr;
    struct sockaddr_in	clientAddr;
    unsigned int			sockAddrSize;
    int			sFd;
    int			newFd;
    int 		result;
    char		buffer[256];
    char		missatge[] = "Test";
    int         temps = 0;
    char        tempsstr[3];
    char        nummosstr[3];
    int         nummos = 0;
    int         valorTemp = 25000;
    char        valorTempstr[5];
    int         bufferlen = 0;

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
        printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        /*Rebre*/
        memset( buffer, 0, 256 );
        result = read(newFd, buffer, 256);//Siempre que queramos enviar o recibir algo se ha de pasar por el buffer para poder operar con el
        bufferlen = strlen(buffer);

        printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
        if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]){
                case 'M':
                    switch(buffer[2]){
                        case '0':
                            strcpy(buffer,"{M0}\n");

                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                                break;
                        case '1':
                            strcpy(nummosstr,"000");
                            strcpy(tempsstr,"000");
                            tempsstr[0]='0';
                            tempsstr[1]=buffer[3];
                            tempsstr[2]=buffer[4];
                            temps = atoi(tempsstr);//A partir d'aqui es pasarien el valor del temps a una funcio per adquirir les dades
                            nummosstr[0]='0';
                            nummosstr[1]='0';
                            nummosstr[2]=buffer[5];
                            printf("Numero de mostres %s\n",nummosstr);
                            nummos = atoi(nummosstr);
                            strcpy(buffer,"{M0}\n");
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n El temps es: %d i el numero de mostres: %d\n",	result, buffer,temps, nummos);
                                break;
                        default:
                            strcpy(buffer,"{M1}\n");
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);
                                break;
                        }
                        break;
                case 'U':
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",valorTemp);
                    sprintf(valorTempstr,"%d",valorTemp);
                    printf("Valor de la temp: %s\n",valorTempstr);
                    strcpy(missatge,"{U0");
                    strcat(missatge,valorTempstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);

                    break;
                    case 'X':
                        break;
                    case 'Y':
                        break;
                    case 'R':
                        break;
                    case 'B':
                        break;
                    default:
                        break;
                    }

            }
        else if  (buffer[0] != '{' || buffer[bufferlen-1] != '}') {
            strcpy(buffer, "{E2}\n");
            result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
            printf("L'estructura del missatge rebut no es reconeix\n");
            strcpy(missatge, "Error en l'estructura\n");
            printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
        }

        /*Enviar*/




        /*Tancar el socket fill*/
        result = close(newFd);
    }
}


