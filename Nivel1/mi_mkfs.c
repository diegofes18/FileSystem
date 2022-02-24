#include "bloques.h"

char* buf;

// int main(int argc, char **argv){
//     int fd;
//     char* buf;

//     fd=bmount(argv[1]);
//     memset(&buf,0,BLOCKSIZE);
    
//     for(int i=0;argv[2];i++){
//         bwrite(i,&buf);
//     }
//     bumount();
// }
int main(int argc, char **argv){

    //Mirar la sintaxis
    if(argc != 3){
        perror("Sintaxis incorrecta -> ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return EXIT_FAILURE;
    }
    //Obtener parametros del comando
    char *camino=argv[1];
    int nbloques=atoi(argv[2]);
    
}