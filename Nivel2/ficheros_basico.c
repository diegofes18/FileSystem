
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

    int tamano=(nbloques / 8) / BLOCKSIZE; //obtenemos el tamaño

    if(((nbloques / 8) % BLOCKSIZE) != 0){
        //si la división no es exacta añadimos 1 al tamaño
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
    
    if((ninodos*INODOSIZE)%BLOCKSIZE != 0){
        //si no es exacto añadimos uno mas
        return tamAI+1;
    }
    return tamAI;


}

/*
Inicializa el mapa de bits
*/
int initMB(){

    unsigned char buffer[BLOCKSIZE];

    //Ponemos todas las posiciones de buffer a 0 mediante memset
    if(memset(buffer, 0, BLOCKSIZE)==NULL){
        return -1;
    }
    
    //Creamos un superbloque
    struct superbloque SB;

    //Leemos el superbloque creado
    if(bread(posSB,&SB)==-1){
        return -1;
    }

    //Inizializamos cada bloque del Mapa de bits
    for(int i =  SB.posPrimerBloqueMB; i<= SB.posUltimoBloqueMB; i++){
       if(bwrite(i,buffer)==-1){
           return -1;
       }   
    }

}

int initAI(){

    unsigned char buffer[BLOCKSIZE];

    //Ponemos todas las posiciones de buffer a 0 mediante memset
    if(memset(buffer, 0, BLOCKSIZE)==NULL){
        return -1;
    }

    //Creamos un inodo de tamaño BLOCKSIZE/INODOSIZE 
    struct inodo arrinodos[BLOCKSIZE/INODOSIZE];
    
    //Creamos un superbloque
    struct superbloque SB;

    //Leemos el superbloque creado
    if(bread(posSB,&SB)==-1){
        return -1;
    }

    int contador=SB.posPrimerInodoLibre+1;

    // Iteramos en todos los bloques del array de inodos.
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){

        // Iteramos en cada estructura de inodos.
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++){

            // Iniciliza el contenido del inodo.
            arrinodos[j].tipo = 'l';

            if (contador < SB.totInodos){

                arrinodos[j].punterosDirectos[0] = contador;
                contador++;
            }else{
                //Forzar salida al llegar al último nodo
                arrinodos[j].punterosDirectos[0] = UINT_MAX;
            }
        }

        //Escribimos el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &arrinodos) == -1){

            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
  
    

}



