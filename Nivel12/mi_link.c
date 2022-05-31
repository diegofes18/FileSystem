//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        perror("Sintaxis incorrecta -> ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return -1;
    }
    //Montamos y comprobamos
    if(bmount(argv[1])==-1) {
        perror("Error el montar\n");
        return -1;
    }
    const char *camino_1 = argv[2], *camino_2 = argv[3];
    int err = mi_link(camino_1, camino_2);
    if (err < 0) {
         perror("No se han podido enlazar las rutas.\n");
        return err;
    }
   //Desmontamos el dispostivo virtual
    if (bumount() == -1){

        perror("Error al desmontar\n"); 
        return -1;
    }
    return 0;
}