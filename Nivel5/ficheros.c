#include "ficheros.h"
#define DEBUGGER 0


//Escribe el contenido del buffer en un fichero indicado en el inodo
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){

    //declaraciones de variables
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesescritos = 0;
    int auxByteEscritos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    //lectura del inodo.
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error in mi_write_f(): leer_inodo() \n");
        return -1;
    }

    //comprobamos que el inodo tenga los permisos para escribir
    if ((inodo.permisos & 2) != 2){
        perror("El inodo no tiene permisos para escribir en mi_write_f() \n");
        return -1;
    }

    //Asignaciones de las variables.
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtencion del numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if (nbfisico == -1){
        perror("Error in mi_write_f(): traducir_bloque_inodo()\n");
        return -1;
    }

    //leemos el bloque fisico
    if (bread(nbfisico, buf_bloque) == -1){
        perror("Error in mi_write_f(): bread()\n");
        return -1;
    }

    //Caso en el que lo que queremos escribir cabe en un bloque fisico
    if (primerBL == ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);
 
        //Escribimos el bloque fisico en el disco
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error in mi_write_f(): bwrite()\n");
            return -1;
        }
        bytesescritos += nbytes;
    }

    //Caso en el que la escritura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL){

        // Primero bloque escrito parcialmente
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        //Escribimos el bloque fisico 
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error in mi_write_f(): bwrite()\n");
            return -1;
        }

        bytesescritos += auxByteEscritos - desp1;

        // Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++){
            //Obtenemos los bloques intermedios
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if (nbfisico == -1){
                perror("Error in mi_write_f(): traducir_bloque_inodo()\n");
                return -1;
            }

            //Escribimos los bloques intermedios
            auxByteEscritos = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if (auxByteEscritos == -1){
                perror("Error in mi_write_f(): bwrite()\n");
                return -1;
            }
            bytesescritos += auxByteEscritos;
        }

        // Último bloque escrito parcialmente
        
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == -1){
            perror("Error in mi_write_f(): traducir_bloque_inodo()\n");
            return -1;
        }

        //Leemos el bloque fisico
        if (bread(nbfisico, buf_bloque) == -1){
            perror("Error in mi_write_f(): bread()\n");
            return -1;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error in mi_write_f(): bwrite()\n");
            return -1;
        }

        bytesescritos += desp2 + 1;
    }

    //Leer el inodo actualizado.
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error in leer_inodo(): mi_write_f() \n");
        return -1;
    }

    //Actualizar la metainformación

    //Comprobar si lo que hemos escrito es mas grande que el fichero
    if (inodo.tamEnBytesLog < (nbytes + offset)){
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error in escribir_inodo(): mi_write_f() \n");
        return -1;
    }

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == bytesescritos)
    {
#if DEBUGGER
        printf("\tmi_write_f: BIEN\n");
        printf("\tmi_read_f(): nbfisico = %i\n", nbfisico);

#endif
        return bytesescritos;
    }
    else
    {
#if DEBUGGER
        printf("mi_write_f: MAL\n\tnbytes:%i\n\tbytesescritos:%i\n", nbytes, bytesescritos);
#endif
        return EXIT_FAILURE;
    }


    
}


/*
Lee informacion de un fichero/directorio y la almacena en un buffer de memoria
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

    //variables
    unsigned int PrimerBloque, UltimoBloque;
    int desp1, desp2, nBloqueFis;
    int leidos = 0;
    int aux = 0;
    char unsigned buffer[BLOCKSIZE];
    struct inodo inodo;

    //lectura inodo
    if(leer_inodo(ninodo,&inodo)==-1){
        perror("Error en leer_inodo");
        return leidos;
    }

    //miramos si el inodo tenga los permisos para leer
    if ((inodo.permisos & 4) != 4){
        perror("No hay permisos de lectura");
        return leidos;
    }

    //la funcion no puede leer mas alla del tamano en bytes logicos del inodo
    if(offset>=inodo.tamEnBytesLog){
        leidos=0;
        return leidos;
    }

     //pretende leer mas alla de EOF
    if((offset+nbytes)>=inodo.tamEnBytesLog){
        //leemos solo los bytes que podemos desde el offset hasta EOF
        nbytes=inodo.tamEnBytesLog-offset;
    }

    //Asignaciones de las variables.
    PrimerBloque = offset / BLOCKSIZE;
    UltimoBloque = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //obtenemos el numero de bloque fisico
    nBloqueFis=traducir_bloque_inodo(ninodo,PrimerBloque,0);

    //lo que queremos leer cabe en un bloque fisico
    if (PrimerBloque == UltimoBloque){
        if (nBloqueFis != -1){
            //Leemos el bloque fisico del disco
            aux=bread(nBloqueFis, buffer);
            if (aux == -1){
                perror("Error en la funcion bread()");
                return -1;
            }

            memcpy(buf_original, buffer + desp1, nbytes);
        }
        
        leidos=nbytes; 

    }else if (PrimerBloque < UltimoBloque){ //la lectura ocupa mas de un bloque fisico
    
        //lectura parcial del primer bloque
        if (nBloqueFis != -1){
            //lectura del bloque fisico del disco
            aux = bread(nBloqueFis, buffer);
            if (aux == -1){
                perror("Error en la funcion bread()");
                return -1;
            }

            memcpy(buf_original, buffer + desp1, BLOCKSIZE - desp1);
        }

        leidos=BLOCKSIZE - desp1;

        //nos encargamos de los bloques intermedios
        for(int i=PrimerBloque+1; i<UltimoBloque; i++){
            //Obtenemos los bloques intermedios
            nBloqueFis=traducir_bloque_inodo(ninodo, i, 0);
            
            if (nBloqueFis!=-1){
                //lectura bloque fisico
                aux = bread(nBloqueFis, buffer);
                if (aux == -1){
                    perror("Error en la funcion bread()");
                    return -1;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i -PrimerBloque-1) * BLOCKSIZE, buffer, BLOCKSIZE);
            }

            leidos += BLOCKSIZE;
        }

        //lectura ultimo bloque
        nBloqueFis = traducir_bloque_inodo(ninodo, UltimoBloque, 0);
        if (nBloqueFis != -1){
            //lectura del bloque fisico
            aux = bread(nBloqueFis, buffer);
            if (aux == -1){
                perror("Error en la funcion bread()");
                return -1;
            }

            //calculamos el byte logico del ultimo bloque hasta donde hay que leer
            memcpy(buf_original + (nbytes - desp2 - 1), buffer, desp2 + 1);
        }

        leidos += desp2 + 1;

    }

    //lectura del inodo actualizado
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error en mi_read_f()");
        return -1;
    }

    //actualizacion de los metadatos
    inodo.atime = time(NULL);

    //escritura del inodo
    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error en mi_read_f()");
        return -1;
    }

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == leidos)
    {
#if DEBUGGER
        printf("\tmi_read_f: BIEN\n");
#endif
        return leidos;
    }
    else
    {
#if DEBUGGER
        printf("mi_read_f(): MAL\n\tnbytes:%i\n\tbytesleidos:%i\n", nbytes, leidos);
#endif
        return -1;
    }

}


//Actializa metainformacion de un ficheros
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    //Leemos el inodo
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==-1){
        perror("error mi stat:leer inodo");
        return -1;

    }
    // Actualizamos valores del inodo
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXIT_SUCCESS;
}

/*
Cambia los permisos de un fichero/directorio con el valor que indique el argumento permisos
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){

    struct inodo inodo;
    //lectura inodo
    if(leer_inodo(ninodo,&inodo)) {
        perror("Error en la funcion leer_inodo");
        return -1;
    }

    //actualizamos ctime
    inodo.ctime=time(NULL);

    //cambiamos los permisos 
    inodo.permisos=permisos;

    if (escribir_inodo(ninodo, inodo)){
        perror("Error en la funcion leer_inodo()");
        return -1;
    }

    return EXIT_SUCCESS;
}
