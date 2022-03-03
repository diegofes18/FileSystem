
#include "ficheros_basico.h"



int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB; //Definimos la zona de memoria (variable de tipo superbloque)

    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
	
	if(bwrite(posSB,&SB) < 0) return -1; //Error BWRITE	
	return 0;
}

/*
Calcula el tamaño en bloques necesario para el mapa de bits
*/
int tamMB(unsigned int nbloques){

    int tamano=(nbloques / 8) / BLOCKSIZE;

    if(((nbloques / 8) % BLOCKSIZE) != 0){
        tamano=tamano+1;
    }

    return tamano;

}

/*
Calcula el tamaño en bloques del array de inodos
*/
int tamAI(unsigned int ninodos){
    //calculamos espacio
    int tamAI=ninodos / (BLOCKSIZE / INODOSIZE);
    //si no es exacto añadimos uno mas
    if((ninodos*INODOSIZE)%BLOCKSIZE != 0){
        return tamAI+1;
    }
    return tamAI;


}

/*
Inicializa el mapa de bits
*/
int initMB(){

    unsigned char buffer[BLOCKSIZE];

    if(memset(buffer, 0, BLOCKSIZE)==NULL){
        return -1;
    }
    
    struct superbloque SB;

    if(bread(posSB,&SB)==-1){
        return -1;
    }

    for(int i =  SB.posPrimerBloqueMB; i<= SB.posUltimoBloqueMB; i++){
       if(bwrite(i,buffer)==-1){
           return -1;
       }   
    }

}

int initAI(){

    unsigned char buffer[BLOCKSIZE];

    if(memset(buffer, 0, BLOCKSIZE)==NULL){
        return -1;
    }

    struct inodo arrinodos[BLOCKSIZE/INODOSIZE];
    
    struct superbloque SB;

    if(bread(posSB,&SB)==-1){
        return -1;
    }
    int contador=SB.posPrimerInodoLibre+1;
    // Iteramos en todos los bloques del array de inodos.
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Iteramos en cada estructura de inodos.
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {
        // Iniciliza el contenido del inodo.
            arrinodos[j].tipo = 'l';
            if (contador < SB.totInodos)
            {
                arrinodos[j].punterosDirectos[0] = contador;
                contador++;
            }
            else
            {
                arrinodos[j].punterosDirectos[0] = UINT_MAX;
            }
        }
    //Escribimos el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &arrinodos) == -1)
        {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
  
    

}



