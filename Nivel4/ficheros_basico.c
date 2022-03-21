
#include "ficheros_basico.h"

#define DEBUG3 1

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

    //Ponemos a 1 en el MB los bits que corresponden a los bloques que ocupa el
    //superbloque, el propio MB, y el array de inodos.
    for (unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++){
        reservar_bloque();
    }
    //Restamos el tamaño de los metadatos a la cantidad de bloques libres
    SB.cantBloquesLibres -= 1 + tamAI(SB.totInodos) + tamMB(SB.totBloques);
    if(bwrite(posSB,&SB)==-1){
        return -1;
    }

    return EXIT_SUCCESS;

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

    int fin = 0;
    int contador=SB.posPrimerInodoLibre+1;
    // Iteramos en todos los bloques del array de inodos.
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI)&&(fin==0); i++)
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
                fin=1;
                break;
            }
        }

        //Escribimos el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &arrinodos) == -1){
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
        
    struct superbloque SB;
    //Leemos el superbloque
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
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; //Posición absoluta del dispositivo virtual donde leer el bit

    unsigned char bufferMB[BLOCKSIZE];

    //Leemos el bloque donde esta el bit 
    if(bread(nbloqueabs,bufferMB)== -1){
        return -1;
    }
    
    posbyte=posbyte % BLOCKSIZE; //Posicion del byte relativa al bloque 

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
    
    struct superbloque SB;
    //Leemos el superbloque
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

    //Localizamos qué byte dentro de ese bloque tiene algún 0
    while(libre==0){
        if (bread(posBloqueMB, bufferMB) == EXIT_FAILURE) {
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

    //Localizamos el byte que contiene el 0 dentro del bloque encontrado anteriormente
    unsigned int posbyte = 0;
    while (bufferMB[posbyte] == 255){
        posbyte++;
    }

    // Localizamos el primer bit dentro de ese byte que vale 0
    unsigned char mascara = 128; // 10000000
    unsigned int posbit = 0;
    while (bufferMB[posbyte] & mascara)     {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }

    //Determinar cuál es finalmente el nº de bloque físico que podemos reservar
    int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

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
    //Leemos el superbloque
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
    /// Lee el superbloque del disco.
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return EXIT_FAILURE;
    }

    // Buffer de inodos.
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Leemos el bloque que contiene el inodo
    if (bread((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))),inodos) == -1){
        return -1;
    }

    //Modificamos el buffer con el inodo
    inodo=inodos[(ninodo % (BLOCKSIZE / INODOSIZE))];

    //Escribimos el buffer en el disco
    if (bwrite((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))),inodos) == -1){
        return -1;
    }

    return EXIT_SUCCESS;
    
}

/*
Lee un determinado inodo del array de inodos para volcarlo 
en una variable de tipo struct inodo pasada por referencia
*/
 int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    //Lectura del superbloque del disco.
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        return -1;
    }

    //Buffer de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Lectura del bloque que contiene el inodo
    if (bread((SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE))),inodos) == -1){
        return -1;
    }

    //Modifica el buffer con el inodo 
    *inodo = inodos[(ninodo % (BLOCKSIZE / INODOSIZE))];

    return EXIT_SUCCESS;
 }


/*
Encuentra el primer inodo libre , lo reserva, devuelve su número 
y actualiza la lista enlazada de inodos libres.
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos){
     // Lee el superbloque del disco.
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }

    // Comprobar si hay inodos libres.
    if (!SB.cantInodosLibres)
    {
        return -1;
    }

    // Obtener el primer inodo libre.
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;

    // Buffer de un inodo.
    struct inodo inodo;

    // Actualiza la lista enlazada de inodos libres.
    if (leer_inodo(posInodoReservado, &inodo))
    {
        return -1;
    }
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];

    // Inicializa todos los campos del inodo.
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.numBloquesOcupados = 0;
    for (int i = 0; i < (sizeof(inodo.punterosDirectos) / sizeof(unsigned int));
         i++)
    {
        inodo.punterosDirectos[i] = 0;
    }
    for (int i = 0; i < (sizeof(inodo.punterosIndirectos) /
                         sizeof(unsigned int));
         i++)
    {
        inodo.punterosIndirectos[i] = 0;
    }

    // Escribe el inodo en el array de inodos del disco.
    if (escribir_inodo(posInodoReservado, inodo))
    {
        return -1;
    }

    // Actualizar el superbloque y escribirlo en el disco.
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }

    return posInodoReservado;

}    


/*
Se encarga de obtener el rango de punteros en el que se situa el bloque logico que buscamos
y obti ene la di r eccion almacenada en el puntero del inodo
*/  
int obtener_nRangoBL (struct inodo *inodo , unsigned int nblogico, unsigned int *ptr) {
    if(DIRECTOS>nblogico){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
          
    }else if ( INDIRECTOS0>nblogico){
        *ptr=inodo->punterosIndirectos[0];
        return 1;
          
    }else if(INDIRECTOS1>nblogico){
        *ptr=inodo->punterosIndirectos[1];
        return 2;
        
    }else if(INDIRECTOS2>nblogico){
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
            return (nblogico - INDIRECTOS0)/ NPUNTEROS; 
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS; 
        }

    }else if(nblogico < INDIRECTOS2){//ej nblogico=400.004 
        if (nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }else if (nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }else if (nivel_punteros = 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
}

/*
Se encarga de obtener el numero de bloque fisico correspondiente 
a un bloque logico determinado del inodo indicado. 
Enmascara la gestion de los diferentes rangos de punteros directos e indirectos 
del inodo, de manera que funciones externas  no tienen que preocuparse de como acceder 
a los bloques fisicos apuntados desde el inodo
*/
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    //Declaracion de variables
    struct inodo inodo;
    unsigned int ptr,ptr_ant;
    int buffer[NPUNTEROS];
    int salvar_inodo, nRangoBL, nivel_punteros,indice;

    if(leer_inodo(ninodo,&inodo)==-1){
        perror("Error al traducir el bloque");
        return -1;
    }

    ptr=0;
    ptr_ant=0;
    salvar_inodo=0;
    
    //Obtenemos el rango
    nRangoBL=obtener_nRangoBL(&inodo,nblogico,&ptr);//0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros=nRangoBL;//el nivel_punteros +alto es el que cuelga del inodo

    //miramos todos los niveles 
    while(nivel_punteros>0){
    
        if(ptr==0){

            if(reservar==0){
                //si no hay punteros avisamos del error
                perror("Error en obtener_nRangoBL");
                return -1;

            }else{
            //reservamos los bloques de los punteros y creamos enlaces
            //desde inodo hasta datos
            salvar_inodo=1;
            ptr=reservar_bloque; //de punteros
            inodo.numBloquesOcupados++;
            inodo.ctime=time(NULL); //fecha actual

            if(nivel_punteros==nRangoBL){
                //el bloque cuelga directamente del inodo
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;

            }else{//el bloque cuelga de otro bloque de punteros

                buffer[indice] = ptr;// (imprimirlo para test)

                ////salvamos en el dispositivo el buffer de punteros modificado
                if (bwrite(ptr_ant, buffer) == -1){
                        perror ("Error al obtener_nRangoBL");
                        return -1;
                    }
                }

            }

            //ponemos a 0 todos los punteros del buffer
            if(memset(buffer, 0, BLOCKSIZE)==NULL){
                return -1;
            }

            }else{
                if (bread(ptr, buffer) == -1){
                perror("Error al obtener_nRangoBL");
                return -1;
            }
            }

            indice = obtener_indice(nblogico, nivel_punteros);
            ptr_ant = ptr;        //guardamos el puntero
            ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
            nivel_punteros--;
            }

            //nos encontramos en el nivel de datos
            if(ptr==0){
                //no existe bloque de datos
                if(reservar==0){
                    //error lectura ∄ bloque
                    return -1;

                }else{
                    salvar_inodo = 1;
                    ptr = reservar_bloque(); //de datos
                    inodo.numBloquesOcupados++;
                    inodo.ctime = time(NULL);

                    if(nRangoBL==0){
                        inodo.punterosDirectos[nblogico]=ptr;// (imprimirlo para test)

                    }else{
                        buffer[indice] = ptr; // (imprimirlo para test)

                        if (bwrite(ptr_ant, buffer) == -1){
                            perror("Error en obtener_nRangoBL");
                            return -1;
                            }
                    }
                }
            }

            if (salvar_inodo == 1){
                if (escribir_inodo(ninodo, inodo) == -1){
                    perror("Error en salvar inodo");
                    return -1;
        }
    }

            return ptr; //nbfisico del bloque de datos

    }
    


 




