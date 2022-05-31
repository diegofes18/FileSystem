//Diego Bermejo, Marc Cañellas y Gaston Panizza

#include "simulacion.h"

#define DEBUG 1
int acabados = 0;

void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0){
        acabados++;
    }
}

int main(int argc, char **argv){

    //comprobamos que la sintaxis sea correcta
    if (argc != 2){
        perror("Error de sintaxis: ./simulacion <disco>\n" );
        return -1;
    }

    // Montamos el dispositivo virtual (padre)
    if (bmount(argv[1]) == -1){
        perror("./simulacion: Error al  montar el dispositivo\n");
        exit(0);
    }

    //directorio de simulacion
    char camino[21] = "/simul_";
    time_t time_now;
    time(&time_now);
    struct tm *tm = localtime(&time_now);
    sprintf(camino + strlen(camino), "%d%02d%02d%02d%02d%02d/",tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (mi_creat(camino, 6) == -1){
        fprintf(stderr, "./simulacion: Error al crear el directorio '%s'\n", camino);
        exit(0);
    }

    //asociamos el sigchld al reaper()
    signal(SIGCHLD, reaper);

    pid_t pid;

    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++){
        pid = fork();

        if (pid == 0){
            bmount(argv[1]); //hijo

            //creamos el directorio
            char nombredir[38];
            sprintf(nombredir, "%sproceso_%d/", camino, getpid());

            if (mi_creat(nombredir, 6) < 0){
                fprintf(stderr, "./simulación.c: Error al crear el fichero del proceso\n");
                bumount();
                exit(0);
            }

            //creamos el fichero prueba.dat
            char nombrefich[48];
            sprintf(nombrefich, "%sprueba.dat", nombredir);

            if (mi_creat(nombrefich, 6) < 0){
                fprintf(stderr, "./simulación.c: Error al crear el fichero prueba.dat del proceso\n");
                bumount();
                exit(0);
            }

            //generacion de numeros random
            srand(time(NULL) + getpid());

            for (int nescritura = 0; nescritura < NUMESCRITURAS; nescritura++){

                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura + 1;
                registro.nRegistro = rand() % REGMAX;
                mi_write(nombrefich, &registro, (registro.nRegistro * sizeof(struct REGISTRO)), sizeof(struct REGISTRO));

                usleep(50000); //0,05 seg
            }
#if DEBUG
            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", proceso, NUMESCRITURAS, nombrefich);
#endif
            //Desmontar el dispositivo (hijo)
            bumount();
            exit(0);
        }

        usleep(200000); //0,2 seg
    }

    //bucle para permitir que el padre espere a los hijos
    while (acabados < NUMPROCESOS){
        pause();
    }

    if (bumount() == -1){
        perror("./simulación.c: Error desmontando el dispositivo\n");
        exit(0);
    }

#if DEBUG
    fprintf(stderr, "Total de procesos terminados: %d\n", acabados);
#endif

    return EXIT_SUCCESS;
}

