
//#include "bloques.h"
//#include "ficheros_basico.c"
#include "ficheros_basico.h"


char* buf;


int main(int argc, char **argv){

    //Mirar la sintaxis
    if(argc != 3){
        perror("Sintaxis incorrecta -> ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return -1;
    }
    
    //Obtener parametros del comando
    char *camino=argv[1];
    int nbloques=atoi(argv[2]);
    int ninodos = nbloques/4;

    //iniciamos el buffer
    unsigned char buf[BLOCKSIZE];
    if(!memset(buf, 0, BLOCKSIZE)){
        return -1;
    }


    //Montamos y comprobamos
    if(bmount(camino)==-1) {
        perror("Error el montar \n");
        return -1;
    }

    //Escribimos en el dispositivo
    for(int i=0; i<nbloques; i++){
        if(bwrite(i,buf)==-1){
            perror("Error al escribir en el dispostivo virtual");
            return -1;
        }
    }

    //Iniciamos metadatos p
    if (initSB(nbloques, ninodos) == -1){

        perror("Fallo al iniciar el superbloque del despositivo virtual.\n");
        return -1;
    }

    if (initMB() == -1){

        perror("Fallo al iniciar el mapa de bits del despositivo virtual.\n");
        return -1;
    }

    if (initAI() == -1){

        perror("Fallo al iniciar el array de inodos del despositivo virtual.\n");
        return -1;
    }

    //Creacion de directorio raíz
    if(reservar_inodo('d', 7)==-1){
        perror("Error en la reserva del inodo raiz");
        return -1;
    }
    
    //Desmontamos el dispostivo virtual
    if (bumount() == -1){

        perror("Error al desmontar\n"); 
        return -1;
    }

    return EXIT_SUCCESS; 


}