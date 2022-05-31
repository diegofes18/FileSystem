//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;
static int descriptor = 0;
static unsigned int inside_sc = 0;


/*
Función que se encarga de montar el dispositivo virtual, 
devuelve el descriptor o -1 (si ha ido mal)
*/
int bmount(const char *camino){

    //Semaforo
    if (!mutex){
        mutex = initSem();
        if (mutex == SEM_FAILED){
            return -1;
        }
    }

    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if (descriptor == -1){
        perror("Error al abrir el fichero.\n");
        return -1;
    }

    return descriptor;
}

/*
Funcion que desmonta el dispositivo virtual
0->si se ha cerrado correctamente
-1->si ha habido error
*/
int bumount(){

    deleteSem();

    if (close(descriptor) == -1){
        return -1;
    }

    return EXIT_SUCCESS;

}

/*
Escribe 1 bloque en el dispositivo virtual y devuelve el número
de bytes que se han escrito
*/
int bwrite(unsigned int nbloque, const void *buf){

    //movemos el puntero del fichero en el offset correcto
    int off_t=lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);

    if(off_t!=-1){

        //escritura del bloque
        size_t b = write(descriptor, buf, BLOCKSIZE);

        //Si no se ha podido escribir
        if (b < 0){
            perror("bwrite(): Error al escribir el bloque.\n");
            return -1;
        }

        return b;
    }
    else{
        perror("bwrite(): Error al posicionarse en el fichero.\n");
        return -1;
    }

}

/*
Función que se encarga de leer un bloque del dispositivo virtual
y devuelve el número de bytes leídos
*/
int bread(unsigned int nbloque, void *buf){

    //movemos el puntero del fichero en el offset correcto
    int off_t=lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET);

    if(off_t!=-1){

        //lectura del bloque
        size_t b = read(descriptor, buf, BLOCKSIZE);

        //Si no se ha podido leer
        if (b < 0){
            perror("bread(): Error al leer el bloque.\n");
            return -1;
        }
        return b;
    }
    else{
        perror("bread(): Error al posicionarse en el fichero.\n");
        return -1;
    }

    

}

void mi_waitSem() {
   if (!inside_sc) {
       waitSem(mutex);
   }
   inside_sc++;
}

void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}
