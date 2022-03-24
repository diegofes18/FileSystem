#include "ficheros.h"

//Escribe el contenido del buffer en un fichero indicado en el inodo
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){

    //Asignacion de variables
    int primerBL=offset/BLOCKSIZE;
    int ultimoBL=(offset+nbytes-1)/BLOCKSIZE;
    
    //desplazamientos donde cae el offset
    int desp1=offset%BLOCKSIZE;
    int desp2=(offset+nbytes-1)%BLOCKSIZE;
    int bloquefisico;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    int bytescritos=0;
    int auxbyteescritos=0;

    //leemos el inodo
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
        perror("Error en mi_write_f al leer el inodo");
        return EXIT_FAILURE;
    }

    //Miramos permiso de escritura
    if ((inodo.permisos&2)!=2){
        perror("No tiene permisos de escritura");
        return EXIT_FAILURE;
    }
    //Obtenemos numero de bloque fisico
    bloquefisico=traducir_bloque_inodo(ninodo,primerBL,1);
    if(bloquefisico==-1){
        perror("error al escribir en el fichero");
        return EXIT_FAILURE;
    }
    //Leemos el bloque fisico
    if(bread(bloquefisico,buf_bloque)== EXIT_FAILURE){
        perror ("Error en mi_write_f al leer bloque fisico");
        return EXIT_FAILURE;
    }
    //Caso en que el buffer cabe en un bloque 
    if(primerBL==ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        auxbyteescritos=bwrite(bloquefisico, buf_bloque);
        if(auxbyteescritos==EXIT_FAILURE){
            perror("error en mi_write en bwrite");
            return EXIT_FAILURE;
        }
        byteescritos+=bloquefisico;
    }




}

