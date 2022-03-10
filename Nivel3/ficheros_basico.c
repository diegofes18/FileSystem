
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
//Inicializamos el array de inodos
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

/*
Escribe el valor indicado por el parámetro bit en un determinado bit 
del MB que representa el bloque nbloque
*/
int escribir_bit(unsigned int nbloque, unsigned int bit){
        
    struct superbloque SB;
    //Leemos el superbloque
    if(bread(SBPOS,&SB)==-1){
        return -1;
    }
    //inicializamos las variables
    unsigned int posbyte = nbloque / 8; //posicion del byte en el MB
    unsigned int posbit = nbloque % 8; //posicion del bit dentro de ese byte
    unsigned int nbloqueMB = posbyte / BLOCKSIZE; //Bloque del MB
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posicion absoluta

    unsigned char bufferMB[BLOCKSIZE];
    //leemos el bloque donde esta el bit 
    if(bread(nbloqueabs,bufferMB)== EXIT_FAILURE){
        return -1;
    }
    //posicion del byte relativa al bloque 
    posbyte=posbyte % BLOCKSIZE;

    //mascara y mover bits
    unsigned char mascara=128; //1000000
    mascara>>=posbit; //movemos los bits a la derecha

    //cambiamos la logica de la mascara en funcion de bit de
    if(bit){
       
        bufferMB[posbyte] |=mascara; //OR 
    }
    else{
        bufferMB[posbyte] &=~mascara; // operadores AND y NOT para bits
    }
    //Escribimos el bufferMB en el dispositivo virtual
    if (bwrite(nbloqueabs, bufferMB) == -1){
        return -1;
    }

    return EXIT_SUCCESS;
    
}

/*
Lee un determinado bit del MB y devuelve el valor del bit leído
*/
char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    unsigned char mascara = 128; // 10000000
    //Leemos el superbloque
    if (bread(posSB, &SB) == -1) {
        return -1;
    }   
    unsigned int posbyte = nbloque / 8; //Posición del byte en el MB
    unsigned int posbit = nbloque % 8; //Posición del bit dentro de ese byte
    unsigned int nbloqueMB = posbyte / BLOCKSIZE; // bloque del MB que se halla ese bit para leerlo
    unsigned int nbloqueabs = SN.posPrimerBloqueMB + nbloqueMB //Posición absoluta del dispositivo virtual donde leer el bit

    unsigned char bufferMB[BLOCKSIZE];
    //Leemos el bloque donde esta el bit 
    if(bread(nbloqueabs,bufferMB)== EXIT_FAILURE){
        return -1;
    }
    
    posbyte=posbyte % BLOCKSIZE; //Posicion del byte relativa al bloque 

    mascara >>= posbit;          // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha))
    return mascara
 }
 

 int reservar_bloque(){
    
    struct superbloque SB;
    //Leemos el superbloque
    if(bread(SBPOS,&SB)==-1){
        return -1;
    }
     //Compruebamos si hay bloques libres en el disco.
    if (SB.cantBloquesLibres == 0)
    {
        return EXIT_FAILURE;
    } 
    
 }


/*
Libera un bloque determinado 
*/
int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    //Leemos el superbloque
    if (bread(posSB, &SB) == -1) {
        return -1;
    }

    //Ponemos a 0 el bit del MB correspondiente al bloque nbloque
    escribir_bit(nbloque,0);

    //Incrementamos la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres++;

    //Salvamos el superbloque
    if (bwrite(posSB, &SB) == -1) {
        return -1;
    }
    
    return nbloque;
}

/*
Escribe el contenido de una variable de tipo struct inodo 
en un determinado inodo del array de inodos
*/
int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    //Leemos el superbloque para obtener la localización del array de inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        return -1;
    }

    //Obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado
    bloqueArray=SB.posPrimerBloqueAI+(ninodo/(BLOCKSIZE/INODOSIZE));

    //buffer de lectura de array de inodos
    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    //Leemos el bloque del array de inodos correspondiente
    if (bread(posBloqueInodo, inodos) == -1){
        return -1;
    }

    //Escribimos el inodo en el lugar correspondiente del array
    inodo=inodos[ninodo%(BLOCKSIZE/INODOSIZE)];

    //Escribimos el bloque modificado en el dispositivo virtual
    if (bwrite(nbloqueabs, bufferMB) == -1){
        return -1;
    }

    //Si todo ha ido bien devolvemos 0
    return 0;
    
}
 int leer_inodo(unsigned int ninodo, struct inodo *inodo){
     struct superbloque SB;
     //Leemos el superbloque
    if (bread(0,&SB) == -1) {
    return -1;
    }
    //Declaramos el atributo posinodo i el struct inodo
    //posInodo es la posicion del primer bloque del array de inodo
    //mas en numero de inodopor inodosize, que es lo que ocupa el inodo, divido
    //por el tam del bloque
    unsigned int posInodo = SB.posPrimerBloqueAI + (ninodo * INODOSIZE) / BLOCKSIZE;

 }

/*
Encuentra el primer inodo libre , lo reserva, devuelve su número 
y actualiza la lista enlazada de inodos libres.
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos){

 }






