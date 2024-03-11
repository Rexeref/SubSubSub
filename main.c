#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif // WIN32

/*
Cose da fare:
    ALGORITMO
    Ordinati per dimensione, dividendo lo spazio in spazi sempre pi� piccoli si pu� semplificare la quantit�
    di passaggi dovuti per calcolare i primi ip delle subnet

    -----------------------------------------------------------------
    |  512               |  512               |  256     |64|64|    |
    -----------------------------------------------------------------
    In questo caso in 3 slot da 512 (1536) ho messo 1408 host di 5 sottoreti diverse senza per� lasciare spazi inutilizzati fra gli host.
*/

typedef struct
{
    int element1;
    int element2;
    int element3;
    int element4;
    int cidr; /// ATTENZIONE: LE MASCHERE DI RETE VENGONO SEMPLIFICATE CON LA NOTAZIONE CIDR PER COMODITA' DI CALCOLO
} ip;

typedef struct
{
    ip nameIp;
    char name[16];
} net;

//
/// >Prototipi Funzioni
//

void printIp(ip input); // printa l'ip dato formattato
void printNet(net input); // printa la rete data formattata
int numberIsInRange(int input); // Controlla se il numero � nel range che parte da 0 e finisce con 255, se non valido outputta -1
ip fillIpIfValid(int element1, int element2, int element3, int element4); // Valida e riempie uno spazio per ip calcolando in automatico il cidr
ip fillIpIfValidWithCidr(int element1, int element2, int element3, int element4, int cidr); // stessa cosa ma il cidr � manuale
int whatCidrMaskContains(int input); // calcola la maschera di rete minima per contenere il numero di host indicati in una singola rete
int calculateCidrClassfull(int element1); // calcola il cidr dato il primo elemento di un ip usando il classfull
net prepareSubNet(int numberOfElements, char name[16]); // Inizializza i dati della subnetmask
void swap(net *a, net *b);
void selectionSort(net array[], int size);
void printArray(net array[], int size);

ip nextIp(ip input);
ip lastIp(ip input);
ip unsignedLongToIp(unsigned long ipAddress);
unsigned long ipToUnsignedLong(ip n);

//
///  >Variabili Globali
//

ip originIp; // IP DI ROOT
int dimensioneArrayDinamico = 0;

// memoria dinamica gestita con malloc di spazi di memoria di dimensione ip
ip *derivatedIp;
net *subNetworks;

//
/// >Funzioni
//

int main()
{
    #ifdef _WIN32
    int showFlag=0;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
    #endif // _WIN32

    //
    // INSERIMENTO DATI

    printf("INSERISCI IP ROOT (formato a.b.c.d):\n> \e[0;96m");
    int appoggio[4];
    scanf("%d.%d.%d.%d", &appoggio[0], &appoggio[1], &appoggio[2], &appoggio[3]);
    fflush(stdin);printf("\e[0;93m");
    originIp = fillIpIfValid(appoggio[0], appoggio[1], appoggio[2], appoggio[3]);
    printIp(originIp);
    printf("\e[0m\n");

    dimensioneArrayDinamico = 0;
    subNetworks = malloc(dimensioneArrayDinamico*sizeof(net));
    char inputStr[16];
    int inputInt;
    net *appoggioSubNetworks;
    appoggioSubNetworks = malloc(dimensioneArrayDinamico*sizeof(net));
    while(strcmp(inputStr, "ok") != 0)
    {
        printf("\e[0mInserisci nome Virtual Lan (inserisci \"\e[0;31mok\e[0m\" per concludere l'inserimento):\n> \e[0;96m");
        scanf("%[^\n]%*c", &inputStr);
        fflush(stdin);printf("\e[0;93m");
        if(strcmp(inputStr, "ok") != 0)
        {
            printf("\e[0mInserisci numero host richiesti:\n> \e[0;96m");
            scanf("%d", &inputInt);
            fflush(stdin);
            printf("\e[0;93m");

            free(appoggioSubNetworks);
            appoggioSubNetworks = malloc(dimensioneArrayDinamico*sizeof(net));
            for(int i = 0; i < dimensioneArrayDinamico; i++)
            {
                appoggioSubNetworks[i] = subNetworks[i];
            }
            free(subNetworks);
            dimensioneArrayDinamico++;
            subNetworks = malloc(dimensioneArrayDinamico*sizeof(net));
            for(int i = 0; i < dimensioneArrayDinamico - 1; i++)
            {
                subNetworks[i] = appoggioSubNetworks[i];
            }
            subNetworks[dimensioneArrayDinamico - 1] = prepareSubNet(inputInt, inputStr);
        }
    }

    printf("\n\n\n\n\n\n");

    //
    // ALGORITMO

    #ifdef WIN32
    system("cls");
    #endif // WIN32

    printf("Base Network: \e[1;96m");
    printIp(originIp);
    printf("\e[0m\n\n");
    selectionSort(subNetworks, dimensioneArrayDinamico);

    int appoggioCidr = subNetworks[0].nameIp.cidr;
    subNetworks[0].nameIp = originIp;
    subNetworks[0].nameIp.cidr = appoggioCidr;

    for(int i = 1; i < dimensioneArrayDinamico; i++)
    {
        appoggioCidr = subNetworks[i].nameIp.cidr;
        subNetworks[i].nameIp = nextIp(subNetworks[i - 1].nameIp);
        subNetworks[i].nameIp.cidr = appoggioCidr;
    }

    printArray(subNetworks, dimensioneArrayDinamico);

    #ifdef WIN32
    system("PAUSE");
    #endif // WIN32

    return 0;
}

int numberIsInRange(int input)
{
    int output = -1;
    if(input >= 0 && input <= 255)
    {
        output = input;
    }
    return output;
}

ip fillIpIfValid(int element1, int element2, int element3, int element4)
{
    ip output = {-999, 104, 69, 420, -1};
    if(numberIsInRange(element1) != -1 && numberIsInRange(element2) != -1 && numberIsInRange(element3) != -1 && numberIsInRange(element4) != -1 )
    {
        output.element1 = element1;
        output.element2 = element2;
        output.element3 = element3;
        output.element4 = element4;
        output.cidr = calculateCidrClassfull(element1);
    }
    return output;
}

ip fillIpIfValidWithCidr(int element1, int element2, int element3, int element4, int cidr)
{
    ip output = {-999, 104, 69, 420, -1};
    if(numberIsInRange(element1) != -1 && numberIsInRange(element2) != -1 && numberIsInRange(element3) != -1 && numberIsInRange(element4) != -1 )
    {
        output.element1 = element1;
        output.element2 = element2;
        output.element3 = element3;
        output.element4 = element4;
        output.cidr = cidr;
    }
    return output;
}

void printIp(ip input)
{
    if(input.element1 == -999)
    {
        printf("E' stato rilevato un ip non valido, chiusura del programma . . . \nCODICE ERRORE [0x1]\n");
        exit(1);
    }
    printf("%-3d.%-3d.%-3d.%-3d", input.element1, input.element2, input.element3, input.element4);
    if(input.cidr != -1)
    {
        printf("/%d", input.cidr);
    }
}

void printNet(net input)
{
    printf(" \e[1;96m%-16s\e[0m from    \e[1;92m", input.name);
    printIp( input.nameIp );
    printf("\e[0m    to    \e[1;95m");
    printIp( lastIp(input.nameIp) );
    printf("\e[0m;\n");
}

int whatCidrMaskContains(int input)
{
    int output, contains = 0, modifiedInput = input + 3; // MODIFIED INPUT � il valore di input tenendo conto del fatto della presenza degli indirizzi obbligatori (Gateway, name e broadcast)
    for(output = 0; output <= 32 && contains == 0; output++)
    {
        if( pow( (double) 2, (double) output) >= modifiedInput )
        {
            contains = 1;
        }
    }
    return 33 - output;
}

int calculateCidrClassfull(int element1)
{
    int output = -1;
    if(element1 >= 1 && element1 <= 127)
    {
        output = 8;
    }
    else if(element1 >= 128 && element1 <= 191)
    {
        output = 16;
    }
    else if(element1 >= 192 && element1 <= 223)
    {
        output = 24;
    }
    return output;
}

net prepareSubNet(int numberOfElements, char name[])
{
    printf("%-16s > %-5d > ", name, numberOfElements);
    if(whatCidrMaskContains(numberOfElements) > originIp.cidr)
    {
        net output = {fillIpIfValidWithCidr(255, 255, 255, 255, whatCidrMaskContains(numberOfElements)), "."};
        strcpy(output.name, name);
        printf("CIDR = %-2d;\n", output.nameIp.cidr);
        return output;
    }
    printf("Impossibile creare sottorete con CIDR %d, troppi host richiesti . . .\nCODICE ERRORE [0x2]\n", whatCidrMaskContains(numberOfElements));
    exit(2);
}

// function to swap the the position of two elements
void swap(net *a, net *b)
{
    net temp = *a;
    *a = *b;
    *b = temp;
}

void selectionSort(net array[], int size)
{
    for (int step = 0; step < size - 1; step++)
    {
        int min_idx = step;
        for (int i = step + 1; i < size; i++)
        {

            // To sort in descending order, change > to < in this line.
            // Select the minimum element in each loop.
            if (array[i].nameIp.cidr < array[min_idx].nameIp.cidr)
                min_idx = i;
        }

        // put min at the correct position
        swap(&array[min_idx], &array[step]);
    }
}

// function to print an array
void printArray(net array[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        printNet(array[i]);
    }
    printf("\n");
}

unsigned long ipToUnsignedLong(ip n)
{
    unsigned long number = n.element1 * pow( (double) 2, (double) 24) + n.element2 * pow( (double) 2, (double) 16) + n.element3 * pow( (double) 2, (double) 8) + n.element4;
    //printf("\nNUMBER %lu\n", number);
    return number;
}

ip unsignedLongToIp(unsigned long ipAddress)
{
    ip output;
    output.element4 = ( ipAddress >> (0) ) & 0xFF;
    output.element3 = ( ipAddress >> (8) ) & 0xFF;
    output.element2 = ( ipAddress >> (16) ) & 0xFF;
    output.element1 = ( ipAddress >> (24) ) & 0xFF;
    return output;
}

ip nextIp(ip input)
{
    //printf("\nMOVE %d\n", (int) pow(2, (32 - input.cidr)));
    return unsignedLongToIp(ipToUnsignedLong(input) + (int) pow( (double) 2, (double) (32 - input.cidr) ));
}

ip lastIp(ip input)
{
    ip output = unsignedLongToIp(ipToUnsignedLong(input) - 1 + (int) pow( (double) 2, (double) (32 - input.cidr)));
    output.cidr = input.cidr;
    return output;
}
