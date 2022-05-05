//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        perror("Sintaxis incorrecta -> ./mi_ls <disco> </ruta_directorio>.");
        return -1;
    }
    const char *camino = argv[2];
    char buf[1000000];
    memset(buf, 0, 1000000);
    char tipo = '\0';

    if (bmount(argv[1])==-1) {
        perror("Error al montar el dispositivo virtual.");
        return -1;  
    }

    int err = mi_dir(camino, buf,&tipo);

    if(err < 0){
        mostrar_error_buscar_entrada(err);
        return -1;
    }

    fprintf(stderr,"%s\n",buf);

    if (bumount() == -1){
        perror("Error al desmontar el dispositivo virtual.\n");
        return -1;
    }
    return err;
}