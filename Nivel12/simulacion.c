// Diego Bermejo, Marc Cañellas y Gaston Panizza

#include "simulacion.h"

#define DEBUG 1
int acabados = 0;

//funcion enterrador
void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
#if DEBUG
        fprintf(stderr, "[simulación.c → Acabado proceso con PID %d, realizadas %d escrituras por el proceso numero: %d\n", ended, NUMESCRITURAS, acabados);
#endif
  }
}

int main(int argc, char const *argv[]){

    //asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    //comprobamos la sintaxis del comando
    if (argc != 2){
       perror("Error de sintaxis: ./simulacion <disco>\n" );
        return -1;
    }

    //montamos el dispositivo (padre)
    if (bmount(argv[1]) == -1){
        return -1;
    }

    //creamos el directorio de simulacion: /simul_aaaammddhhmmss/

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *tiempo = malloc(14);
    sprintf(tiempo, "%d%02d%02d%02d%02d%02d", tm.tm_year + 1900,tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char *camino = malloc(14 + 8);
    strcat(strcpy(camino, "/simul_"), tiempo);
    strcat(camino, "/");
    fprintf(stderr, "Directorio de simulación:%s \n", camino);
    
    char buf[80];
    strcpy(buf, camino);

    fprintf(stderr, "Buffer de simulación:%s \n", buf);

    if((mi_creat(camino,6))<0){
        return -1;
    }

    for(int proceso=1; proceso<=NUMPROCESOS; proceso++){ //Para proceso:=1 hasta proceso<=NUMPROCESOS hacer
        pid_t pid = fork();
        if(pid==0){ //si es el hijo

            //montamos el dispositivo para el hijo
            bmount(argv[1]);

            char nombredir[80];
            //sprintf(nombredir, "%sproceso_%d/", buf, getpid());

            //creamos el directorio del proceso añadiendo el PID al nombre
            if (mi_creat(nombredir, 6) < 0){
                bumount();
                exit(0);
            }

            //creamos el fichero prueba.dat
            char nfichero[100];
            sprintf(nfichero, "%sprueba.dat", nombredir);

            if (mi_creat(nfichero, 4) < 0){
                bumount();
                exit(0);
            }

            fprintf(stderr, "Fichero del proceso %i creado\n", proceso);

            //inicializamos la semilla de numeros aleatorios
            srand(time(NULL) + getpid());

            for(int escritura=1;escritura<=NUMESCRITURAS; escritura++){ //Para nescritura:=1 hasta nescritura<=NUMESCRITURAS hacer

                //inicializamos el registro
                struct REGISTRO r;

                r.fecha = time(NULL);
                r.pid = getpid();
                r.nEscritura = escritura;
                r.nRegistro = rand() % REGMAX;

                //escribimos el registro
                mi_write(nfichero, &r, r.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));

#if DEBUG
                fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", escritura, buf);
#endif
                //esperar 0,05 seg para hacer la siguiente escritura
                usleep(50000);

            }
            fprintf(stderr, "Directorio de simulación:%s \n", camino);
            //desmontamos el dispositivo
            bumount(); //hijo
            exit(0); //necesario para que se emita la señal SIGCHLD
        }

        usleep(200000); //esperar 0,15 seg para lanzar siguiente proceso
    }

     //permitimos que el padre espere por todos los hijos
    while (acabados<NUMPROCESOS){
        pause();
    }

    if (bumount() < 0){ //padre
        perror("Error al desmontar el dispositivo\n");
        exit(0);
    }

    fprintf(stderr, "Total de procesos terminados: %d\n", acabados);

}

