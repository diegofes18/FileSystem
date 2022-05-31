//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA
#include "ficheros.h"
#define DEBUGGER 0
/**
 Escribe el contenido procedente de un buffer en el disco virtual
*/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesescritos = 0;
    int auxByteEscritos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    
    //lectura del inodo
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("mi_write_f(): Error leer_inodo() \n");
        return -1;
    }

    //miramos los permisos del indod
    if ((inodo.permisos & 2) != 2){ //permisos de escritura
        perror("No hay permisos de escritura\n");
        return -1;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    mi_waitSem();
    //obtenemos el numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if (nbfisico == -1){
        perror("mi_write_f(): Error al traducir_bloque_inodo()\n");
        mi_signalSem();
        return -1;
    }

    mi_signalSem();

    //lectura del bloque fisico
    if (bread(nbfisico, buf_bloque) == -1){
        perror("mi_write_f(): Error al leer el bloque fisico\n");
        return -1;
    }

    //lo que queremos escribir cabe en un bloque fisico
    if (primerBL == ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        //escritura del bloque en el disco
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error mi_write_f(): bwrite()\n");
            return -1;
        }

        bytesescritos += nbytes;
    }

    //Caso en el que la escritura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL){
        //Parte 1: Primero bloque escrito parcialmente
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        //escritura del bloque fisico en el disco
        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error mi_write_f(): bwrite()\n");
            return -1;
        }

        bytesescritos += auxByteEscritos - desp1;

        //Parte 2: Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++){
            //Obtenemos los bloques intermedios
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if (nbfisico == -1){
                perror("Error mi_write_f(): traducir_bloque_inodo()\n");
                mi_signalSem();
                return -1;
            }

            //Escribimos los bloques intermedios
            auxByteEscritos = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if (auxByteEscritos == -1){
                perror("Error mi_write_f(): bwrite()\n");
                return -1;
            }

            bytesescritos += auxByteEscritos;
        }

        //Parte 3: Último bloque escrito parcialmente
        //obtencion del bloque fisico
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nbfisico == -1){
            perror("Error mi_write_f(): traducir_bloque_inodo()\n");
            mi_signalSem();
            return -1;
        }

        mi_signalSem();
        
        if (bread(nbfisico, buf_bloque) == -1){
            perror("Error mi_write_f(): bread()\n");
            return -1;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        auxByteEscritos = bwrite(nbfisico, buf_bloque);
        if (auxByteEscritos == -1){
            perror("Error mi_write_f(): bwrite()\n");
            return -1;
        }

        bytesescritos += desp2 + 1;
    }

    mi_waitSem(); 

    //Leer el inodo actualizado.
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error leer_inodo(): mi_write_f() \n");
        return -1;
    }

    //actualizacion de la metainformación
    //comprobamos si lo que hemos escrito es mas grande que el fichero
    if (inodo.tamEnBytesLog < (nbytes + offset)){
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error escribir_inodo(): mi_write_f() \n");
        return -1;
    }

    mi_signalSem();

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == bytesescritos){
#if DEBUGGER
        fprintf(stderr,"\tmi_write_f: BIEN\n");
        fprintf(stderr,"\tmi_read_f(): nbfisico = %i\n", nbfisico);
#endif
        mi_signalSem();
        return bytesescritos;
    }

    else{
#if DEBUGGER
        fprintf(stderr,"mi_write_f: MAL\n\tnbytes:%i\n\tbytesescritos:%i\n", nbytes, bytesescritos);
#endif
        mi_signalSem();
        return -1;
    }
}

/**
Escribe el contenido procedente de un buffer en el disco virtual
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesleidos = 0;
    int auxByteLeidos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    //lectura inodo
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error in mi_read_f(): leer_inodo()\n");
        return bytesleidos;
    }

    //miramos los permisos para leer
    if ((inodo.permisos & 4) != 4){
        perror("Error in mi_read_f(): No hay permisos de lectura!\n");
        return bytesleidos;
    }

    if (offset >= inodo.tamEnBytesLog){
        //no se puede leer mas
        return bytesleidos;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog){ //pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //obtenemos el numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

    //Caso el cual lo que queremos leer cabe en un bloque fisico
    if (primerBL == ultimoBL){
        if (nbfisico != -1){
            //lectura del bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == -1){
                perror("Error mi_read_f(): bread()\n");
                return -1;
            }

            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        bytesleidos = nbytes;
    }

    //Caso en el que la lectura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL){
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != -1){
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == -1){
                fprintf(stderr, "Error mi_read_f(): bread()\n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytesleidos = BLOCKSIZE - desp1;

        //Parte 2: Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++){
            //obtenemos los bloques intermedios
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != -1){
                auxByteLeidos = bread(nbfisico, buf_bloque);
                if (auxByteLeidos == -1){
                    perror("Error mi_read_f(): bread()\n");
                    return -1;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }

            bytesleidos += BLOCKSIZE;
        }

        //Parte 3: Último bloque leido parcialmente
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != -1){
            //lectura del bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == -1){
                perror("Error mi_read_f(): bread()\n");
                return -1;
            }

            //calculamos el byte lógico del último bloque hasta donde hemos de leer
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }

        bytesleidos += desp2 + 1;
    }

    mi_waitSem();

    //lectura del inodo actualizado
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error leer_inodo(): mi_read_f()\n");
        return -1;
    }

    //actualizamos la metainformación
    inodo.atime = time(NULL);

    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error escribir_inodo(): mi_read_f()\n");
        return -1;
    }

    //miramos posibles errores de escritura
    if (nbytes == bytesleidos){
#if DEBUGGER
        fprintf(stderr,"\tmi_read_f: BIEN\n");
#endif
        mi_signalSem();
        return bytesleidos;
    }

    else{
#if DEBUGGER
        fprintf(stderr,"mi_read_f(): MAL\n\tnbytes:%i\n\tbytesleidos:%i\n", nbytes, bytesleidos);
#endif
        mi_signalSem();
        return -1;
    }
}

/**
Devuelve la metainformación de un fichero/directorio
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    //lectura del inodo
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo)){
        perror("Error mi_stat_f(): leer_inodo()\n");
        return -1;
    }

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

/**
Cambia los permisos de un fichero/directorio
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){

    mi_waitSem();

    //lectura del inodo
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error mi_chmod_f(): leer_inodo()\n");
        mi_signalSem();
        return -1;
    }

    //cambiamos los permisos del archivo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error mi_chmod_f(): leer_inodo()\n");
        mi_signalSem();
        return -1;
    }

    mi_signalSem();
    return EXIT_SUCCESS;
}

/**
Truncar inodo pasado.
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    
    int lib;
    struct inodo inodo;

    //lectura inodo
    if (leer_inodo(ninodo, &inodo) == -1){
        perror(" Error mi_truncar_f(): leer_inodo()\n");
        return -1;
    }

    if ((inodo.permisos & 2) != 2){ //permisos de escritura
        perror(" Error mi_truncar_f(): El inodo no tiene permisos.\n");
        return -1;
    }

    //comprobamos que no intenten truncar mas allá del tamaño de bytes lógicos
    if (nbytes > inodo.tamEnBytesLog){
        perror(" Error mi_truncar_f(): No se puede truncar porque, nbytes > tamaño de bytes lógicos");
        return -1;
    }

    //Obtener bloque logico
    int primerBL;
    if (nbytes % BLOCKSIZE == 0){
        primerBL = nbytes / BLOCKSIZE;
    }

    else{
        primerBL = (nbytes / BLOCKSIZE) + 1;
    }

    //liberar bloques a partir del primerBL del inodo
    lib = liberar_bloques_inodo(primerBL, &inodo);

    //actualizamos
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= lib;

    if (escribir_inodo(ninodo, inodo) == -1){
        perror("Error mi_truncar_f(): escribir_inodo()\n");
        return -1;
    }

    return lib;
}
