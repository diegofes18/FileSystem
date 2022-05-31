//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA
#include "directorios.h"

int main(int argc, char const **argv) {
    if (argc != 3) {
        perror("Sintaxis incorrecta -> ./mi_rm <disco> </ruta>");
        return -1;
    }
    const char *camino = argv[2];
    int err;
     //Montamos y comprobamos
    if(bmount(argv[1])==-1) {
        perror("Error el montar\n");
        return -1;
    }

    err = mi_unlink(camino);
    if (err < 0) {
        perror("No se ha podido borrar la entrada\n");
        return err;
    }
    //Desmontamos el dispostivo virtual
    if (bumount() == -1){

        perror("Error al desmontar\n"); 
        return -1;
    }
    return EXIT_SUCCESS;
}