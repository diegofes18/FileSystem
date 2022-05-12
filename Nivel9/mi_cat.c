//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

/*
Muestra todo el contenido de un fichero 
*/
int main(int argc, char **argv){
    /*
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
    */

   //Comprobamos que los parametros sean correctos
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis errónea: ./mi_cat <disco> </ruta_fichero>\n");
        return EXIT_FAILURE;
    }
    // Montar el dispositivo en el sistema.
    if (bmount(argv[1]) == EXIT_FAILURE)
    {
        fprintf(stderr, "mi_cat.c: Error al montar el dispositivo.\n");
        return EXIT_FAILURE;
    }

    //Obtenemos los parámetros de argv
    int tambuffer = BLOCKSIZE * 4;
    int bytes_leidos = 0;
    int bytes_leidosAux = 0;
    int offset = 0;
    char *camino = argv[2]; // path
    char buffer[tambuffer];

    memset(buffer, 0, sizeof(buffer));

    //Leemos todo el fichero o hasta completar el buffer
    bytes_leidosAux = mi_read(camino, buffer, offset, tambuffer);
    while (bytes_leidosAux > 0)
    {
        //Actualiza el número de bytes leidos.
        bytes_leidos += bytes_leidosAux;

        //Escribe el contenido del buffer en el destino indicado.
        write(1, buffer, bytes_leidosAux); // imprime pr pantalla

        //Limpia el buffer de lectura, actualiza el offset y vuelve a leer.
        memset(buffer, 0, sizeof(buffer));
        offset += tambuffer;
        //fprintf(stderr, " \n");
        bytes_leidosAux = mi_read(camino, buffer, offset, tambuffer);
    }

    fprintf(stderr, " \n");
    if (bytes_leidos < 0)
    {
        mostrar_error_buscar_entrada(bytes_leidos);
        bytes_leidos = 0;
    }
    fprintf(stderr, "Total_leidos: %d\n", bytes_leidos);

    // Desmonta el dispositivo del sistema
    bumount();
    return EXIT_SUCCESS;
}
