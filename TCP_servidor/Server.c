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

#define SERVER_PORT_NUM        5001
#define SERVER_MAX_CONNECTIONS    4

#define REQUEST_MSG_SIZE    1024

void clampTofour(char str2Clamp[4]) {//Aquesta funcio fa que un string de menys de 4 caracters es converteixi en un de 4 caracters, posant 0s a l'esquerra
    char tempString[4] = "0000";//Aquesta variable emmagatzema l'string temporalmente
    strcpy(tempString, str2Clamp);//Emmagatzemem la string

    switch (strlen(str2Clamp)) {//Depenent de la longitud seleccionem
        case 1:
            memset(str2Clamp, 0, strlen(str2Clamp));//Buidem la string original
            str2Clamp[0] = '0';//Escrivim 3 zeros
            str2Clamp[1] = '0';
            str2Clamp[2] = '0';
            str2Clamp[3] = tempString[0];//Copiem el caracter a l'string
            break;
        case 2:
            memset(str2Clamp, 0, strlen(str2Clamp));//Buidem la string original
            str2Clamp[0] = '0';//Escrivim 2 zeros
            str2Clamp[1] = '0';
            strcat(str2Clamp, tempString);//Copiem l'string original al final
            break;
        case 3:
            memset(str2Clamp, 0, strlen(str2Clamp));//Buidem la string original
            str2Clamp[0] = '0';//Escrivim 1 zero
            strcat(str2Clamp, tempString);//Copiem l'string original al final
            break;
        case 4://No fem res
            break;
        default:
            //strcpy(str2Clamp,"Ovrf");
            break;
    }
}

int max(int a[], int n) {//Aquesta funcio busca el valor maxim en un array d'enters
    int i, min, max;//S'agafa el valor de la posicio 0 i a partir d'aquest es van comparant els següents
    min = max = a[0];
    for (i = 1; i < n; i++) {

        if (max < a[i])
            max = a[i];
    }
    // printf("maximum of array is : %d\n",max);
    return (max);
}

int min(int a[], int n) {//Aquesta funcio busca el valor minim en un array d'enters, funciona igual que la funcio de max
    int i, min, max;
    min = a[0];
    for (i = 1; i < n; i++) {
        if (min > a[i])
            min = a[i];

    }
    // printf("minimum of array is : %d\n",min);
    return (min);
}


/************************
*
*
* tcpServidor
*
*
*/
int addDecimal(char str2Format[5]) {//Aquesta funcio afegeix el punt decimal a l'string es a dir fa Numero Numero . numero numero, en cas que la longitud sigui tres afegira un zero al principi
    char textBuffer[5];//Aquesta variable emmagatzema temporalment l'string
    if (strlen(str2Format) == 4) {
        textBuffer[0] = str2Format[2];//Guardem els dos valors despres del decimal a la variable temporal
        textBuffer[1] = str2Format[3];
        str2Format[2] = '.';//Escrivim el decimal en la variable original
        str2Format[3] = textBuffer[0];//Tornem els dos valors guardats
        str2Format[4] = textBuffer[1];
    }
    if (strlen(str2Format) == 3) {
        textBuffer[0] = '0';//Escrivim un 0 a la variable temportal
        textBuffer[1] = str2Format[0];//Copiem el valor abans del decimal a la variable temporal
        textBuffer[2] = '.';//Copiem el decimal a la variable temporal
        textBuffer[3] = str2Format[1];//Copiem els ultims valors a la variable temportal
        textBuffer[4] = str2Format[2];
        memset(str2Format, 0, strlen(str2Format));//Buidem la variable original
        strcpy(str2Format, textBuffer);//Copiem la variable temporal a la variable original
    }
}

int lenHelper(unsigned x) {//Aquesta funcio ens serveix per saber la longitud d'un enter, es a dir, quants caracters te
    if (x >= 1000000000) return 10;
    if (x >= 100000000) return 9;
    if (x >= 10000000) return 8;
    if (x >= 1000000) return 7;
    if (x >= 100000) return 6;
    if (x >= 10000) return 5;
    if (x >= 1000) return 4;
    if (x >= 100) return 3;
    if (x >= 10) return 2;
    return 1;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    unsigned int sockAddrSize;
    int sFd;
    int newFd;
    int result;
    char buffer[256];
    char missatge[] = "Test";
    int temps = 0;//Aquesta variable guarda el valor del temps que s'envia amb la funcio marxa, format: enter
    char tempsstr[3];//Aquesta variable guarda el valor del temps que s'envia amb la funcio marxa, format: string
    char nummosstr[3];//Aquesta variable guarda el valor del numero de mostres a fer amb la funcio marxa, format: string
    int nummos = 0;//Aquesta variable guarda el valor del numero de mostres a fer amb la funcio marxa, format: enter
    char valorTempAntstr[5];//Aquesta variable guarda el valor de la temperatura mes antiga, format: string
    int bufferlen = 0;//Aquesta variable guarda el valor de la longitud del buffer, format: enter
    char valorTempMaxstr[5];//Aquesta variable guarda el valor de la temperatura mes alta, format: string
    int tempMax = 0;//Aquesta variable guarda el valor de la temperatura mes alta, format enter
    char valorTempMinstr[5];//Aquesta variable guarda el valor de la temperatura mes baixa, format: string
    int tempMin = 0;//Aquesta variable guarda el valor de la temperatura mes baixa, format: string
    //int         contadorDades = 23;
    char contadorDadesstr[4];
    int i = 0;//Aquesta variable es un comptador
    // int c = 0;//Aquesta variable es un comptador
    // char textbuffer,number[5];
    int temperatures[100];//Aquest array emmagatzema tots els valors de les mostres de temperatura
    int numMostres = 0;//Aquesta variabla guarda el valor del nombre de mostres
    //int cursor = 0;
    int modifier = 0;//Aquesta variable ens indica en quina posicio del array es troba el valor mes antic
    int val = 0;//Aquesta variable guarda el valor de la temperatura (generada aleatoriament)
    time_t t;//Aquesta variable ens serveix per fer servir el temps com a seed per generar valors aleatoris
    // char nummstring[5];
    //int pointer = 0;
    // int numval = 0;
    //int leng = 0;
    //char textbufferstr[5];
    int index = 0;//Aquesta variable ens serveix per saber a partir d'on s'ha de començar a escriure en l'array de temperatures cada vegada que s'activa la marxa
    int overflow = 0;//Aquesta variable ens indica si s'ha arribat a la posició 100 de l'array
    // int cursorOVerflow = 0;


    srand((unsigned) time(&t));//Aquesta funcio fa el seed per la generacio aleatoria de valors

    /*Preparar l'adreça local*/
    sockAddrSize = sizeof(struct sockaddr_in);
    bzero((char *) &serverAddr, sockAddrSize); //Posar l'estructura a zero
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT_NUM);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*Crear un socket*/
    sFd = socket(AF_INET, SOCK_STREAM, 0);

    /*Nominalitzar el socket*/
    result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);

    /*Crear una cua per les peticions de connexió*/
    result = listen(sFd, SERVER_MAX_CONNECTIONS);

    /*Bucle s'acceptació de connexions*/




    while (1) {
        printf("\nServidor esperant connexions\n");

        /*Esperar conexió. sFd: socket pare, newFd: socket fill*/
        newFd = accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
        printf("Connexión acceptada del client: adreça %s, port %d\n", inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        /*Rebre*/
        memset(buffer, 0, 256);//Aquesta posicio posa a 0 les 256 posicions del buffer
        result = read(newFd, buffer,
                      256);//Siempre que queramos enviar o recibir algo se ha de pasar por el buffer para poder operar con el
        bufferlen = strlen(buffer);

        printf("Missatge rebut del client(bytes %d): %s\n", result, buffer);
        if (buffer[0] == '{' &&
            buffer[bufferlen - 1] == '}') {//Comprovem que el primer caracter del buffer compleixi amb la trama
            switch (buffer[1]) {//Amb el segon caracter del buffer determinem quina es la instruccio enviada pel client
                //FUNCIO DE MARXA
                case 'M':
                    switch (buffer[2]) {
                        //PARAR MARXA
                        case '0':
                            memset(buffer, 0, strlen(buffer));//Buidem el buffer
                            memset(missatge, 0, strlen(missatge));//Buidem l'string de missatge
                            strcpy(missatge, "{M0}\n");//Copiem el missatge a l'string de missatge
                            strcpy(buffer, missatge);//Copiem de missatge a buffer
                            result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);//Escrivim el que hem enviat
                            result = close(newFd);//Tanquem la connexio
                            break;
                        //ACTIVAR MARXA
                        case '1':
                            memset(missatge, 0, strlen(missatge));//Buidem el buffer
                            strcpy(nummosstr, "000");//Copiem "000" a l'string del numero de mostres
                            strcpy(tempsstr, "000");//Copiem "000" a l'string del temps de mostreig (Si no es fa aixo no funcionen)
                            tempsstr[0] = '0';
                            tempsstr[1] = buffer[3];//Es copien els valors a l'string del temps de mostreig i s'emmagatzema a la variable apropiada
                            tempsstr[2] = buffer[4];
                            temps = atoi(tempsstr);//Es converteix el valor de temps rebut en un int
                            //A partir d'aqui es pasarien el valor del temps a una funcio per adquirir les dades
                            nummosstr[0] = '0';
                            nummosstr[1] = '0';
                            nummosstr[2] = buffer[5];//Es copia el valor del numero de mostres
                            printf("Numero de mostres %s\n", nummosstr);
                            nummos = atoi(nummosstr);//Es converteix el numero de mostres en un int i s'emmagatzema a la variable apropiada
                            memset(buffer, 0, strlen(buffer));//Es buida el buffer
                            memset(missatge, 0, strlen(missatge));//Es buida la variable de missatge
                            strcpy(missatge, "{M0}\n");//Es copia la resposta a missatge
                            strcpy(buffer, missatge);//Es copia missatge a buffer
                            result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
                            printf("Missatge enviat a client(bytes %d): %s\n El temps es: %d i el numero de mostres: %d\n",result, buffer, temps, nummos);

                            for (i = 0; i < nummos; i++) {//Aixo generara els valors que s'han demanat
                                val = (rand() % 10000);//Es genera un nombre aleatori del 0 al 9999
                                temperatures[(i + index) % 100] = val;//Es guarda el valor generat en la posicio correcta de l'array
                            }
                            index += nummos;//Es guarda el valor del numero de mostres generades per tal de saber a partir d'on s'ha de continuar la propera vegada
                            numMostres = index;//Es guarda el numero de mostres fetes

                            if (overflow == 1 && index >= modifier) {//Aquesta funcio ajusta el cursor de la posicio mes antiga en cas que es sobreescrigui
                                modifier = index;
                            }
                            tempMax = max(temperatures, numMostres);//Es calcula la temperatura maxima
                            tempMin = min(temperatures, numMostres);//Es calcula la temperatura minima
                            result = close(newFd);
                            break;
                        //ERROR
                        default:
                            memset(buffer, 0, strlen(buffer));
                            memset(missatge, 0, strlen(missatge));
                            strcpy(buffer, "{M1}\n");
                            result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                            printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
                            result = close(newFd);
                            break;
                    }
                    break;
                    //FUNCIO DE DADA MES ANTIGA
                case 'U':
                    memset(missatge, 0, strlen(missatge));//Es buida el missatge
                    if (modifier == 100) {//Si la posicio es 100
                        modifier = 0;//La posicio passa a ser 0
                    }
                    if (modifier < numMostres) {//Si la posicio del valor mes antic es mes petit que el numero de mostres
                        printf("%d\n", temperatures[modifier]);//Mostrem el valor de la mostra mes antiga pel terminal(debug)
                        sprintf(valorTempAntstr, "%d", temperatures[modifier]);//Convertim de int a string el valor de la mostra mes antiga i la guardem en la variable apropiada
                        printf("Valor de la temp: %s\n", valorTempAntstr);//Comprovacio que la transformacio ha anat be (debug)
                        strcpy(missatge, "{U0");//Comencem a composar el missatge
                        addDecimal(valorTempAntstr);//Afegim el decimal
                        strcat(missatge, valorTempAntstr);//Afegim el string de la dada mes antiga amb decimal al missatge
                        strcat(missatge, "}\n");//Afegim el final de trama
                        printf("%s\n", missatge);//Comprovem el missatge(Debug)
                        memset(buffer, 0, strlen(buffer));//Buidem el buffer
                        strcpy(buffer, missatge);//Copiem el missatge al buffer
                        result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                        printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                        modifier++;//Incrementem la variable que ens indica on es la dada mes antiga
                    } else if (modifier == numMostres) {//ERROR
                        strcpy(missatge, "{U2}");
                        memset(buffer, 0, strlen(buffer));
                        strcpy(buffer, missatge);
                        result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                        printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    }

                    result = close(newFd);

                    break;
                    //FUNCIO DE TEMPERATURA MAXIMA
                case 'X':
                    memset(missatge, 0, strlen(missatge));//Es buida el missatge
                    printf("%d\n", tempMax);//S'escriu el valor de la temperatura mes alta (debug)
                    sprintf(valorTempMaxstr, "%d", tempMax);//Es transforma el valor de la temperatura mes alta en un string i s'emmagatzema a la variable apropiada
                    printf("Valor de la temp: %s\n", valorTempMaxstr);//Comprovacio que la transformacio ha anat be (debug)
                    strcpy(missatge, "{X0");//Comencem a composar el missatge
                    addDecimal(valorTempMaxstr);//Afegim el decimal
                    strcat(missatge, valorTempMaxstr);//Afegim el valor al missatge
                    strcat(missatge, "}\n");//Afegim el final de trama
                    printf("%s\n", missatge);//Comprovem el missatge (Debug)
                    memset(buffer, 0, strlen(buffer));//Buidem el buffer
                    strcpy(buffer, missatge);//Copiem el missatge al buffer
                    result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    result = close(newFd);
                    break;
                    //FUNCIO DE TEMPERATURA MINIMA
                case 'Y':
                    memset(missatge, 0, strlen(missatge));//Buidem el missatge
                    printf("%d\n", tempMin);//Escrivim el valor de la temperatura minima (debug)
                    sprintf(valorTempMinstr, "%d", tempMin);//Es transforma el valro de la temperatura mes baixa en un string i s'emmagatzema a la variable apropiada
                    printf("Valor de la temp: %s\n", valorTempMinstr);//Comprovacio que la transformacio ha anat be (debug)
                    strcpy(missatge, "{Y0");//Comencem a composar el missatge
                    addDecimal(valorTempMinstr);//Afegim el decimal
                    strcat(missatge, valorTempMinstr);//Afegim el valor al missatge
                    strcat(missatge, "}\n");//Afegim el final de trama
                    printf("%s\n", missatge);//Comprovem el missatge (debug)
                    memset(buffer, 0, strlen(buffer));//Buidem el buffer
                    strcpy(buffer, missatge);//Copiem el missatge al buffer
                    result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    result = close(newFd);
                    break;
                    //FUNCIO DE RESET DE MAXIM I MINIM
                case 'R':
                    memset(missatge, 0, strlen(missatge));//Buidem el missatge
                    strcpy(missatge, "{R0}\n");//Escrivim el missatge
                    memset(buffer, 0, strlen(buffer));//BUidem el buffer
                    strcpy(buffer, missatge);//Copiem el missatge al buffer
                    tempMax = max(temperatures, numMostres);//Calculem la temperatura maxima
                    tempMin = min(temperatures, numMostres);//Calculem la temperatura minima
                    result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena'
                    printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    result = close(newFd);
                    break;
                    //FUNCIO DE NOMBRE DE MOSTRES
                case 'B':
                    if (numMostres < 100) {//Si el numero de mostres es menor que 100
                        sprintf(contadorDadesstr, "%d", (numMostres - modifier));//Convertim el valor del numero de mostres - la posicio on estigui el valor mes antic a string
                    } else if (numMostres - modifier >= 100) {//Si el numero de mostres es major que 100
                        sprintf(contadorDadesstr, "%d", 100);//Escrivim 100 al numero de mostres (format string)
                    }
                    clampTofour(contadorDadesstr);//Convertim el numero de mostres a 4 caracters (afegint 0s al principi)
                    printf("valor: %s \n", contadorDadesstr);//Comprovem el missatge (Debug)
                    memset(missatge, 0, strlen(missatge));//Buidem el missatge
                    strcpy(missatge, "{B0");//Comencem a composar el missatge
                    missatge[3] = contadorDadesstr[0];//Copiem el valor al missatge
                    missatge[4] = contadorDadesstr[1];
                    missatge[5] = contadorDadesstr[2];
                    missatge[6] = contadorDadesstr[3];
                    missatge[7] = '}';//Afegim el final de trama
                    missatge[8] = '\n';
                    //strcat(missatge,contadorDadesstr);
                    //strcat(missatge,"}\n");
                    printf("Missatge %s\n", missatge);//Comprovem el missatge (debug)
                    memset(buffer, 0, strlen(buffer));//Buidem el missatge
                    strcpy(buffer, missatge);//Copiem el missatge al buffer
                    result = write(newFd, buffer, strlen(buffer) + 1);
                    printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                    result = close(newFd);
                    break;
                    //ERRORS
                default:
                    strcpy(buffer, "{E1}\n");
                    result = write(newFd, buffer, strlen(buffer) + 1); //+1 per enviar el 0 final de cadena
                    printf("L'instruccio rebuda no es reconeix\n");
                    strcpy(missatge, "Error en l'instruccio\n");
                    printf("Missatge enviat a client(bytes %d): %s\n", result, missatge);
                    result = close(newFd);
                    break;
            }
        //ERROR DE FORMAT
        } else if (buffer[0] != '{' || buffer[bufferlen - 1] != '}') {
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


