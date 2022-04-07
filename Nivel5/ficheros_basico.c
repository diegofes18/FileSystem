#include "ficheros_basico.h"

#define DEBUG3 0 //Debugger del nivel 3
#define DEBUG4 1 //Debugger del nivel 4

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
Inicializa el superbloque
*/
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
	return EXIT_SUCCESS;
}

/*
Inicializa el mapa de bits
*/

int initMB(){

    unsigned char buffer[BLOCKSIZE];

    //ponemos el buffer a 0 mediante memset
    if (memset(buffer, 0, BLOCKSIZE) == NULL){
        return -1;
    }

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //int tamMB = SB.posUltimoBloqueMB - SB.posPrimerBloqueMB;

    //inicializamos el mapa de bits
    for (int i = SB.posPrimerBloqueMB; i <=SB.posUltimoBloqueMB; i++){
        if (bwrite(i, buffer) == -1){
            return -1;
        }
    }

    //Ponemos a 1 los bits de los bloques del superbloque, MB y inodos
    for (unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++){
        reservar_bloque();
    }

    return EXIT_SUCCESS;

 }

 /*
Inicializamos el array de inodos
*/
int initAI(){

    unsigned char buffer[BLOCKSIZE];

    //ponemos el buffer a 0 mediante memset
    if (memset(buffer, 0, BLOCKSIZE) == NULL){
        return -1;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //variable para tratar la salida del bucle
    int end = 0;

    //variable para contar la cantidad de inodos libres
    int contador = SB.posPrimerInodoLibre + 1;

    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && end == 0; i++){
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++){

            inodos[j].tipo = LIBRE; 

            if (contador < SB.totInodos){
                inodos[j].punterosDirectos[0] = contador;
                contador++;
            }

            else{
                //al llegar al último nodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                end = 1;
                break;
            }
        }

        //escritura del bloque de inodos en el dispositivo virtual
        if (bwrite(i, &inodos) == -1){
            return -1;
        }
    }

    return EXIT_SUCCESS;
}


/*
Escribe el valor indicado por el parámetro bit en un determinado bit 
del MB que representa el bloque nbloque
*/
int escribir_bit(unsigned int nbloque, unsigned int bit){

    //lectura del superbloque    
    struct superbloque SB;
    if(bread(posSB,&SB)==-1){
        return -1;
    }

    //inicializamos las variables
    unsigned int posbyte = nbloque / 8; //posicion del byte en el MB
    unsigned int posbit = nbloque % 8; //posicion del bit dentro de ese byte
    unsigned int nbloqueMB = posbyte / BLOCKSIZE; //Bloque del MB
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posicion absoluta

    unsigned char bufferMB[BLOCKSIZE];
    
    //leemos el bloque donde esta el bit 
    if(bread(nbloqueabs,bufferMB)== -1){
        return -1;
    }

    //posicion del byte relativa al bloque 
    posbyte=posbyte % BLOCKSIZE;

    //mascara y mover bits
    unsigned char mascara=128; //1000000
    mascara>>=posbit; //movemos los bits a la derecha

    //cambiamos la logica de la mascara en funcion de bit de
    if(bit==1){
       
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

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        return -1;
    } 

    unsigned int posbyte = nbloque / 8; //Posición del byte en el MB
    unsigned int posbit = nbloque % 8; //Posición del bit dentro de ese byte
    unsigned int nbloqueMB = posbyte / BLOCKSIZE; // bloque del MB que se halla ese bit para leerlo
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posición absoluta del dispositivo virtual donde leer el bit

    unsigned char bufferMB[BLOCKSIZE];

    //Leemos el bloque donde esta el bit 
    if(bread(nbloqueabs,bufferMB)== -1){
        return -1;
    }
    
    posbyte=posbyte % BLOCKSIZE; //Posicion del byte relativa al bloque 
    unsigned char mascara = 128;
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha))

    #if DEBUG3
    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
    #endif

    return mascara;
 }

/*
Encuentra el primer bloque libre, consultando el MB,
lo ocupa  y devuelve su posición
*/
int reservar_bloque(){

    //lectura del superbloque  
    struct superbloque SB;
    if(bread(posSB,&SB)==-1){
        return -1;
    }
    
    //Comprobamos si hay bloques libres en el disco.
    if (SB.cantBloquesLibres == 0){
        return -1;
    } 

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    unsigned int posBloqueMB = SB.posPrimerBloqueMB;
    int libre = 0;

    //Inicializamos a 1 el buffer auxiliar
    if (memset(bufferAux, 255, BLOCKSIZE) == NULL){
        return -1;
    }

    //localizamos la posición del primer bloque del MB que tenga algún bit a 0
    while(libre==0){
        if (bread(posBloqueMB, bufferMB) == -1) {
            perror("Error al reservar_bloque()\n");
            return -1;
        }

        int iguales = memcmp(bufferMB, bufferAux, BLOCKSIZE); 

        if (iguales != 0){
            libre = 1;
            break;
        }

        posBloqueMB++;
    }

    //localizamos el byte que contiene el 0 dentro del bloque encontrado anteriormente
    unsigned int posbyte = 0;
    while (bufferMB[posbyte] == 255){
        posbyte++;
    }

    //localizamos el primer bit dentro de ese byte que vale 0
    unsigned char mascara = 128; // 10000000
    unsigned int posbit = 0;
    while (bufferMB[posbyte] & mascara)     {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }

    //Determinar cuál es finalmente el nº de bloque físico que podemos reservar
    unsigned int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    //Indicamos que el bloque está reservado
    if (escribir_bit(nbloque, 1) == -1){
        return -1;
    }

    SB.cantBloquesLibres--;

    // Rellenar el bufffer con 0's
    if (memset(bufferAux, 0, BLOCKSIZE) == NULL){
        perror("Error while memset in reservar_bloque()\n");
        return -1;
    }

    //Escribimos en ese bloque el buffer anterior por si habia información "basura"
    if (bwrite(SB.posPrimerBloqueDatos + nbloque - 1, bufferAux) == -1){
        perror("Error al reservar_bloque()\n");
        return -1;
    }

    //Decrementamos la cantidad de bloques libres en el campo correspondiente del superbloque, 
    //y salvamos el superbloque 
    if (bwrite(posSB, &SB) == -1) {
        return -1;
    }

    return nbloque;
}

/*
Libera un bloque determinado 
*/

int liberar_bloque(unsigned int nbloque){

    //lectura del superbloque
    struct superbloque SB;
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

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //Buffer de lectura de un array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Operación para obtener el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    //Lectura del bloque que contiene el inodo
    if (bread(posBloqueInodo,inodos) == -1){
        return -1;
    }

    // Modifica el buffer con el inodo en el lugar correspondiente.
    inodos[(ninodo % (BLOCKSIZE / INODOSIZE))] = inodo;

    //Escribimos el bloque modificado en el dispositivo virtual
    if (bwrite(posBloqueInodo,inodos) == -1){
        return -1;
    }

    return EXIT_SUCCESS;
    
}

/*
Lee un determinado inodo del array de inodos para volcarlo 
en una variable de tipo struct inodo pasada por referencia.
*/

int leer_inodo(unsigned int ninodo, struct inodo *inodo){

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //Buffer de lectura de un array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Operación para obtener el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    //Lectura del bloque que contiene el inodo
    if (bread(posBloqueInodo,inodos) == -1){
        return -1;
    }

    // Modifica el buffer con el inodo en el lugar correspondiente.
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXIT_SUCCESS;
 }
 

/*
Encuentra el primer inodo libre , lo reserva, devuelve su número 
y actualiza la lista enlazada de inodos libres
*/

int reservar_inodo(unsigned char tipo, unsigned char permisos){

    //lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //miramos que haya bloques libres
    if (SB.cantBloquesLibres == 0){
        return -1;
    }

    //actualizamos la lista enlazada de inodos libres
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;
    SB.cantInodosLibres--;
    

    //variables
    struct inodo inodoAUX;
    inodoAUX.tipo = tipo;
    inodoAUX.permisos = permisos;
    inodoAUX.nlinks = 1;
    inodoAUX.tamEnBytesLog = 0;
    inodoAUX.atime = time(NULL);
    inodoAUX.mtime = time(NULL);
    inodoAUX.ctime = time(NULL);
    inodoAUX.numBloquesOcupados = 0;

    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 3; j++){
            inodoAUX.punterosIndirectos[j] = 0;
        }
        inodoAUX.punterosDirectos[i] = 0;
    }

    //escritura del inodo en la posición del que era el primer inodo libre
    if (escribir_inodo(posInodoReservado, inodoAUX) == -1){
        return -1;
    }

    if (bwrite(posSB, &SB) == -1){
        return -1;
    }

    return posInodoReservado;
}

/*
Se encarga de obtener el rango de punteros en el que se situa el bloque logico que buscamos
y obti ene la di r eccion almacenada en el puntero del inodo
*/ 

int obtener_nRangoBL (struct inodo *inodo , unsigned int nblogico, unsigned int *ptr) {
    if(nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
          
    }else if (nblogico < INDIRECTOS0){
        *ptr=inodo->punterosIndirectos[0];
        return 1;
          
    }else if(nblogico < INDIRECTOS1){
        *ptr=inodo->punterosIndirectos[1];
        return 2;
        
    }else if(nblogico < INDIRECTOS2){
        *ptr=inodo->punterosIndirectos[2];
        return 3;

    }else{
        *ptr=0;
        perror("Error al obtener el nRangoBL");
        return -1;
 
    } 

}

/*
Se encarga de  generalizar la obtencion de los indices de los bloques de punteros
*/

int  obtener_indice (unsigned int nblogico, int nivel_punteros){
    if (nblogico < DIRECTOS) {//ej nblogico=8
        return nblogico;

    }else if(nblogico < INDIRECTOS0) {//ej nblogico=204
        return nblogico - DIRECTOS; 

    }else if(nblogico < INDIRECTOS1){//ej nblogico=30.004 
        if(nivel_punteros == 2){
            return ((nblogico - INDIRECTOS0)/ NPUNTEROS); 
        } else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS0) % NPUNTEROS); 
        }

    }else if(nblogico < INDIRECTOS2){//ej nblogico=400.004 
        if (nivel_punteros == 3){
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));
        }else if (nivel_punteros == 2){
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);
        }else if (nivel_punteros == 1){
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }
    }
    return -1;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{
    struct inodo inodo;
    unsigned int ptr;
    int ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];

    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error in traducir_bloque_inodo(): leer_inodo()\n");
        return -1;
    }

    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           //el nivel_punteros +alto es el que cuelga del inodo

    while (nivel_punteros > 0){ //iterar para cada nivel de indirectos
        if (ptr == 0){ //no cuelgan bloques de punteros
            if (reservar == 0){
                
                return -1;
            }
            else{ //reservar bloques punteros y crear enlaces desde inodo hasta datos

                salvar_inodo = 1;
                ptr = reservar_bloque(); //de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); //fecha actual

                if (nivel_punteros == nRangoBL){
                    //el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUG4
                    printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nRangoBL - 1, ptr, ptr, nivel_punteros);
#endif
                }
                else{
                    buffer[indice] = ptr; 

#if DEBUG4
                    printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nivel_punteros, indice, ptr, ptr, nivel_punteros);
#endif
                    if (bwrite(ptr_ant, buffer) == -1){
                        perror("Error in obtener_nRangoBL: bwrite(ptr_ant, buffer)\n");
                        return -1;
                    }
                } //el bloque cuelga de otro bloque de punteros
            }
        }

        if (bread(ptr, buffer) == -1){
            perror("Error in obtener_nRangoBL: bread(ptr, buffer)\n");
            return -1;
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } 
    //al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0){ //no existe bloque de datos

        if (reservar == 0){ //error lectura ∄ bloque
            return -1;

        }else{

            salvar_inodo = 1;
            ptr = reservar_bloque(); //de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);

            if (nRangoBL == 0){
                inodo.punterosDirectos[nblogico] = ptr;
               
#if DEBUG4
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n",
                       nblogico, ptr, ptr, nblogico);
#endif
            }
            else{
                buffer[indice] = ptr; 
#if DEBUG4
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n",
                       indice, ptr, ptr, nblogico);
#endif

                if (bwrite(ptr_ant, buffer) == -1){
                    fprintf(stderr, "Error in obtener_nRangoBL: bwrite(ptr_ant, buffer)\n");
                    return -1;
                }
            }
        }
    }

    if (salvar_inodo == 1){
        if (escribir_inodo(ninodo, inodo) == -1){
            fprintf(stderr, "Error en salvar inodo: escribir_inodo(ninodo, inodo)\n");
            return -1;
        }
        //sólo si lo hemos actualizado
    }

    return ptr; //nbfisico del bloque de datos
}