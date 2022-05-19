//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

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
    //Inodo liberado
    struct STAT p_stat;
    if (mi_stat_f(ninodo, &p_stat)){
        perror( "truncar.c: Error mi_stat_f()\n");
        return -1;
    }
    
    //Configurar la fecha actual para mostrarla
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

    //Información del inodo escrito
    printf("\nDATOS INODO %d:\n", ninodo);
    printf("tipo=%c\n", p_stat.tipo);
    printf("permisos=%d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nLinks= %d\n", p_stat.nlinks);
    printf("tamEnBytesLog= %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados= %d\n", p_stat.numBloquesOcupados);

    //Desmontar dispositivo virtual
    if(bumount() == -1){
        perror("Error en truncar.c -->\nError al cerrar fichero\n");
        return -1;
  }
    return EXIT_SUCCESS;
}
