
#include "bloques.h"
#include "ficheros_basico.c"


char* buf;


int main(int argc, char **argv){

    //Mirar la sintaxis
    if(argc != 3){
        perror("Sintaxis incorrecta -> ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return EXIT_FAILURE;
    }
    
    //Obtener parametros del comando
    char *camino=argv[1];
    int nbloques=atoi(argv[2]);
    int ninodos = nbloques/4;

    //iniciamos el buffer
    char buf[BLOCKSIZE];
    memset(buf, 0, BLOCKSIZE);


    //Montamos y comprobamos
    if(bmount(camino)==-1) {
        perror("Error el montar \n");
        return EXIT_FAILURE;
    }

    //Escribimos en el dispositivo
    for(int i=0; i<nbloques; i++){
        if(bwrite(i,buf)==EXIT_FAILURE){
            perror("Error al escribir en el dispostivo virtual");
            return EXIT_FAILURE;
        }
    }

    //Iniciamos metadatos p

    if (initSB(nbloques, ninodos) == EXIT_FAILURE)
    {
        perror("Fallo al iniciar el superbloque del despositivo virtual.\n");
        return EXIT_FAILURE;
    }
    if (initMB() == EXIT_FAILURE)
    {
        perror("Fallo al iniciar el mapa de bits del despositivo virtual.\n");
        return EXIT_FAILURE;
    }
    if (initAI() == EXIT_FAILURE)
    {
        perror("Fallo al iniciar el arrat de inodos del despositivo virtual.\n");
        return EXIT_FAILURE;
    }
    
    //Desmontamos el dispostivo virtual
     if (bumount() == EXIT_FAILURE)
    {
        perror("Error al desmontar\n"); 
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; 


}