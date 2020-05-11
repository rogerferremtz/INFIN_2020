#define PRINT(a, b) for (int index = 0; index < b; index++) Serial.print(a[index]); Serial.println();     //Instrucció per escriure al port serie la resposta de l'esclau al mestre.

int valor_ADC;                          //Variable on s'emmagatzema l'últim valor de l'ADC.
bool PARADA;                            //Variable booleana que indica si la conversió està en marxa o no (utilitzat a Operació 'C' i al Timer).
int comptador = 0;                      //Variable que compta els segons que van passant (veure Timer).
int mostreig = 200;                     //Valor de temps de mostreig, quan la variable de comptador sigui igual a la de mostreig, es genera una interrupció al Timer i s'inicia la conversió.
unsigned long iniciMillis;              //Variable usada per a fer l'indicador visual de mostreig.
unsigned long actualMillis;             //Variable usada per a fer l'indicador visual de mostreig.
const unsigned long tblink = 400;       //Variable usada per a determinar la durada de l'indicador visual de mostreig.

void setup()
{
//_________________________________________________________________CONFIGURACIÓ USART_____________________________________________________________________________________________________________________________

   Serial.begin(9600);                                //Fixem la velocitat de transmissió a 9600 bits/s, per defecte s'estableixen 8 bits de dades, sense paritat i un bit de stop.
   Serial.setTimeout(50);                             //Fixem el temps màxim que està llegint el port sèrie, 50ms.
//________________________________________________________________________________________________________________________________________________________________________________________________________________

 //_________________________________________________________________CONFIGURACIÓ ADC_______________________________________________________________________________________________________________________________

  /*ADMUX = (1<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (0<<MUX3) | (1<<MUX2) | (0<<MUX1) | (1<<MUX0);    //Posem els bits REFS a [1 1] per així tenir la referència interna de 1.1V, amb la qual obtindrem més resolució (1.1/1024).
                                                                                                     //Fixem l'ADLAR=0, per tenir ADC[7..0] al ADCL i els ADC[9..8] al ADCH.
                                                                                                     //Fixem els bits de MUX a [0101] => A5.
    ADCSRA = ((1 << ADEN)|                            //Posem l'ADC en marxa.
             (0 << ADSC)|                             //Bit START CONVERSION a 0, ja que volem l'AUTO TRIGGER.
             (1 << ADATE)|                            //Habilitem l'AUTO TRIGGERING. L'ADC començarà la conversió quan hi hagi un flanc de pujada a un altre senyal de trigger que es fixarà al ADTS.
             (0 << ADIF)|                             //Aquest bit es posa a 1 quan s'acaba una conversió ADC i s'actualitzen els registres de dades.
             (1 << ADIE)|                             //Si es defineix el bit ADIE a 1 significa que quan l’ADC fa una mesura, es produeix una interrupció, la interrupció despertarà el xip del mode SLEEP.
             (1<<ADPS2)|
             (1<<ADPS1)|
             (1<<ADPS0));                             //Fixem el Prescaler a 128 de manera que tenim un clock de 125Khz per fer les mesures.
                                                      //El rellotge ADC ha d’estar entre 50kHz i 200kHz, ja que el rellotge de l'Arduino Uno és de 16MHz volem dividir-lo per 128 per obtenir 125kHz.
                                                      //L'interrupció ADIF es posa a 1 quan ha acabat conversió.

    ADCSRB = ((1<<ADTS2)|(0<<ADTS1)|(1<<ADTS0));    //Cada cop que el TIMER/COUNTER1 es compari amb el valor del registre B, fa una interrupció, despertem al xip automaticament.
                                                    //No hem d’utilitzar registres per configurar el mode SLEEP ja que utilitzem la biblioteca avr / sleep.h

    //________________________________________________________________________________________________________________________________________________________________________________________________________________
  */

  pinMode(LED_BUILTIN,OUTPUT);                                        //Configurem el pin 13 com a sortida.
  PARADA = HIGH;                                                      //El sistema ha de començar en mode PARADA.

}

void loop()
{
   actualMillis = millis();                                           //El valor actualMillis prendrà el valor en mil.lisegons des de l'inici d'execució del programa (usat en el blink del LED13).
   if (Serial.available())                                            //Quan el port serial estigui disponible, quan hi arribi alguna dada.
   {
      char comanda[20];                                               //Comanda és el missatge que ens envia el mestre.
      size_t count = Serial.readBytesUntil('\n', comanda, 20);        //Comptem els caracters que hi ha a la comanda fins arribar al caracter '\n', serveix per implementar el PRINT.
    
  switch(comanda[1]){                                                 //En funció de quin caracter tenim a la posició 1 del buffer, executem una instrucció o una altra.
        case 'M':
          operaciom(comanda, count);                                  //A la funció de Operació MARXA, li enviem el buffer sencer i la seva longitud.
        break;
        case 'S':
          operacios(comanda, count);                                  //A la funció de Operació SORTIDA, li enviem el buffer sencer i la seva longitud.
        break;
        case 'E':
          operacioe(comanda, count);                                  //A la funció de Operació ENTRADA, li enviem el buffer sencer i la seva longitud.
        break;
        case 'C':
          operacioc(comanda, count);                                  //A la funció de Operació CONVERTIDOR, li enviem el buffer sencer i la seva longitud.
        break;
      }
   }
   check_LED();                                                       //Cridem la funció que controlarà l'intermitència del LED13.                                               
}

void operaciom(char comandam[20], int caracters){     //Funció un cop se'ns ha demanat la instrucció de MARXA/PARA.
    char respostaerror1m[4]={'A','M','1','Z'};        //Definim el array que s'envia al mestre en cas d'error 1.
    char respostaerror2m[4]={'A','M','2','Z'};        //Definim el array que s'envia al mestre en cas d'error 2.
    char respostam[4]={'A','M','0','Z'};              //En cas de ser la comanda correcte, treiem missatge amb codi de retorn '0 = OK'.
    int error = 0;                                    //Variable que s'incrementa cada cop que trobem una segona 'A' abans de la 'Z'.
    int t1 = comandam[3] - '0';                       //Convertim el byte alt del nombre de segons de mostreig a integer.
    int t2 = comandam[4] - '0';                       //Convertim el byte baix del nombre de segons de mostreig a integer.
    mostreig = t1*10 + t2;                            //Formem un sol nombre que és el temps de mostreig.
    
    for (int index = 1; index < caracters; index++){  //Avaluem cada caracter del buffer des de la posició 1, en cas d'haver-hi una altra 'A',
      if( comandam[index] == 'A' ){                   //que no estigui en la posició 1, incrementem la variable error que farà enviar una resposta d'error.
        error = error + 1;
      }
    }

    if(comandam[5] != 'Z' || error > 0){              //Si no hi ha una 'Z' a la posició 5, o hi ha més d'una 'A', enviem error 1.
      PRINT(respostaerror1m,4);
    }else if(mostreig > 20 || mostreig < 1){          //Comprobem que el temps de mostratge rebut estigui entre 1 i 20,
      PRINT(respostaerror2m,4);                       //sino, enviem el missatge d'error 2.
      }else{                                          
        if(comandam[2]=='1'){
          //PRINT(respostam,4);                       //En cas de no haver-hi cap error, enviem la resposta amb codi de error '0 = OK'.
          PARADA = LOW;                               //En cas de posar en marxa la conversió, posem la variable PARADA a LOW, i inicialitzem la variable "comptador" a 0.
          comptador = 0;
          temporitzador();                            //Cridem a la funció que configura el Timer.
          iniciMillis = actualMillis;                 //Iguala el valor inicial amb l'actual
        }else if(comandam[2]=='0'){                   //Si rebem indicació de PARADA, treiem pel monitor sèrie els últims valors llegits.
          if (PARADA == LOW){
            PRINT(respostam,4);                       //En cas de no haver-hi cap error, enviem la resposta amb codi de error '0 = OK'.
            PARADA = HIGH;                            //En cas de aturar la conversió, posem la variable PARADA a HIGH, i treiem pel monitor el valor de la última conversió de l'ADC.
          }else {
            PRINT(respostaerror2m,4);
          }
        }
      }
}

void operacios(char comandas[20], int caracters){
    char respostaerror1s[4]={'A','S','1','Z'};        //Definim el array que s'envia al mestre en cas d'error 1.
    char respostaerror2s[4]={'A','S','2','Z'};        //Definim el array que s'envia al mestre en cas d'error 2.
    char respostas[4]={'A','S','0','Z'};              //En cas de ser la comanda correcte, treiem missatge amb codi de retorn '0 = OK'.
   
    int error = 0;                                    //Variable que s'incrementa cada cop que trobem una segona 'A' abans de la 'Z'.
    int sortida = 0;                                  //Variable en la qual es guardarà el valor del pin de la sortida.
    
    int n1 = comandas[2] - '0';                       //Convertim el byte alt del nombre de sortida a integer.
    int n2 = comandas[3] - '0';                       //Convertim el byte baix del nombre de sortida a integer.
    sortida = n1*10 + n2;                             //Formem un sol nombre que és la sortida digital.
    pinMode(sortida,OUTPUT);                          //Definim el pin digital de la sortida com a OUTPUT.

    for (int index = 1; index < caracters; index++){  //Avaluem cada caracter del buffer des de la posició 1, en cas d'haver-hi una altra 'A',
      if( comandas[index] == 'A' ){                   //que no estigui en la posició 1, incrementem la variable error que farà enviar una resposta d'error.
        error = error + 1;
      }
    }

    if(comandas[5] != 'Z' || error > 0){              //Si no hi ha una 'Z' a la posició 5, o hi ha més d'una 'A', enviem error 1.
      PRINT(respostaerror1s,4);
    }else if(sortida > 13 || sortida < 0){            //Comprobem que la sortida estigui entre 0 i 13, que són els pins d'Arduino
      PRINT(respostaerror2s,4);                       //sinò, enviem el missatge d'error 2.
      }else if(comandas[4]=='1'){                     //Avaluem en cas que el valor de la sortida rebut sigui 1.
          PRINT(respostas,4);
          digitalWrite(sortida,HIGH);                 //Posem a HIGH la sortida que s'ens ha demanat.
        }else if(comandas[4]=='0'){                   //Avaluem en cas que el valor de la sortida rebut sigui 0.
          PRINT(respostas,4);
          digitalWrite(sortida,LOW);                  //Posem a LOW la sortida que s'ens ha demanat.
        }
}

void operacioe(char comandae[20], int caracters){
    char respostaerror1e[5]={'A','E','1','Z'};        //Definim el array que s'envia al mestre en cas d'error 1.
    char respostaerror2e[5]={'A','E','2','Z'};        //Definim el array que s'envia al mestre en cas d'error 2.
    char respostae1[5]={'A','E','0','1','Z'};         //En cas de ser la comanda correcte, treiem missatge amb codi de retorn '0 = OK', s'hi afegeix el valor de l'entrada.
    char respostae0[5]={'A','E','0','0','Z'};         //En cas de ser la comanda correcte, treiem missatge amb codi de retorn '0 = OK', s'hi afegeix el valor de l'entrada.
   
    int error = 0;                                    //Variable que s'incrementa cada cop que trobem una segona 'A' abans de la 'Z'.
    int entrada = 0;                                  //Variable en la qual es guardarà el valor del pin de la entrada.
    
    int n1 = comandae[2] - '0';                       //Convertim el byte alt del nombre de l'entrada a integer.
    int n2 = comandae[3] - '0';                       //Convertim el byte baix del nombre de l'entrada a integer.
    entrada = n1*10 + n2;                             //Formem un sol nombre que és l'entrada digital.
    pinMode(entrada,INPUT);                           //Definim el pin digital de l'entrada com a INPUT.

    for (int index = 1; index < caracters; index++){  //Avaluem cada caracter del buffer des de la posició 1, en cas d'haver-hi una altra 'A',
      if( comandae[index] == 'A' ){                   //que no estigui en la posició 1, incrementem la variable error que farà enviar una resposta d'error.
        error = error + 1;
      }
    }

    if(comandae[4] != 'Z' || error > 0){                //Si no hi ha una 'Z' a la posició 5, o hi ha més d'una 'A', enviem error 1.
      PRINT(respostaerror1e,5);
    }else if(entrada > 13 || entrada < 0){              //Comprobem que la entrada estigui entre 0 i 13, que són els pins d'Arduino, sinò, enviem el missatge d'error 2.
      PRINT(respostaerror2e,4);
      }else if(digitalRead(entrada)== HIGH){            //Avaluem en cas que el valor llegit de l'entrada sigui 1.
              PRINT(respostae1,5);
            }else if(digitalRead(entrada)== LOW){       //Avaluem en cas que el valor llegit de l'entrada sigui 0.
              PRINT(respostae0,5);
            }
}

void operacioc(char comandac[20], int caracters){
    char respostaerror1c[4]={'A','C','1','Z'};         //Definim el array que s'envia al mestre en cas d'error 1.
    char respostaerror2c[4]={'A','C','2','Z'};         //Definim el array que s'envia al mestre en cas d'error 2.
    char adc[4];                                       //Buffer en el qual s'hi guardarà el valor del convertidor en 4 xifres.
    String resposta;
    String AC0 = "AC0";
    String string2 = "0000";
    String Z = String('Z'); 
    
    int error = 0;                                     //Variable que s'incrementa cada cop que trobem una segona 'A' abans de la 'Z'.

    for (int index = 1; index < caracters; index++){   //Avaluem cada caracter del buffer des de la posició 1, en cas d'haver-hi una altra 'A',
      if( comandac[index] == 'A' ){                    //que no estigui en la posició 1, incrementem la variable error que farà enviar una resposta d'error.
        error = error + 1;
      }
    }

    if(comandac[2] != 'Z' || error > 0){               //Si no hi ha una 'Z' a la posició 5, o hi ha més d'una 'A', enviem error 1.
      PRINT(respostaerror1c,4);
    }else if(PARADA == HIGH){                          //Comprobem que la entrada estigui entre 0 i 13, que són els pins d'Arduino, sinò, enviem el missatge d'error 2.
      PRINT(respostaerror2c,4);
    }else if(PARADA == LOW){                           //Avaluem en cas que el valor llegit de l'entrada sigui 1.
            digitalWrite(LED_BUILTIN, HIGH);           //Encenem el LED13, i s'apagarà quan s'arribi al temps establert per la funció check_LED().
            sprintf(adc, "%04d", valor_ADC);           //Emmagatzemem al Buffer "adc" el valor del convertidor expresat en 4 xifres, posant zeros a l'esquerra qual cal.
            string2 = String(adc);                     //Convertim a String el valor de l'ADC.
            resposta = AC0 + string2 + Z ;             //Emmagatzemem en la string resposta el valor de l'ADC, amb el format exigit per l'enunciat.
            Serial.println(resposta);                   
    }
}

void check_LED(){                                                               //Funció per a apagar el LED13 un cop ha sigut activat
    if (actualMillis - iniciMillis > tblink){                                   //En cas que l'interval de temps (temps actual - temps en el que s'ha ences el LED13) superi l'interval definit:
                digitalWrite(LED_BUILTIN, LOW);                                 //Apaga el LED13
                iniciMillis = actualMillis;                                     //Iguala el valor de temps inicial amb l'actual
        } 
}

void temporitzador(){
   //_________________________________________________________________CONFIGURACIÓ TIMER_____________________________________________________________________________________________________________________________
 noInterrupts();                                           //Parem totes les interrupcions abans de configurar el timer.
 TCCR1A = 0;                                               //El registre de control A queda tot a 0.
 TCCR1B = 0;                                               //El registre de control B queda tot a 0.
 TCNT1 = 0;                                                //Inicialitzem el comptador a 0.
 OCR1A = 15625;                                            //Quan el valor del comptador (TCNT1) és igual a OCR1A, s'activa interrupció. Generem una interrupsció cada segon.
 TCCR1B |= (1 << WGM12);                                   //Activem el mode CTC en Timer1. El mode CTC ens permet comptar fins a un valor i reiniciar el comptatge un cop el valor ha estat assolit. 
 TCCR1B |= (1 << CS12)|(0 << CS11)|(1 << CS10);            //Definim el PREESCALER per dividir la freqüència (16MHz) entre 1024, així, fixarem el registre comparador de sortida a (16.000.000/PREESCALER*(1/temps de mostratge)). Generem una interrupció cada segon.
 TIMSK1 |= (1 << OCIE1A);                                  //Registre de configuració d'interrupció. En el nostre cas, el bit OCIEnA que ha d’estar en 1 per indicar al nostre temporitzador que usarem el registre OCRnA per al comparador. 
 interrupts();                                             //Activem les interrupcions novament.
 //________________________________________________________________________________________________________________________________________________________________________________________________________________                            
}

ISR(TIMER1_COMPA_vect){
    comptador = comptador + 1;                     //Com que el Timer genera una interrupció cada segon, les anem comptant, i quan arriba al nombre de segons que volem, convertim i treiem per pantalla.
    if(comptador == mostreig && PARADA == LOW){            //Afegim la condició que es converteixi sempre que estiguem en mode marxa.
      valor_ADC = analogRead(5);                   //Treiem pel port sèrie els valor de la conversió, i tornem a posar el bit a 0.
      //Serial.println(valor_ADC);
      comptador=0;                                 //Tornem a inicialitzar la variable "comptador" a 0.
    }    
}
