//MARC CAÑELLAS, DIEGO BERMEJO, GASTON PANIZZA

#include "directorios.h"
#define DEBUG 0
#define DEBUG9 0

static struct UltimaEntrada UltimaEntrada[CACHE];
int MAX=CACHE;

/*
Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){

    //error si se empieza con /
    if (camino[0] != '/'){
        return -1;
    }

    //sumamos 1 para evitar la primera /
    char *rest = strchr((camino + 1), '/');
    strcpy(tipo, "f");

    //si encontramos '/'
    if (rest){

        //copiamos todo en inicial menos el resto
        strncpy(inicial, (camino + 1), (strlen(camino) - strlen(rest) - 1));
        //copiamos el resto en final
        strcpy(final, rest);

        if (final[0] == '/'){
            strcpy(tipo, "d");
        }

    }else {
        strcpy(inicial, (camino + 1));
        strcpy(final, "");
    }
    

    return EXIT_SUCCESS;
}



/*
Esta función nos buscará una determinada entrada entre las entradas del inodo correspondiente a su directorio padre. 
Dada una cadena de caracteres y el nº de inodo del directorio padr, donde buscar la entrada en cuestión, obtiene:
-El número de inodo (*p_inodo) al que está asociado el nombre de la entrada buscada.
-El número de entrada  (*p_entrada) dentro del inodo *p_inodo_dir que lo contiene (empezando por 0).
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

    struct entrada entrada;
    struct inodo dir_inodo;
    char inicial[sizeof(entrada.nombre)];      
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    
    //el directorio es raiz
    if(!strcmp(camino_parcial,"/")){
        struct superbloque SB;
        bread(posSB, &SB);
        *(p_inodo)=SB.posInodoRaiz;
        *(p_entrada)=0;
        return EXIT_SUCCESS;
    } 

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));

    if(extraer_camino(camino_parcial, inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUG
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial,final, reservar);
#endif


    //busqueda de la entrada 
    leer_inodo(*(p_inodo_dir), &dir_inodo);

    //permiso de lectura.
    if ((dir_inodo.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }     

    //inicializar el buffer de lectura con 0s
    struct entrada buffer[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer, 0, (BLOCKSIZE/sizeof(struct entrada))*sizeof(struct entrada));

    //cantidad de entradas que contiene el inodo
    cant_entradas_inodo=dir_inodo.tamEnBytesLog/sizeof(struct entrada);
    //nº de entrada inicial
    num_entrada_inodo=0;
    

    int b_leidos = 0;
    if (cant_entradas_inodo > 0){

        b_leidos += mi_read_f(*p_inodo_dir, &buffer, b_leidos, BLOCKSIZE);

        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, buffer[num_entrada_inodo].nombre) != 0)){

            num_entrada_inodo++;

            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0){
                b_leidos += mi_read_f(*p_inodo_dir, &buffer, b_leidos, BLOCKSIZE);
            }
        }
    }

    //si inicial distinto de entrada.nombre
    if(strcmp(buffer[num_entrada_inodo].nombre, inicial) != 0){
        
        switch(reservar){

            //modo consulta. Como no existe retornamos error
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
                break; 

            //modo escritura
            case 1:
            
                if(dir_inodo.tipo=='f'){
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

                }
                //comprobar que tiene permiso de escritura 
                if ((dir_inodo.permisos & 2)!=2){
                
                    return ERROR_PERMISO_LECTURA;
                }
                else{
                    strcpy(entrada.nombre,inicial);
                   
                    if(tipo == 'd'){
                        
                        if(strcmp(final,"/")==0){

                            entrada.ninodo=reservar_inodo('d', 6);
                            
#if DEBUG
                        printf("[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                        }

                        else{

                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }

                    else{

                        entrada.ninodo=reservar_inodo('f',6);

#if DEBUG
                    printf("[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif

                        
                    }


#if DEBUG
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif


                    if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1){
                        
                        if(entrada.ninodo!=-1){
                            liberar_inodo(entrada.ninodo);

#if DEBUG
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif

                        }
                        return -1;
                    }
                }
                
        

        }
    }

    //hemos llegado al final del camino
    if(!strcmp(final,"/")|| !strcmp(final,"")){
        if((num_entrada_inodo<cant_entradas_inodo)&& (reservar==1)){
             //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        //cortamos la recursividad
        *(p_inodo) = buffer[num_entrada_inodo].ninodo;
        *(p_entrada) = num_entrada_inodo;

        return EXIT_SUCCESS;
        
    }else{
        *(p_inodo_dir) = buffer[num_entrada_inodo].ninodo;
        return buscar_entrada(final,p_inodo_dir,p_inodo, p_entrada, reservar, permisos);
    }

    return EXIT_SUCCESS;

 }


/*
Se encarga de mostrar un error por pantalla
*/
void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}

/*
Función de la capa de directorios que crea un fichero/directorio y su entrada de directorio.
*/
int mi_creat(const char *camino, unsigned char permisos){
    //variables
    unsigned int p_inodo_dir = 0; //suponemos que p_inodo_dir vale 0
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    //usamos la función ebuscar_entrada con reservar=1
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0){
        return error;
    }
    return EXIT_SUCCESS;
}


/*
Función de la capa de directorios que pone el contenido del directorio 
en un buffer de memoria y devuelve el número de entradas
*/
int mi_dir(const char *camino, char *buffer, char *tipo){
    struct tm *tm;
    //variables
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    int nEntradas = 0;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); //Permisos para leer
    if (error < 0){
        mostrar_error_buscar_entrada(error);
        return error;
    }

    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == -1){
        return -1;
    }

    if ((inodo.permisos & 4) != 4){
        return -1;
    }

    struct entrada entrada;

    char tmp[100];       
    char tamEnBytes[10]; 

    if (camino[(strlen(camino)) - 1] == '/'){
        if (leer_inodo(p_inodo, &inodo) == -1){
            return -1;
        }

        *tipo = inodo.tipo;

        //Buffer de salida
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        memset(&entradas, 0, sizeof(struct entrada));

        nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        int offset = 0;
        offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

        //Leemos todos las entradas
        for (int i = 0; i < nEntradas; i++){
            //Leer el inodo correspndiente
            if (leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == -1){
                return -1;
            }

            //Tipo
            if (inodo.tipo == 'd'){
                strcat(buffer, "d");
            }

            else{
                strcat(buffer, "f");
            }

            strcat(buffer, "\t");

            //Permisos
            strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
            strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
            strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
            strcat(buffer, "\t");

            //mTime
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            //Tamaño
            sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
            strcat(buffer, tamEnBytes);
            strcat(buffer, "\t");

            //Nombre
            strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            while ((strlen(buffer) % TAMFILA) != 0){
                strcat(buffer, " ");
            }

            strcat(buffer, "\n"); //Preparamos el string para la siguiente entrada

            if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0){
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }

    }else{ //es un archivo
        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));
        leer_inodo(entrada.ninodo, &inodo);
        *tipo = inodo.tipo;

        
        if (inodo.tipo == 'd'){
            strcat(buffer, "d");

        }else{
            strcat(buffer, "f");
        }

        strcat(buffer, "\t");

        //permisos
        strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
        strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
        strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
        strcat(buffer, "\t");

        //mTime
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        //Tamaño
        sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytes);
        strcat(buffer, "\t");

        //Nombre
        strcat(buffer, entrada.nombre);
        while ((strlen(buffer) % TAMFILA) != 0){
            strcat(buffer, " ");
        }

        //Siguiente
        strcat(buffer, "\n");

    }
    
    return nEntradas;
}

/* 
Se encarga de buscar la entrada *camino con buscar_entrada() 
para obtener el nº de inodo (p_inodo).
*/
int mi_chmod(const char *camino, unsigned char permisos){

    //variables.
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);

    if (error < 0){
        return error;
    }

    //si la entrada existe llamamos a la función correspondiente 
    //de ficheros.c pasándole el p_inodo
    mi_chmod_f(p_inodo, permisos);

    return EXIT_SUCCESS;
}


/*
Se encarga de buscar la entrada *camino con buscar_entrada() 
para obtener el p_inodo.
*/
int mi_stat(const char *camino, struct STAT *p_stat){

    //variables.
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if (r < 0){
        return r;
    }

    if (mi_stat_f(p_inodo, p_stat) == -1){
        return -1;
    }

    return p_inodo;
}


/*
Función de directorios.c para leer los nbytes del fichero indicado por camino, 
a partir del offset pasado por parámetro y copiarlos en el buffer buf.
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    //variables.
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error=0;
    int leidos=0;
    int aux=0;	
    
    //comprobamos si leemos un inodo anterior
    for(int i=0; i<(MAX-1);i++){
        if(strcmp(camino,UltimaEntrada[i].camino)==0){
            p_inodo=UltimaEntrada[i].p_inodo;
            aux=1;
        
#if DEBUG9
            perror("[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
#endif
            break;

        }

    }

    if(!aux){
        error=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);

        if(error<0){
            return error;
        }

        //si el cache no esta lleno 
        if(MAX>0){
            strcpy(UltimaEntrada[CACHE - MAX].camino, camino);
            UltimaEntrada[CACHE - MAX].p_inodo = p_inodo;
            MAX--;

#if DEBUG9
            perror("[mi_read() → Actualizamos la caché de lectura]\n");
#endif

        }else{
            for(int j=0;j<(CACHE-1);j++){
                strcpy(UltimaEntrada[j].camino, UltimaEntrada[j+1].camino);
                UltimaEntrada[j].p_inodo = UltimaEntrada[j+1].p_inodo ;
            }

            strcpy(UltimaEntrada[CACHE-1].camino, camino);
            UltimaEntrada[CACHE-1].p_inodo = p_inodo;

#if DEBUG9
            perror("[mi_read() → Actualizamos la caché de lectura]\n");
#endif
        }

    }

    // Realiza la lectura del archivo.
    leidos=mi_read_f(p_inodo, buf, offset, nbytes);
    if (leidos == -1){
        return ERROR_PERMISO_LECTURA;
    }

    return leidos;
}


/*
Función de directorios.c para escribir contenido en un fichero. 
Buscaremos la entrada camino con buscar_entrada() para obtener el p_inodo
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){

    //variables
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error=0;
    int escritos=0;
    int aux=0;

    //Recorrido del cache para comprobar si la escritura es sobre un inodo anterior
    for (int i=0; i<(MAX-1); i++){
        if (strcmp(camino, UltimaEntrada[i].camino) == 0){
            p_inodo = UltimaEntrada[i].p_inodo;
            aux=1;
            break;
        }
    }

    if(!aux){
        //obtenemos el inodo con el metodo buscar_entrada con los permisos de lectura (4)
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); 
        if (error<0){
            return error;
        }

        //si el cahce no está lleno
        if(MAX>0){
            strcpy(UltimaEntrada[CACHE-MAX].camino, camino);
            UltimaEntrada[CACHE-MAX].p_inodo = p_inodo;
            MAX--;
#if DEBUG9
            perror("[mi_write() → Actualizamos la caché de escritura]\n");
#endif

        }else{
            for (int i=0; i<CACHE-1; i++){
                strcpy(UltimaEntrada[i].camino, UltimaEntrada[i+1].camino);
                UltimaEntrada[i].p_inodo=UltimaEntrada[i+1].p_inodo;
            }

            strcpy(UltimaEntrada[CACHE-1].camino, camino);
            UltimaEntrada[CACHE-1].p_inodo = p_inodo;

#if DEBUG9
            perror("[mi_write() → Actualizamos la caché de escritura]\n");
#endif
        }
    }

    //escritura en el archivo
    escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    if (escritos == -1){
        escritos=0;
    }

    return escritos;
    
}

/*
Crea el enlace de una entrada de directorio camino2 al inodo 
 especificado por otra entrada de directorio camino1.
*/
int mi_link(const char *camino1, const char *camino2){
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct entrada entrada;
    struct inodo inodo;
    if (buscar_entrada(camino1, &p_inodo_dir, &p_inodo, &p_entrada, 0, '0') < 0){
        perror("Entrada de camino 1 no existe de mi_link\n");
    }
    int ninodo = p_inodo;
	if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error en leer inodo de mi_link\n");
        return -1;
    }
    if (inodo.tipo != 'f' && (inodo.permisos & 4) != 4){
        perror("No tiene permisos de lectura en mi_link\n");
        return -1;
    }
	p_inodo_dir = 0; //para poder volver a usarla

    if (buscar_entrada(camino2, &p_inodo_dir, &p_inodo, &p_entrada, 1, '6') < 0){
        perror("Entrada ya existe\n");
    }
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1){
        perror("Error en mi_read_f de mi_link\n");      
    }
	liberar_inodo(entrada.ninodo);
    entrada.ninodo = ninodo;

    if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1){
        perror("Error en mi_write_f de mi_link\n");       
        return -1;
    }
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error en leer inodo de mi_link\n");
        return -1;
    }
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    escribir_inodo(ninodo, inodo);
    return 0;
} 

int mi_unlink(const char *camino){

    //variables
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;

    //buscamos el archivo
    error=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);

    if(error<0){
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    //inodo que hay que borrar
    struct inodo inodo;

    if(leer_inodo(p_inodo, &inodo) == -1){
        return -1;
    }

    if((inodo.tipo == 'd')&&(inodo.tamEnBytesLog>0)){
        return -1;
    }

    //inodo del directorio
    struct inodo inodo_directorio;

    if(leer_inodo(p_inodo_dir, &inodo_directorio) == -1){
        return -1;
    }

    //numero de entradas 
    int entradas= inodo_directorio.tamEnBytesLog/sizeof(struct entrada);

    //HAY QUE ELIMINAR LA ÚLTIMA ENTRADA

    //si la entrada no es laultima
    if(p_entrada!= (entradas -1) ){
            struct entrada entrada;
            if(mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada)*(entradas -1), sizeof(struct entrada)) == -1){
                return -1;
            }

            if(mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada)*(p_entrada), sizeof(struct entrada)) == -1){
                return -1;
            }
            
    }

    //si la entrada es la ultima, eliminamos
    if(mi_truncar_f(p_inodo_dir, sizeof(struct entrada)*(entradas -1))==-1){
        return -1;
    }

    inodo.nlinks--;

    //no hay enlaces
    if(!inodo.nlinks){
        //liberamos inodo
        if(liberar_inodo(p_inodo)==-1){
            return -1;
        }

    }else{
        //actualizamos
        inodo.ctime=time(NULL);
        if(escribir_inodo(p_inodo, inodo)==-1){
            return -1;
        }

    }

    return EXIT_SUCCESS;

}