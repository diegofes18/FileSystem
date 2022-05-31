//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

#include "ficheros_basico.h"

#define DEBUG3 0 //Debugger del nivel 3
#define DEBUG4 0 //Debugger del nivel 4

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

    int tamMB = SB.posUltimoBloqueMB - SB.posPrimerBloqueMB;

    //inicializamos el mapa de bits
    for (int i = SB.posPrimerBloqueMB; i <= tamMB + SB.posPrimerBloqueMB; i++){
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

    //escribimos el bufferMB en el dispositivo virtual
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

    //miramos que numero de bloque fisico podemos reservar
    unsigned int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    //bloque reservado
    if (escribir_bit(nbloque, 1) == -1){
        return -1;
    }

    SB.cantBloquesLibres--;

    if (memset(bufferAux, 0, BLOCKSIZE) == NULL){
        perror("Error while memset in reservar_bloque()\n");
        return -1;
    }

    //volvemos a escribirlo en el bucle
    if (bwrite(SB.posPrimerBloqueDatos + nbloque - 1, bufferAux) == -1){
        perror("Error al reservar_bloque()\n");
        return -1;
    }

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

    //ponemos a 0 el bit del MB correspondiente al bloque nbloque
    escribir_bit(nbloque,0);

    //incrementamos la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres++;

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

    //buffer de lectura de un array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //obtenemos el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    //lectura del bloque que contiene el inodo
    if (bread(posBloqueInodo,inodos) == -1){
        return -1;
    }

    //modificamos el buffer con el inodo en el lugar correspondiente
    inodos[(ninodo % (BLOCKSIZE / INODOSIZE))] = inodo;

    //escritura del bloque modificado en el dispositivo virtual
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

    //buffer de lectura de un array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //obtenemos el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    //lectura del bloque que contiene el inodo
    if (bread(posBloqueInodo,inodos) == -1){
        perror("leer_inodo(): Error al leer el bloque de array de inodos.\n");
        return -1;
    }

    //modificamos el buffer con el inodo en el lugar correspondiente
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXIT_SUCCESS;
 }
 

/*
Encuentra el primer inodo libre , lo reserva, devuelve su número 
y actualiza la lista enlazada de inodos libres
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos){
    
    struct inodo inodoAUX;

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
    leer_inodo(posInodoReservado, &inodoAUX);
    SB.posPrimerInodoLibre = inodoAUX.punterosDirectos[0];
    SB.cantInodosLibres--;
    

    //variables
    inodoAUX.tipo = tipo;
    inodoAUX.permisos = permisos;
    inodoAUX.nlinks = 1;
    inodoAUX.tamEnBytesLog = 0;
    inodoAUX.atime = time(NULL);
    inodoAUX.mtime = time(NULL);
    inodoAUX.ctime = time(NULL);
    inodoAUX.numBloquesOcupados = 0;

    //directos
    for (int i = 0; i < 12; i++){
        inodoAUX.punterosDirectos[i] = 0;
    }

    //indirectos
    for (int j = 0; j < 3; j++){
        inodoAUX.punterosIndirectos[j] = 0;
    }

    //escritura del inodo en la posición del que era el primer inodo libre
    if (escribir_inodo(posInodoReservado, inodoAUX) == -1){
        perror("reservar_inodo(): Error escribir_bit()\n");
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
        return (nblogico - DIRECTOS); 

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

/*
se encarga de obtener el nº de bloque físico correspondiente 
a un bloque lógico determinado del inodo indicado
*/
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
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

/*
Liberar un inodo implica por un lado, que tal inodo pasará a la cabeza de la lista de inodos libres
y tendremos un inodo más libre en el sistema.
*/
int liberar_inodo(unsigned int ninodo) {

    int liberados = 0;
    struct inodo inodo;

    //lectura inodo para saber cuantos bloques ocupa
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("liberar_inodo(): Error al leer_inodo()\n");
        return -1;
    }

    //liberamos los bloques logicos
    if (inodo.tamEnBytesLog > 0){
        liberados = liberar_bloques_inodo(0, &inodo);
    }

    inodo.numBloquesOcupados -= liberados;

    //todos los inodos liberados?
    if (inodo.numBloquesOcupados != 0){
        fprintf(stderr, "Faltan por liberar %d bloques del inodo", inodo.numBloquesOcupados);
        return -1;
    }

    //actualizamos inodo
    inodo.tipo = LIBRE; //lo dejamos libre
    inodo.tamEnBytesLog = 0; //lo dejamos vacio

    //lectura del superbloque del dispositivo virtual
    struct superbloque SB;
    if (bread(posSB, &SB) == -1){
        perror("liberar_inodo(): Error al leer el SB.\n");
        return -1;
    }

    //Incluimos el inodo liberado en la lista enlazada como primer inodo libre, y enlazamos
    //este con el anterior para no perder el orden
    unsigned int auxinodo = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    inodo.punterosDirectos[0] = auxinodo;
    SB.cantInodosLibres++;

    //Guardamos los datos modificados del inodo
    if (escribir_inodo(ninodo, inodo) == -1){
        fprintf(stderr, "liberar_inodo(): Error escribir_inodo().\n");
        return -1;
    }

    if (bwrite(posSB, &SB) == -1){
        perror("liberar_inodo(): Error al escribir el SB.\n");
        return -1;
    }

    return ninodo;
}


/*
libera todos los bloques ocupados a partir del bloque lógico indicado por el argumento primerBL
*/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; //array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];  //punteros a bloques de punteros de cada nivel
    int indices[3];    //indices de cada nivel
    int liberados = 0; // nº de bloques liberados
    int breads=0;
    int bwrites=0;

    if ((inodo->tamEnBytesLog) == 0){// el fichero está vacío
        return liberados; 
    }

    //Calculamos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0){
        //Ultimo Bloque Lógico es un bloque entero
        ultimoBL = ((inodo->tamEnBytesLog) / BLOCKSIZE) - 1;
    }
    else{
        //Ultimo Bloque Lógico no es un bloque entero
        ultimoBL = (inodo->tamEnBytesLog) / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;

#if DEBUGN6
    printf("[liberar_bloques_inodo()-> primerBL: %d, ultimoBL: %d]\n", primerBL, ultimoBL);
#endif

    //Recorrido de los bloques lógicos del inodo
    for (nBL = primerBL; nBL <= ultimoBL; nBL++){

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
        if (nRangoBL < 0){
            fprintf(stderr, "Error: al obtener el rango del BL en el método %s()",__func__);
            return -1;
        }

        nivel_punteros = nRangoBL; //el nivel_punteros  mas alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { //cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);
            if ((indice == 0) || (nBL == primerBL)){
                //solo leemos del dispositivo si no está ya cargado previamente en un buffer
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == -1){
                    fprintf(stderr, "Error: leer el dispositivo no cargado previamente, en el método %s()",__func__);
                    return -1;
                }
                breads++;//incrementamos número de lecturas
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0){ //si existe bloque de datos

            liberar_bloque(ptr);
            liberados++;
#if DEBUGN6
    printf("[liberar_bloques_inodo()-> liberado BF %d de datos par a BL %d]\n", ptr, nBL);
#endif
            if (nRangoBL == 0){ //es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL){

                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];

                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0){
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;

                       //Incluir mejora para saltar los bloques que no sea necesario explorar!!! ...
                        switch (nivel_punteros-1){
                        case 0:
                            nBL += NPUNTEROS - indices[nivel_punteros-1] - 1;
                            break;
                        case 1:
                            nBL += NPUNTEROS * (NPUNTEROS -
                                                indices[nivel_punteros-1]) -1;
                            break;
                        case 2:
                            nBL += (NPUNTEROS * NPUNTEROS) *
                                       (NPUNTEROS - indices[nivel_punteros-1]) - 1;
                            break;
                        default:
                            break;
                        }
                        

#if DEBUGN6
    printf("[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n", ptr, nivel_punteros, nBL);
#endif

                        if (nivel_punteros == nRangoBL){
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else{ //escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == -1){
                            fprintf(stderr, "Error: de escritura en el método %s()",__func__);
                            return -1;
                        }
                        bwrites++;//incrementamos número de escrituras
                        //hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        //superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }//fsi
                }//fmientras
            }//fsi

        }//fsi
    }//fpara
#if DEBUGN6
    printf("[liberar_bloques_inodo()-> total bloques liberados: %d,total breads: %d, total bwrites: %d]\n", liberados,breads,bwrites);
#endif
    return liberados;
}