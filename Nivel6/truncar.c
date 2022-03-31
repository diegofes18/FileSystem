#include "ficheros.h"
int main(int argc, char **argv){
    //Validación de sintaxis
    if (argc != 4){
        perror("Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return -1;
    }
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);
    //Montar el dispositivo virtual
    if(bmount(argv[1])==-1){
        perror("Error en truncar.c -->\nFallo en bmount\n");
        return -1;
    }
    //Si nbytes = 0 liberar_inodo(), sino mi_truncar_f()
    if (nbytes == 0){
        liberar_inodo(ninodo);       
    }else{ 
        mi_truncar_f(ninodo, nbytes);
    }
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    
    //Configurar la fecha actual para mostrarla
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Guardamos los permisos para mostrarlos
    int permisos = (int)inodo.permisos;

    //Mostrar datos del inodo y su tipo
    printf("\n Datos Inodo: %d\n", ninodo);
    printf("Tipo: %c\n", inodo.tipo);
    printf("Permisos: %d\n", permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("N. links: %d\n", inodo.nlinks);
    printf("Tamaño en bytes lógicos: %d\n", inodo.tamEnBytesLog);
    printf("N. de bloques ocupados: %d\n\n", inodo.numBloquesOcupados);

    //Desmontar dispositivo virtual
    if(bumount() == -1){
        perror("Error en truncar.c -->\nError al cerrar fichero\n");
        return -1;
  }
    return EXIT_SUCCESS;
}
