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
#include <time.h>


#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4
#define REQUEST_MSG_SIZE	1024





void clampTofour(char str2Clamp[4]){  // Fem que el string sigui de 4
    char tempString[4]="0000";
    strcpy(tempString,str2Clamp);

    switch(strlen(str2Clamp)){
        case 1:
            memset(str2Clamp,0,strlen(str2Clamp));
            str2Clamp[0] ='0';
            str2Clamp[1] ='0';
            str2Clamp[2] ='0';
            str2Clamp[3]=tempString[0];
        break;
        case 2:
            memset(str2Clamp,0,strlen(str2Clamp));
            str2Clamp[0] ='0';
            str2Clamp[1] ='0';
            strcat(str2Clamp,tempString);
        break;
        case 3:
            memset(str2Clamp,0,strlen(str2Clamp));
            str2Clamp[0] ='0';
            strcat(str2Clamp,tempString);
        break;
        case 4:
        break;
        default:
            //strcpy(str2Clamp,"Ovrf");
        break;
    }
}




int max(int a[],int n){ //Funció que ens serveix per a trobar el valor màxim dins de tot l'array de temperatures emmagatzemades
    int i,min,max;
    min=max=a[0];
    for(i=1; i<n; i++){
        if(max<a[i])
         max=a[i];
    }
    return(max);
}

int min(int a[],int n){ //Funció que ens serveix per a trobar el valor mínim dins de tot l'array de temperatures emmagatzemades
    int i,min,max;
    min=a[0];
    for(i=1; i<n; i++){
        if(min>a[i])
        min=a[i];
    }
    return(min);
}


/************************
*
*
* tcpServidor
*
*
**************************/
int addDecimal(char str2Format[5]){  //Funció per poder posar el punt decimal
    char textBuffer[5];
    if(strlen(str2Format) == 4){
        textBuffer[0] = str2Format[2];
        textBuffer[1] = str2Format[3];
        str2Format[2] = '.';
        str2Format[3] = textBuffer[0];
        str2Format[4] = textBuffer[1];
    }
    if(strlen(str2Format) == 3){
        textBuffer[0] = '0';
        textBuffer[1] = str2Format[0];
        textBuffer[2] = '.';
        textBuffer[3] = str2Format[1];
        textBuffer[4] = str2Format[2];
        memset(str2Format,0,strlen(str2Format));
        strcpy(str2Format,textBuffer);
    }
}

int lenHelper(unsigned x) {			//Aquesta funció, ens permet saber la longitut de les cadenes
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

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
    char        valorTempAntstr[5];
    int         bufferlen = 0;
    char        valorTempMaxstr[5];
    int         tempMax = 0;
    char        valorTempMinstr[5];
    int         tempMin = 0;
    int         contadorDades = 23;
    char        contadorDadesstr[4];
    int 		i = 0;
    int 		c = 0;
    char 		textbuffer,number[5];
    int 		temperatures[100];
    int 		numMostres = 0;
    int 		cursor = 0;
    int 		modifier = 0; //Aquesta variable ens serveix per decrementar-la cada cop que es demana la mostra més antiga
    int 		val = 0;
    time_t t;
    char 		nummstring[5];
    int 		pointer = 0;
    int 		numval = 0;
    int 		leng = 0;
    char		textbufferstr[5];
    int 		index = 0;
    int 		overflow = 0;
    int 		cursorOVerflow = 0;
    
    
    srand((unsigned) time(&t));
    
    
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
        result = read(newFd, buffer, 256);//Sempre que vulguem enviar o rebre alguna cosa s'ha de passar pel buffer per poder operar amb ella
        bufferlen = strlen(buffer);

        printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
		if(buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]){
                case 'M':
                    switch(buffer[2]){
						case '0':			//Client vol PARADA
                            memset(buffer,0,strlen(buffer));
                            memset(missatge,0,strlen(missatge));
                            strcpy(missatge,"{M0}\n");
                            strcpy(buffer,missatge);
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                            result = close(newFd);
						break;
						case '1':			//Client vol MARXA

                            memset(missatge,0,strlen(missatge));
                            strcpy(nummosstr,"000");
                            strcpy(tempsstr,"000");
                            tempsstr[0]='0';
                            tempsstr[1]=buffer[3];
                            tempsstr[2]=buffer[4];
                            temps = atoi(tempsstr);//A partir d'aquí es pasarien el valor del temps a una funció per adquirir les dades
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

                            for( i = 0 ; i < nummos ; i++ ) {   //Bucle "for" que ens serveix, de moment, per introduïr valors, en aquest cas aleatoris, al buffer
                                val = (rand() % 10000);
                                temperatures[(i+index)%100] = val;
                            }

                            index += nummos;
                                numMostres = index;

                            if (overflow == 1 && index >= modifier){
                                modifier = index;
                            }
                            tempMax = max(temperatures,numMostres);
                            tempMin = min(temperatures,numMostres);
                            result = close(newFd);
                        break;
						default:
                            memset(buffer,0,strlen(buffer));
                            memset(missatge,0,strlen(missatge));
                            strcpy(buffer,"{M1}\n");
                            result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);
                            result = close(newFd);
                        break;
                        }
				break;
                case 'U':  //Client vol MOSTRA MÉS ANTIGA
                    memset(missatge,0,strlen(missatge));
                    if (modifier == 100){
                        modifier = 0;
                    }
                    if(modifier < numMostres) {
                        printf("%d\n", temperatures[modifier]);
                        sprintf(valorTempAntstr, "%d", temperatures[modifier]);
                        printf("Valor de la temp: %s\n", valorTempAntstr);
                        strcpy(missatge, "{U0");
                        addDecimal(valorTempAntstr);
                        strcat(missatge, valorTempAntstr);
                        strcat(missatge, "}\n");
                        printf("%s\n", missatge);
                        memset(buffer, 0, strlen(buffer));
                        strcpy(buffer, missatge);
                        result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                        printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                        modifier++;
                    }
                    else if(modifier == numMostres){
                        strcpy(missatge,"{U2}");
                        memset(buffer,0,strlen(buffer));
                        strcpy(buffer,missatge);
                        result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                        printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    }

                    result = close(newFd);
				break;
				case 'X':  //Client vol la TEMPERATURA MÀXIMA
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",tempMax);
                    sprintf(valorTempMaxstr,"%d",tempMax);
                    printf("Valor de la temp: %s\n",valorTempMaxstr);
                    strcpy(missatge,"{X0");
                    addDecimal(valorTempMaxstr);
                   // clampTofive(valorTempAntstr);
                    strcat(missatge,valorTempMaxstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                break;
                case 'Y': //Client vol la TEMPERATURA MÍNIMA
                    memset(missatge,0,strlen(missatge));
                    printf("%d\n",tempMin);
                    sprintf(valorTempMinstr,"%d",tempMin);
                    printf("Valor de la temp: %s\n",valorTempMinstr);
                    strcpy(missatge,"{Y0");
                    addDecimal(valorTempMinstr);
                    //clampTofive(valorTempAntstr);
                    strcat(missatge,valorTempMinstr);
                    strcat(missatge,"}\n");
                    printf("%s\n",missatge);
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                break;
                case 'R':  //Client vol fer RESET MÁXIM I MÍNIM
                    memset(missatge,0,strlen(missatge));
                    strcpy(missatge,"{R0}\n");
                    memset(buffer,0,strlen(buffer));
                    strcpy(buffer,missatge);
                    tempMax = max(temperatures,numMostres);
                    tempMin = min(temperatures,numMostres);
                    result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
                    result = close(newFd);
                break;
                case 'B':  //Client vol el NOMBRE DE MOSTRES EMMAGATZEMADES
                    if(numMostres < 100){
                        sprintf(contadorDadesstr,"%d",(numMostres-modifier));
                    }
                    else if(numMostres-modifier >= 100){
                        sprintf(contadorDadesstr,"%d",100);
                    }
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

                   //strcat(missatge,contadorDadesstr);
                   //strcat(missatge,"}\n");
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
