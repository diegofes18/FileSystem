//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

#include "bloques.h"

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

    //iniciamos el buffer
    char buf[BLOCKSIZE];
    memset(buf, 0, BLOCKSIZE);


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

    //Desmontamos el dispostivo virtual
     if (bumount() == -1){
        perror("Error al desmontar\n"); 
        return -1;
    }
    
    return EXIT_SUCCESS; 


}