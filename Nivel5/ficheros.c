#include "ficheros.h"
#define DEBUGGER 0

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
    if(leer_inodo(ninodo,&inodo)==-1){
        perror("Error en mi_write_f al leer el inodo");
        return -1;
    }

    //Miramos permiso de escritura
    if ((inodo.permisos&2)!=2){
        perror("No tiene permisos de escritura");
        return -1;
    }
    //Obtenemos numero de bloque fisico
    bloquefisico=traducir_bloque_inodo(ninodo,primerBL,1);
    if(bloquefisico==-1){
        perror("error al escribir en el fichero");
        return -1;
    }
    //Leemos el bloque fisico
    if(bread(bloquefisico,buf_bloque)== -1){
        perror ("Error en mi_write_f al leer bloque fisico");
        return -1;
    }
    //Caso en que el buffer cabe en un bloque 
    if(primerBL==ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        auxbyteescritos=bwrite(bloquefisico, buf_bloque);
        if(auxbyteescritos==-1){
            perror("error en mi_write en bwrite");
            return -1;
        }
        bytescritos+=nbytes;
    }
    
    //Caso en el que ocupa mas de un bloque
   if(primerBL<ultimoBL){
       //primer bloque escrito parcialmente
       memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
       //escritura de bloque en disco
       auxbyteescritos=bwrite(bloquefisico, buf_bloque);
       bytescritos+=auxbyteescritos - desp1;
       
       //Bloques intermedios
       for(int i=primerBL+1; i<ultimoBL; i++){
           //traducimos a bloques intermedios
           bloquefisico=traducir_bloque_inodo(ninodo ,i ,1);
           if(bloquefisico==-1){
               perror("traducibloque da error en mi_write");
               return -1;
           }
           //escritura de bloques
           auxbyteescritos=bwrite(bloquefisico, buf_original + (BLOCKSIZE - desp1)+(i-primerBL-1)* BLOCKSIZE);
           bytescritos+=auxbyteescritos;
           
       }
       //Ultimo bloque
       bloquefisico=traducir_bloque_inodo(ninodo,ultimoBL,1);
       if(bloquefisico==-1){
           return EXIT_FAILURE;
       }
       //leemos el bloque fisico
       if(bread(bloquefisico, buf_bloque)== EXIT_FAILURE){
           perror("Error al leer el bloque");
           return EXIT_FAILURE;
       }
       memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
       
       auxbyteescritos=bwrite(bloquefisico,buf_bloque);
       
       bytescritos+=desp2 + 1;
    }   
    if(leer_inodo(ninodo,&inodo)==-1){
         perror("error en leer inodo mi_write"); 
         return EXIT_FAILURE;
    }
       
       //Actualizamos metainformacion       
    if (inodo.tamEnBytesLog < (nbytes + offset)){
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

     inodo.mtime = time(NULL);

     if(escribir_inodo(ninodo, inodo)== -1){
         perror("error en escribir inodo");
     }
     //comprobamos
     if(nbytes==bytescritos){ 
         return bytescritos;
     }
     else{
         return -1;
     }
       
   
}


/*
Lee informacion de un fichero/directorio y la almacena en un buffer de memoria
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

    //Variables
    int PrimerBloque= offset / BLOCKSIZE;
    int UltimoBloque=(offset + nbytes - 1) / BLOCKSIZE;
    int nBloqueFis;
    int desp1=offset % BLOCKSIZE;
    int desp2=(offset + nbytes - 1) % BLOCKSIZE;
    int leidos=0;
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

    //miramos que no haya errores de escritura 
    if (nbytes == leidos){
#if DEBUGGER
        printf("\tmi_read_f: BIEN\n");
#endif
        return leidos;
    }
    else
    {
#if DEBUGGER
        printf("mi_read_f(): MAL\n\tnbytes:%i\n\tbytesleidos:%i\n", nbytes, bytesleidos);
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
        return EXIT_FAILURE;

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

