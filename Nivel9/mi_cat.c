//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char **argv){
    if (argc != 3) {
        perror("Error de sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }
    if (bmount(argv[1]) == -1){
        perror("Error al montar el disco\n");
        return -1;
    }
    char *camino = argv[2];
    unsigned int offset = 0, rBytes = 0;
    char buf[BLOCKSIZE];

    if (camino[strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "La entrada %s no es un fichero\n", camino);
        return -1;
    }
    memset(buf, 0, BLOCKSIZE);
    int err = mi_read(camino, buf, offset, BLOCKSIZE);

    while(err>0) {
        write(1, buf, err);
        rBytes += err;
        offset += BLOCKSIZE;
        memset(buf, 0, BLOCKSIZE);
        err = mi_read(camino, buf, offset, BLOCKSIZE);
    }
    char bufInfo[100];
    sprintf(bufInfo, "\n Total leídos: %d\n", rBytes);
    write(2, bufInfo, strlen(bufInfo));

    if (bumount() == -1){
        perror("Error al desmontar el dispositivo virtual.\n");
        return -1;
    }
    return 0; 
}
