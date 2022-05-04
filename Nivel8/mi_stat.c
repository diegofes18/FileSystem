// Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char const *argv[])  {
    if (argc != 3){
        perror("Sintaxis incorrecta -> ./mi_stat <disco> </ruta>");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        perror("Error al montar el dispositivo virtual.");
        return -1;
    }
    const char *camino = argv[2];
    struct STAT stat;
    int err = mi_stat(camino, &stat);

    if(err < 0){
        perror("Error al listar el directorio.");
        return err;
    }
    printf("Tipo: %c\n",stat.tipo);
    printf("Permisos: %c\n",stat.permisos);
    printf("atime: %s",ctime(&stat.atime));
    printf("mtime: %s",ctime(&stat.mtime));
    printf("ctime: %s",ctime(&stat.ctime));
    printf("nlinks: %u\n",stat.nlinks);
    printf("tamEnBytesLog: %u\n",stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",stat.numBloquesOcupados);
    printf("Fichero: %s\n",camino);

    if (bumount() == EXIT_FAILURE){
        perror("Error al desmontar el dispositivo virtual.\n");
        return EXIT_FAILURE;
    }
    return err;
}