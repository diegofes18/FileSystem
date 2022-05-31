//Diego Bermejo, Marc Cañellas y Gaston Panizza

#include "directorios.h"
#define DEBUG 1

int main(int argc, char const *argv[]){

    if (argc != 5){
        perror("Error de sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    //Comprobamos si es un fichero
    if ((argv[2][strlen(argv[2]) - 1]) == '/'){
        perror("No es un fichero.\n");
        return -1;
    }

    int bytes_escritos;
    //montamos el dispositivo virtual
    if (bmount(argv[1]) == -1){
        perror("mi_escribir.c: Error al montar el dispositivo virtual.\n");
        return -1;
    }

#if DEBUG
    fprintf(stderr, "Longitud texto: %ld\n", strlen(argv[3]));
#endif

    bytes_escritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if (bytes_escritos < 0){
        mostrar_error_buscar_entrada(bytes_escritos);
        bytes_escritos = 0;
    }

    if (bumount() == -1)
    {
        return -1;
    }

#if DEBUG
    fprintf(stderr, "Bytes escritos: %d\n", bytes_escritos);
#endif

    return EXIT_SUCCESS;
    

}