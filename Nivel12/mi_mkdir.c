//Diego Bermejo, Marc Cañellas y Gaston Panizza
#include "directorios.h"

/*Comando que crea un fichero o directorio llamando
a la funcion mi_creat(). Si la ruta acaba en / sera
un directorio
*/
int main(int argc, char *argv[]){
    
    if(argc != 4){
        perror("Error de sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n");
        return -1;
    }
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7){
        perror("Error en los permisos");
        return -1;
    }
    unsigned char permiso = atoi(argv[2]);

    if((argv[3][strlen(argv[3])-1] == '/')){
        //Montamos el disco
        if (bmount(argv[1]) == -1){
            perror("Error al montar el dispositivo virtual");
            return -1;
        }
        int error;
        if((error = mi_creat(argv[3], permiso)) < 0){
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        
        bumount();
    }
    else{
        perror("La ruta va mal");
    }
    return EXIT_SUCCESS;
}