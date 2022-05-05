//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        perror("Sintaxis incorrecta -> ./mi_ls <disco> </ruta_directorio>.");
        return -1;
    }
    const char *camino = argv[2];
    char buf[1000000];

    if (bmount(argv[1])==-1) {
        perror("Error al montar el dispositivo virtual.");
        return -1;  
    }
    int err = mi_mkdir(camino, buf);

    if(err < 0){
        perror("Error al listar el directorio.");
        return err;
    }
    fprintf(stderr,"%s\n",buf);

    if (bumount() == EXIT_FAILURE){
        perror("Error al desmontar el dispositivo virtual.\n");
        return EXIT_FAILURE;
    }
    return err;
}