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
void clampTofour(char str2Clamp[4]){
    char tempString[4];
    switch(strlen(str2Clamp)){
        case 1:
            strcpy(tempString,str2Clamp);
            str2Clamp[0] ='0';
            str2Clamp[1] ='0';
            str2Clamp[2] ='0';
            strcat(str2Clamp,tempString);
            break;
        case 2:
            strcpy(tempString,str2Clamp);
            str2Clamp[0] ='0';
            str2Clamp[1] ='0';
            strcat(str2Clamp,tempString);
            break;
        case 3:
            strcpy(tempString,str2Clamp);
            str2Clamp[0] ='0';
            strcat(str2Clamp,tempString);
            break;
        case 4:
            break;
        default:
            strcpy(str2Clamp,"Ovrf");
            break;

    }
}

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
    int         valorTempAnt = 25000;
    char        valorTempAntstr[5];
    int         bufferlen = 0;
    char        valorTempMaxstr[5];
    int         valorTempMax = 30500;
    char        valorTempMinstr[5];
    int         valorTempMin = 19700;
    int         contadorDades = 23;
    char        contadorDadesstr[4];
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
                            memset(buffer,0,strlen(buffer));
                            memset(missatge,0,strlen(missatge));
                            strcpy(missatge,"{M0}\n");
                            strcpy(buffer,missatge);
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                            result = close(newFd);
                                break;
                        case '1':
                            memset(missatge,0,strlen(missatge));
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
                            memset(buffer,0,strlen(buffer));
                            memset(missatge,0,strlen(missatge));
                            strcpy(missatge,"{M0}\n");
                            strcpy(buffer,missatge);
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n El temps es: %d i el numero de mostres: %d\n",	result, buffer,temps, nummos);
                            result = close(newFd);
                                break;
                        default:
                            memset(buffer,0,strlen(buffer));
                            memset(missatge,0,strlen(missatge));
                            strcpy(buffer,"{M1}\n");
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);
                            result = close(newFd);
                                break;
                        }
                        break;
                case 'U':
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",valorTempAnt);
                    sprintf(valorTempAntstr,"%d",valorTempAnt);
                    printf("Valor de la temp: %s\n",valorTempAntstr);
                    strcpy(missatge,"{U0");
                    strcat(missatge,valorTempAntstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);

                    break;
                case 'X':
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",valorTempMax);
                    sprintf(valorTempMaxstr,"%d",valorTempMax);
                    printf("Valor de la temp: %s\n",valorTempMaxstr);
                    strcpy(missatge,"{X0");
                    strcat(missatge,valorTempMaxstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                    break;
                case 'Y':
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",valorTempMin);
                    sprintf(valorTempMinstr,"%d",valorTempMin);
                    printf("Valor de la temp: %s\n",valorTempMinstr);
                    strcpy(missatge,"{Y0");
                    strcat(missatge,valorTempMinstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                     break;
                case 'R':
                    memset(missatge,0,strlen(missatge));
                    strcpy(missatge,"{R0}\n");
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                    break;
                case 'B':
                    sprintf(contadorDadesstr,"%d",contadorDades);
                    clampTofour(contadorDadesstr);
                    printf("valor: %s \n",contadorDadesstr);
                   // printf("String: %s\tNum %d\t longitud string %d\n",contadorDadesstr,contadorDades,strlen(contadorDadesstr));
                    memset(missatge,0,strlen(missatge));
                    strcpy(missatge,"{B0");
                    missatge[3] = contadorDadesstr[0];
                    missatge[4] = contadorDadesstr[1];
                    missatge[5] = contadorDadesstr[2];
                    missatge[6] = contadorDadesstr[3];
                    missatge[7] = '}';
                    missatge[8] = '\n';
                    printf("Missatge %s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1);
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                     break;
                default:
                    strcpy(buffer, "{E1}\n");
                    result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
                    printf("L'instruccio rebuda no es reconeix\n");
                    strcpy(missatge, "Error en l'instruccio\n");
                    printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
                    result = close(newFd);
                    break;
                    }

            }
        else if  (buffer[0] != '{' || buffer[bufferlen-1] != '}') {
            strcpy(buffer, "{E2}\n");
            result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
            //printf("L'estructura del missatge rebut no es reconeix\n");
            //strcpy(missatge, "Error en l'estructura\n");
            printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
            result = close(newFd);
        }

        /*Enviar*/




        /*Tancar el socket fill*/
        result = close(newFd);
    }
}


