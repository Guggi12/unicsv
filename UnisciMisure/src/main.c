/***************************************************************************
RespirAVO/UnisciMisure
Copyright 2022 3C A. Avogadro & RespirAVO Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***************************************************************************/

/******************************************************************************
@data: Marzo 2022
@auhor: Classe 3CI as 2021/22

@scopo:
legge i file misure.csv e posizioni.csv
e produce in output il file out.csv mettendo insieme i dati dei due file,
controllando le vicinanze del timestamp e prendendo le misure
che sono distanti temporalmente  meno di 1 minuto  tra i due file

1) formato del file misure.csv
   - timestamp
   - data
   - NO2 (ppb)
   - VOC (ppb)
   - pm 10 (ug/m3)
   - pm 2.5 (ug/m3)
   + altro che non interessa

2) formato del file posizioni.csv
   - timestamp
   - data
   - latitude
   - longitude

3) formato del file out.csv
   - timestamp
   - latitude
   - longitude
   - NO2 (ppb)
   - VOC (ppb)
   - pm 10 (ug/m3)
   - pm 2.5 (ug/m3)

@compilazione:
WINDOWS: mingw32-make
POSIX:  make

@esecuzione:
WINDOWS: mingw32-make run
POSIX:  make run
******************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <filelib.h>
#include <csvlib.h>
#include <assertlib.h>
#include <siglib.h>

// Crea costante con comando di clear
#ifdef _WIN32
    #define CLEARSTR "cls"
#else
    #define CLEARSTR "clear"
#endif

// Macro per screen clear
#define CLEAR() system(CLEARSTR)

// Scelte
#define NO2 1               // Costante che rappresenta la scelta di NO2
#define VOC 2               // Costante che rappresenta la scelta di VOC
#define PM10 3              // Costante che rappresenta la scelta di PM10
#define PM25 4              // Costante che rappresenta la scelta di PM2.5

// massimi limiti consentiti
#define MAX_NO2 10          // ppb
#define MAX_VOC 10          // ppb
#define MAX_PM10 10         // umg/m3
#define MAX_PM25 10         // umg/m3

#define MAX_SPREAD 60


/************************************************************************************************************
RET TYPE    NAME            ARGUMENTS
************************************************************************************************************/
int         leggi_pos       (FILE *file, unsigned long int *time, float *lat, float *lon);
int         leggi_mis       (FILE *file, unsigned long int *time, float *no2, float *voc, float *pm10, float *pm25);
int         check_limite    (int tipo_mis, float misura, char superato[], int *perc_sup);
void        menu            (int *scelta, char nome_mis[]);
float       get_media       (int arr_mis[], int i);


int main(int argc, char const *argv[])
{
    // Inizializza gestore segnali
    sigSetup();

    // Si assicura che siano passati tutti i tre argomenti richiesti
    massert(4 == argc, -1, "Numero insufficiente di argomenti: Richiesti 3, dati %d", argc - 1);

    // Nomi file
    const char *fp_name = argv[1],   // Nome file posizioni
               *fm_name = argv[2],   // Nome file misure
               *fo_name = argv[3];   // Nome file di output

    // Timestamp
    unsigned long int p_time, // Timestamp posizioni
                      m_time; // Timestamp misure
    
    // Posizioni
    float p_lat,    // Latitudine
          p_lon;    // Longitudine
    
    // Array misure
    float misure[5] = { 0, 0, 0, 0, 0 };

    // Misure
    float *no2  = &misure[NO2],      // Diossido d'azzoto
          *voc  = &misure[VOC],      // Composti organici volatili
          *pm10 = &misure[PM10],     // PM10
          *pm25 = &misure[PM25];     // PM2.5
    
    // Scelta
    int   misura;
    char  mis_name[10];

    // Chiede misura da mettere nel file di output
    menu(&misura, mis_name);

    printf("Inizio elaborazione");
    printf("\n........................\n");

    // Apre file stream verso i file richiesti
    FILE *fp = fileOpenRead(fp_name);  // pointer al file delle posizioni
    FILE *fm = fileOpenRead(fm_name);  // pointer al file delle misure
    FILE *fo = fileOpenWrite(fo_name); // pointer al file di output
    
    // Ignora prima riga dei file di input
    csvIgnoreLine(fp);
    csvIgnoreLine(fm);

    // Determina se è possibile continaure o meno
    bool continuare = true;

    // Fino a quando non si raggiunge la fine di uno dei due file di input
    while (continuare)
    {
        if (p_time < m_time)
            continuare = FILE_OK == leggi_pos(fp, &p_time, &p_lat, &p_lon);
        else
            continuare = FILE_OK == leggi_mis(fm, &m_time, no2, voc, pm10, pm25);
        
        // Differenza tra i timestamp di misure e posizioni
        unsigned long diff = labs(p_time - m_time);
        
        // Se la differenza tra i due timestamp è nella forbice accettabile
        if (diff < MAX_SPREAD)
        {
            printf("%lu %f %f %f %f %f %f\n", p_time, p_lat, p_lon, *no2, *voc, *pm10, *pm25);
            // massert(true, -4, "Errore nella scrittura del file di output '%s'", fo_name);
        }
    }

    fclose(fp);
    fclose(fm);
    fclose(fo);
}

int leggi_pos(FILE *file, unsigned long int *time, float *lat, float *lon)
{
    return csvGetEntries(file, time, "%lu", IGNORE, NULL, lat, "%f", lon, "%f");
}

int leggi_mis(FILE *file, unsigned long int *time, float *no2, float *voc, float *pm10, float *pm25)
{
    return csvGetEntries(file, time, "%lu", IGNORE, NULL, no2, "%f", voc, "%f", pm10, "%f", pm25, "%f", IGNORE, NULL, IGNORE, NULL, IGNORE, NULL, IGNORE, NULL);
}

float get_media(int arr_mis[], int i)
{
    return 0;
}

int check_limite(int tipo_mis, float misura, char superato[], int *perc_sup)
{
    strcpy(superato, "NO");
    
    switch (tipo_mis)
    {
        case NO2:
            if (misura > MAX_NO2)
            {
                strcpy(superato, "SI");
                *perc_sup = roundf((100 * (misura - MAX_NO2) / MAX_NO2));
                //  	       printf("\nMAX=%d,misura = %f,perc sup=%d",MAX_NO2,misura,*perc_sup);
            }
            break;
        case VOC:
            if (misura > MAX_VOC)
            {
                strcpy(superato, "SI");
                *perc_sup = roundf((100 * (misura - MAX_VOC) / MAX_VOC));
            }
            break;
        case PM10:
            if (misura > MAX_PM10)
            {
                strcpy(superato, "SI");
                *perc_sup = roundf((100 * (misura - MAX_PM10) / MAX_PM10));
            }
            break;
        case PM25:
            if (misura > MAX_PM25)
            {
                strcpy(superato, "SI");
                *perc_sup = roundf((100 * (misura - MAX_PM25) / MAX_PM25));
            }
            break;
        default:
            printf("\nmisura non prevista\n");
            return -1;
            break;
    }

    return 0;
}

/**************************************************
menu: vidualizza il menu delle scelte utente
@params: char p[]: nome del file delle posizioni
      char m[]: nome del file delle misure
      char o[]: nome del file di output
      int *scelta : scelta dell'utente
      char nome_mis[]: misura da rilevare
**************************************************/
void menu(int *scelta, char nome_mis[])
{
    bool err = true;

    do
    {
        err = false;
        printf("\nquale misura vuoi elaborare:");
        printf("\n1: NO2");
        printf("\n2: VOC");
        printf("\n3: PM10");
        printf("\n4: PM2.5");
        printf("\nscelta: ");
        scanf("%d", scelta);
        
        switch (*scelta)
        {
            case NO2  : strcpy(nome_mis, "NO2 (ppb)");    break;
            case VOC  : strcpy(nome_mis, "VOC (ppb)");    break;
            case PM10 : strcpy(nome_mis, "PM10 (ug/m3)"); break;
            case PM25 : strcpy(nome_mis, "PM25 (ug/m3)"); break;

            default:
                err = true;
                printf("ERRORE: Opzione %d non valida.\n", *scelta);
                break;
        }
    } while (err);
}
