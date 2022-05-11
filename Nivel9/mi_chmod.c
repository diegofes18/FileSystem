//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

int main(int argc, char **argv){
    
    unsigned char permisos;
    int r;

    if(argc != 4){
        perror("Error de sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }

    permisos = atoi(argv[2]);
    if(permisos > 7){
        perror("Error de sintaxis: los permisos estan mal \n");
        return -1;
    }
    //Montamos el disco
    if(bmount(argv[1]) == -1){
        perror("Error al montar el disco\n");
        return -1;
    }
    r = mi_chmod(argv[3], permisos);
    if(r < 0){
        mostrar_error_buscar_entrada(r);
        return -1;
    }
    bumount();
    return -1;
}
