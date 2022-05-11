//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 5) {
        perror("Error de sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }
    if (bmount(argv[1]) == -1){
        perror("Error al montar el disco\n");
        return -1;
    }
    char *camino = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]), length = strlen(texto);

    if (camino[strlen(camino) - 1] == '/') {
        fprintf(stderr, "La entrada %s no es un fichero\n", camino);
        return -1;
    }
    int err = mi_write(camino, texto, offset, length);

    if (err < 0) {
        fprintf(stderr, "No se ha podido escribir en la entrada %s\n", camino);
        return -1;
    }
    fprintf(stderr, "\nNúmero de bytes escritos: %d\n", errores);
    
    if (bumount() == -1) {
        perror("Error al desmontar el dispositivo virtual.\n");
        return -1;
    }
    return 0;
}