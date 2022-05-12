// Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char const *argv[])  {
    /*
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

    int p_inodo = mi_stat(argv[2], &stat);

    //Motramos valores
    printf("Nº de inodo: %d\n", p_inodo);
    printf("Tipo: %c\n",stat.tipo);
    printf("Permisos: %c\n",stat.permisos);
    printf("atime: %s",ctime(&stat.atime));
    printf("mtime: %s",ctime(&stat.mtime));
    printf("ctime: %s",ctime(&stat.ctime));
    printf("nlinks: %u\n",stat.nlinks);
    printf("tamEnBytesLog: %u\n",stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",stat.numBloquesOcupados);
    printf("Fichero: %s\n",camino);

    if (bumount() == -1){
        perror("Error al desmontar el dispositivo virtual.\n");
        return -1;
    }
    return err;
    */
   struct STAT p_stat;
    int p_inodo;

    if (argc != 3)
    {
        fprintf(stderr, "Error de sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n");
        return EXIT_FAILURE;
    }

    // Monta el disco en el sistema.
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error de montaje de disco.\n");
        return EXIT_FAILURE;
    }

    p_inodo = mi_stat(argv[2], &p_stat);

    if (p_inodo < 0)
    {
        mostrar_error_buscar_entrada(p_inodo);
        return EXIT_FAILURE;
    }

    // Variables para fecha y hora.
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Motramos valores
    printf("Nº de inodo: %d\n", p_inodo);
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    bumount();
}