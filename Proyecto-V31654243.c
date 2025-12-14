//16-10-2025
//Gabriel Becerra
//C.I. 31.654.243
//Seccion 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

///////////////
//Estructuras//
///////////////

typedef struct {
    char nombre[51];
    int min;
    int seg;
    int punt;          // Popularidad de la cancion del 1 al 100
    int ultimaHora;    // Para controlar repeticiones
} cancion;

typedef struct {
    char empresa[31];
    int duracionSeg;
    int repeticiones;  
    int repActual;     // Para control interno
} publicidad;

typedef struct {
    char nombre[101];
    int durMin;
    int durSeg;
    int segmentos;
    int preferencia;   // Rating del show del 1 al 10
    int usadoHoy;      // Para no repetir mas de una vez por dia
} show;

typedef struct {
    int tiempo;      
    char tipo;         // 'S' show, 'C' cancion, 'P' publicidad
    int dia;           //1-Lunes, 2-Martes...
    char nombre[120];
} evento;              //Para consulta de usuario al final del programa

/////////////////////////////////////
//Funciones para lectura de entrada//
/////////////////////////////////////

void leerCanciones(cancion *canciones,int *nroCanciones){
    FILE *in = fopen("canciones.in","r");
    if(!in){
        perror("Error con archivo de Canciones"); //Validamos la existencia del archivo
        exit(1);
    }

    fscanf(in," %d",nroCanciones);
    fgetc(in); //Consumimos el salto de Linea

    for(int i=0;i<(*nroCanciones);i++){
        //Variables auxiliares
        char nombreTemp[100];
        int min,sec,punt;

        if(fscanf(in,"%[^\t]\t%d\t%d\t%d\n",nombreTemp,&min,&sec,&punt) != 4){ 
            //Leemos nombre hasta el primer \t
            printf("Error de lectura en la linea %d de canciones.in\n",i+1);
            continue;
        }

        strcpy(canciones[i].nombre,nombreTemp);
        canciones[i].min = min;
        canciones[i].seg = sec;
        canciones[i].punt = punt;

        canciones[i].ultimaHora = -9999; // La inicializamos como no usada

    }
    fclose(in);
}

void leerPublicidad(publicidad *cunas,int *nroCunas){
    FILE *in = fopen("publicidad.in","r");
    if(!in){
        perror("Error con archivo de publicidad"); //Validamos la existencia del archivo
        exit(1);
    }

    char nombreTemp[100];
    int segs, veces;
    *nroCunas = 0;

    while(fscanf(in," %[^\t]\t%d\t%d\n",nombreTemp,&segs,&veces) == 3){

        strcpy(cunas[*nroCunas].empresa, nombreTemp);
        cunas[*nroCunas].duracionSeg = segs;
        cunas[*nroCunas].repeticiones = veces;
        cunas[*nroCunas].repActual = 0;
        (*nroCunas)++;
    }
    fclose(in);
}

void leerShows(show *shows,int *nroShows){
    FILE *in = fopen("shows.in","r");
    if(!in){
        perror("Error con archivo de shows"); //Validamos la existencia del archivo
        exit(1);
    }

    fscanf(in, " %d",nroShows); 

    
    for (int i=0;i<*nroShows;i++){
        //Variables auxiliares
        char nombreTemp[150];
        int min, seg, segmentos, pref;
        
        int r = fscanf(in," %[^\t]\t%d\t%d\t%d\t%d",nombreTemp,&min,&seg,&segmentos,&pref);

        if(r == EOF) break;
        if(r != 5){
            printf("Error lectura en la linea %d de shows.in\n",i+1);
            continue;
        }

        strcpy(shows[i].nombre, nombreTemp);
        shows[i].durMin = min;
        shows[i].durSeg = seg;
        shows[i].segmentos = segmentos;
        shows[i].preferencia = pref;
        shows[i].usadoHoy = 0;
    }
    fclose(in);
}

//////////////////////////////////////////////////////
//Funciones auxiliares y de programacion del horario//
//////////////////////////////////////////////////////

void convertirTiempo(int totalseg, int *hh, int *mm, int *ss){ 
    *hh = totalseg / 3600; //Funcion auxiliar para manejar el tiempo en segundos
    totalseg %= 3600;
    *mm = totalseg / 60;
    *ss = totalseg % 60;
}

void escribirEvento(FILE *out,int tiempo,char tipo,const char *nombre,evento *programacion,int *nroEventos,int dia){
    int hh, mm, ss;
    convertirTiempo(tiempo, &hh, &mm, &ss); 
    // Guardar en el archivo de salida
    fprintf(out,"%02d:%02d:%02d %c %s\n",hh, mm, ss, tipo, nombre);

    // Guardar en la estructura de eventos para consulta del usuario
    programacion[*nroEventos].tiempo = tiempo;
    programacion[*nroEventos].tipo = tipo;
    strcpy(programacion[*nroEventos].nombre, nombre);
    programacion[*nroEventos].dia = dia;

    (*nroEventos)++; // aumentar contador de eventos
}

void elegirShowsEstelares(show shows[], int nroShows, int estelares[3]){ 
    //Funcion para escoger los shows de mas alto rating para los horarios con mas escucha

    // Inicializar con -1
    for(int i = 0; i < 3; i++) estelares[i] = -1;

    //Seleccionamos los top 3 shows por preferencia
    for(int s = 0; s < 3; s++){
        int mejor = -1;
        int mejorPref = -1;
        for (int i = 0; i < nroShows; i++){
            int yaElegido = 0;
            for(int k = 0; k < s; k++){
                if (estelares[k] == i) yaElegido = 1;
            }
            if(yaElegido) continue;

            if(shows[i].preferencia > mejorPref){
                mejorPref = shows[i].preferencia;
                mejor = i;
            }
        }
        estelares[s] = mejor;
    }
}

int elegirShowAleatorio(show shows[], int nroShows, int duracionRestante){
    //Funcion para cuando toque meter un show en la programacion del dia

    int candidatos[100];
    int pesos[100];
    int n = 0, total = 0;

    for(int i = 0; i < nroShows; i++){
        if(shows[i].usadoHoy) continue; // no repetir el mismo dia

        int segDur = shows[i].durMin * 60 + shows[i].durSeg;
        int tiempoNec = segDur * shows[i].segmentos;
        if(tiempoNec <= duracionRestante){
            candidatos[n] = i;
            pesos[n] = shows[i].preferencia; // prob mas alta para shows de rating alto
            total += pesos[n];
            n++;
        }
    }

    if(n == 0) return -1; // no hay shows que quepan

    // sorteo ponderado
    int r = rand() % total;
    for(int i = 0; i < n; i++){
        if(r < pesos[i]) return candidatos[i];
        r -= pesos[i];
    }

    return candidatos[n-1]; // ultimo recurso
}

int elegirPublicidad(publicidad pubs[], int nroPubs) {
    int pesos[100], total = 0;

    for(int i = 0; i < nroPubs; i++){
        int faltan = pubs[i].repeticiones - pubs[i].repActual;
        pesos[i] = (faltan > 0) ? faltan : 1; // prioridad a las que faltan mas
        total += pesos[i];
    }

    int r = rand() % total;
    for(int i = 0; i < nroPubs; i++){
        if(r < pesos[i]) return i;
        r -= pesos[i];
    }
    return 0; //ultimo recurso (no retorna nada)
}

int elegirCancionparaSeparador(cancion canciones[], int nroCan, int tiempoActual){
    //Funcion para meter una cancion entre segmentos de show o a lo largo del dia

    int pesos[1000];
    int total = 0;

    for(int i=0; i<nroCan; i++){
        int ultima = canciones[i].ultimaHora;
        if(ultima < 0 || tiempoActual - ultima >= 4*3600){
            pesos[i] = canciones[i].punt;  // peso proporcional al rating
        } 
        else{
            pesos[i] = 0; // no disponible
        }
        total += pesos[i];
    }

    if(total == 0){
        // ninguna cumple entonces no podemos insertar una cancion
        return -1;
    }

    //Sorteo ponderado
    int r = rand() % total;
    for (int i=0; i<nroCan; i++) {
        if (r < pesos[i]) return i;
        r -= pesos[i];
    }
    return -1;
}

int duracionCancionSeg(cancion c){ //Auxiliar para manejar en segundos
    return c.min*60 + c.seg;
}

int insertarShowsDistribuidos(int tiempoActual, int tiempoFin, int dia,cancion *canciones, int nroCan,publicidad *pubs, int nroPub,show *shows, int nroShows,int estelares[3],evento *programacion, int *nroEventos, FILE *out){
    static int pubsSeguidas = 0;
    int duracionBloque = tiempoFin - tiempoActual;
    int nroShowsNormales = 0;
    for(int i=0;i<nroShows;i++){
        int esEstelar = 0;
        for(int e=0;e<3;e++)
            if(estelares[e] == i) esEstelar = 1;
            if(!esEstelar && !shows[i].usadoHoy)
            nroShowsNormales++;
        
    }

    // Si no hay shows normales, rellenamos todo el bloque con canciones y publicidades balanceadas
    if (nroShowsNormales == 0){

        while(tiempoActual < tiempoFin){
            int r = rand() % 4; // 0-1 = canción, 2-3 = publicidad

            if((r < 2 || pubsSeguidas >= 3) && nroCan > 0){
                int idxCan = elegirCancionparaSeparador(canciones, nroCan, tiempoActual);
                if(idxCan >= 0){
                    int d = duracionCancionSeg(canciones[idxCan]);
                    if(tiempoFin - tiempoActual >= d + 1){
                        escribirEvento(out, tiempoActual, 'C', canciones[idxCan].nombre, programacion, nroEventos, dia);
                        canciones[idxCan].ultimaHora = tiempoActual;
                        tiempoActual += d + 1;
                        pubsSeguidas = 0;
                        continue;
                    }
                }
            }

            if(nroPub > 0){
                int idxPub = elegirPublicidad(pubs, nroPub);
                int d = pubs[idxPub].duracionSeg;
                if(tiempoFin - tiempoActual >= d + 1){
                    escribirEvento(out, tiempoActual, 'P', pubs[idxPub].empresa, programacion, nroEventos, dia);
                    pubs[idxPub].repActual++;
                    tiempoActual += d + 1;
                    pubsSeguidas++;
                    continue;
                }
            }
            //ultimo recurso: avanzar solo 1 segundo
            tiempoActual += 1;
        }
        return tiempoActual;
    }


    // Caso normal con shows disponibles
    int intervalo = duracionBloque / nroShowsNormales;
    if(intervalo <= 0) intervalo = duracionBloque;
    int proximoShow = tiempoActual + intervalo / 2;

    while(tiempoActual < tiempoFin){
        int durRestante = tiempoFin - tiempoActual;

        // verificamos si tocaria insertar show
        if(tiempoActual >= proximoShow && nroShowsNormales > 0){
            int candidatos[50], n = 0;
            for(int i=0;i < nroShows;i++){
                int esEstelar = 0;
                for(int e=0;e<3;e++)
                    if(estelares[e] == i) esEstelar = 1;
                    if(esEstelar || shows[i].usadoHoy)
                    continue;


                int totalSeg = shows[i].durMin * 60 + shows[i].durSeg;
                int segs = shows[i].segmentos;
                int totalDur = segs * totalSeg + (segs - 1) * 2; // separadores
                if(totalDur <= durRestante)
                    candidatos[n++] = i;
            }

            if(n == 0){
                // No cabe ningun show entonces seguimos rellenando normalmente
                proximoShow = tiempoActual + 1800; // reintenta en 30 min
                continue;
            }

            int idx = candidatos[rand() % n];
            shows[idx].usadoHoy = 1;
            nroShowsNormales--;

            int segDur = shows[idx].durMin * 60 + shows[idx].durSeg;
            for(int s=1;s <= shows[idx].segmentos && tiempoActual < tiempoFin; s++){
                if(tiempoFin - tiempoActual < segDur) break;

                escribirEvento(out, tiempoActual, 'S', shows[idx].nombre, programacion, nroEventos, dia);
                tiempoActual += segDur + 1;

                if(s < shows[idx].segmentos && tiempoActual < tiempoFin){
                    // entre segmentos intercalamos algo
                    int r = rand() % 3;
                    if(r == 0 && nroCan > 0){
                        int idxCan = elegirCancionparaSeparador(canciones, nroCan, tiempoActual);
                        if(idxCan >= 0){
                            int d = duracionCancionSeg(canciones[idxCan]);
                            if(tiempoFin - tiempoActual >= d + 1){
                                escribirEvento(out, tiempoActual, 'C', canciones[idxCan].nombre, programacion, nroEventos, dia);
                                canciones[idxCan].ultimaHora = tiempoActual;
                                tiempoActual += d + 1;
                                continue;
                            }
                        }
                    }
                    int idxPub = elegirPublicidad(pubs, nroPub);
                    int d = pubs[idxPub].duracionSeg;
                    if(tiempoFin - tiempoActual >= d + 1){
                        escribirEvento(out, tiempoActual, 'P', pubs[idxPub].empresa, programacion, nroEventos, dia);
                        pubs[idxPub].repActual++;
                        tiempoActual += d + 1;
                    }
                }
            }
            proximoShow = tiempoActual + intervalo;
        }
        else{
            // Relleno entre shows con equilibrio entre canciones y publicidades
            int r = rand() % 4; // 0-1 = canción, 2-3 = publicidad

            if((r < 2 || pubsSeguidas >= 3) && nroCan > 0){
                int idxCan = elegirCancionparaSeparador(canciones, nroCan, tiempoActual);
                if(idxCan >= 0){
                    int d = duracionCancionSeg(canciones[idxCan]);
                    if(tiempoFin - tiempoActual >= d + 1){
                        escribirEvento(out, tiempoActual, 'C', canciones[idxCan].nombre, programacion, nroEventos, dia);
                        canciones[idxCan].ultimaHora = tiempoActual;
                        tiempoActual += d + 1;
                        pubsSeguidas = 0;
                        continue;
                    }
                }
            }
            if(nroPub > 0){
                int idxPub = elegirPublicidad(pubs, nroPub);
                int d = pubs[idxPub].duracionSeg;
                if(tiempoFin - tiempoActual >= d + 1){
                    escribirEvento(out, tiempoActual, 'P', pubs[idxPub].empresa, programacion, nroEventos, dia);
                    pubs[idxPub].repActual++;
                    tiempoActual += d + 1;
                    pubsSeguidas++;
                    continue;
                }
            }
            tiempoActual += 1; // solo si no cupo nada
        }
    }
    return tiempoActual;
}

void ProgramarDia(int dia, cancion *canciones, int nroCan, publicidad *pubs, int nroPub,show *shows, int nroShows, evento *programacion, int *nroEventos){
    // Funcion principal para el llenado de la programacion diaria
    char archivoSalida[40];
    switch(dia){
        case 1: 
            strcpy(archivoSalida,"grilla_lunes.out"); 
            break;
        case 2: 
            strcpy(archivoSalida,"grilla_martes.out"); 
            break;
        case 3: 
            strcpy(archivoSalida,"grilla_miercoles.out");
            break;
        case 4: 
            strcpy(archivoSalida,"grilla_jueves.out"); 
            break;
        case 5: 
            strcpy(archivoSalida,"grilla_viernes.out"); 
            break;
        case 6: 
            strcpy(archivoSalida,"grilla_sabado.out"); 
            break;
        case 7: 
            strcpy(archivoSalida,"grilla_domingo.out"); 
            break;
    }

    FILE *out = fopen(archivoSalida,"w");
    if(!out){
        printf("Error creando archivo de salida %s\n",archivoSalida);
        return;
    }

    int tiempoActual;
    if(dia == 1){
        tiempoActual = 5*60; //Empieza en 00:05:00
    }
    else{
        tiempoActual = 0;
    }


    //Reiniciamos todo para el inicio del nuevo dia
    for(int i = 0; i < nroCan; i++){
        canciones[i].ultimaHora = -9999;
    }
    for(int i = 0; i < nroPub; i++){
        pubs[i].repActual = 0;
    }
    for(int i = 0; i < nroShows; i++){
        shows[i].usadoHoy = 0;
    }

    // Elegir shows estelares
    int estelares[3];
    elegirShowsEstelares(shows, nroShows, estelares);
    for(int i = 0; i < 3; i++){
        if(estelares[i] >= 0) shows[estelares[i]].usadoHoy = 1; //Marcarlos como usados
    }

    //mezclar aleatoriamente el orden de los shows estelares
    for (int i = 0; i < 3; i++) {
        int j = rand() % 3;
        int temp = estelares[i];
        estelares[i] = estelares[j];
        estelares[j] = temp;
    }


    // Bloques fijos de estelares
    int franjas[3][2] = {
        {7*3600, 9*3600},
        {12*3600, 14*3600},
        {18*3600, 19*3600}
    };

    for(int f = 0; f < 3; f++){
        int inicio = franjas[f][0];
        int fin    = franjas[f][1];

        //Rellenar con shows normales hasta inicio de franja
        tiempoActual = insertarShowsDistribuidos(tiempoActual, inicio,dia,canciones,nroCan,pubs,nroPub,shows,nroShows,estelares,programacion,nroEventos,out);
        if(tiempoActual < inicio) tiempoActual = inicio;

        //Programar show estelar
        int idx = estelares[f];
        if(idx >= 0){
            int segDur = shows[idx].durMin*60 + shows[idx].durSeg;
            int segs   = shows[idx].segmentos;

            for(int s = 1; s <= segs && tiempoActual < fin; s++){
                escribirEvento(out, tiempoActual,'S',shows[idx].nombre,programacion,nroEventos,dia);
                tiempoActual += segDur;

                if(s < segs && tiempoActual < fin){
                    tiempoActual += 1;
                    int idxPub = elegirPublicidad(pubs, nroPub);
                    if(idxPub >= 0){
                        escribirEvento(out, tiempoActual,'P',pubs[idxPub].empresa,programacion,nroEventos,dia);
                        pubs[idxPub].repActual++;
                        tiempoActual += pubs[idxPub].duracionSeg;
                    }
                    tiempoActual += 1;

                    int idxCan = elegirCancionparaSeparador(canciones, nroCan, tiempoActual);
                    if(idxCan >= 0){
                        escribirEvento(out, tiempoActual,'C',canciones[idxCan].nombre,programacion,nroEventos,dia);
                        canciones[idxCan].ultimaHora = tiempoActual;
                        tiempoActual += duracionCancionSeg(canciones[idxCan]);
                    }
                    tiempoActual += 1;
                }
            }
        }
        //Llenar hasta fin de franja
        tiempoActual = insertarShowsDistribuidos(tiempoActual, fin,dia,canciones,nroCan,pubs,nroPub,shows,nroShows,estelares,programacion,nroEventos,out);

    }
    //Rellenar hasta medianoche
    tiempoActual = insertarShowsDistribuidos(tiempoActual, 24*3600,dia,canciones,nroCan,pubs,nroPub,shows,nroShows,estelares, programacion,nroEventos,out);
    fclose(out);

    
}

////////////////////////////////////////
//Funciones para Consultas del usuario//
////////////////////////////////////////

void consultarPorHora(evento *programacion, int nroEventos,int dia){
    int hh, mm, ss;
    printf("Ingrese la hora a consultar (HH MM SS): ");
    scanf("%d %d %d", &hh, &mm, &ss);

    int tiempo = hh*3600 + mm*60 + ss;
    int encontrado = 0;
    evento ultimo;

    for(int i = 0; i < nroEventos; i++){
        if(programacion[i].dia == dia && programacion[i].tiempo <= tiempo){
            ultimo = programacion[i];  //guardo el mas reciente
            encontrado = 1;
        }
    }

    if(encontrado){
        int h, m, s;
        convertirTiempo(ultimo.tiempo, &h, &m, &s);
        printf("A las %02d:%02d:%02d se transmite: %c %s\n",hh, mm, ss, ultimo.tipo, ultimo.nombre);
    } 
    else{
        printf("No hay programacion registrada en ese momento.\n");
    }
}

void consultarPorCancion(evento *programacion, int nroEventos,int dia){
    char nombreBuscado[120];
    printf("Ingrese el nombre de la cancion a buscar: ");
    getchar(); //limpiar salto previo
    fgets(nombreBuscado, sizeof(nombreBuscado), stdin);

    //eliminar salto de linea de fgets
    nombreBuscado[strcspn(nombreBuscado, "\n")] = '\0';

    int encontrado = 0;
    for(int i = 0; i < nroEventos; i++){
        if(programacion[i].dia == dia){
            if(programacion[i].tipo == 'C' && strstr(programacion[i].nombre, nombreBuscado)){
                int hh, mm, ss;
                convertirTiempo(programacion[i].tiempo, &hh, &mm, &ss);
                printf("La cancion '%s' se transmite a las %02d:%02d:%02d\n",programacion[i].nombre, hh, mm, ss);
                encontrado = 1;
            }
        }
        
    }

    if(!encontrado){
        printf("La cancion '%s' no se encuentra en la programacion de hoy.\n",nombreBuscado);
    }
}

void consultarPublicidad(evento *programacion, int nroEventos, int dia) {
    char nombreBuscado[120];
    printf("Ingrese el nombre de la publicidad a buscar: ");
    fgets(nombreBuscado, sizeof(nombreBuscado), stdin);

    //Eliminar salto de linea de fgets
    nombreBuscado[strcspn(nombreBuscado, "\n")] = '\0';

    int encontrado = 0;
    int contador = 0;

    for(int i = 0; i < nroEventos; i++){
        if(programacion[i].dia == dia && programacion[i].tipo == 'P'){
            if(strstr(programacion[i].nombre, nombreBuscado)){
                contador++;
                encontrado = 1;
            }
        }
    }

    if(encontrado){
        printf("La publicidad '%s' aparece %d veces en la programacion de hoy.\n",nombreBuscado, contador);
    }else{
        printf("La publicidad '%s' no se encuentra en la programacion de hoy.\n",nombreBuscado);
    }
}

void mostrarProgramacionCompleta(evento *programacion, int nroEventos,int dia){
    printf("\n======= PROGRAMACION COMPLETA DEL DIA =======\n");
    for(int i = 0; i < nroEventos; i++){
        if(programacion[i].dia == dia){
            int hh, mm, ss;
            convertirTiempo(programacion[i].tiempo, &hh, &mm, &ss);
            printf("%02d:%02d:%02d %c %s\n",hh, mm, ss,programacion[i].tipo,programacion[i].nombre);
        }
        
    }
    printf("=============================================\n");
}

int leerNroEntero(){ //Validacion para el menu de opciones
    int n,c;
    while(1){
        printf("Seleccione Opcion: ");
        if(scanf("%d",&n) == 1){
            while((c = getchar()) != '\n' && c != EOF);
            return n;
        }
        else{
            printf("Entrada invalida, porfavor ingrese un NUMERO entero\n");
            while((c = getchar()) != '\n' && c != EOF);
        }
    }
}

int main(){
    //Arreglos de todos los eventos disponibles y de 'eventos' para llevar registro de la programacion
    cancion canciones[1000];
    publicidad cunas[100];
    show shows[15];
    evento eventos[10000];

    int nroCan = 0,nroShow = 0,nroPub = 0,nroEvent = 0;

    //Leemos entradas
    leerCanciones(canciones,&nroCan);
    leerPublicidad(cunas,&nroPub);
    leerShows(shows,&nroShow);

    //Programamos los 7 dias de la semana
    for(int i=1;i<=7;i++){
        srand(time(NULL) + i * 100); //Inicializamos una semilla nueva por dia para garantizar aleatoreidad
        ProgramarDia(i,canciones,nroCan,cunas,nroPub,shows,nroShow,eventos,&nroEvent);
    }

    //Consultas de usuario
    int opt,dia,fase = 1;
    while(1){

        if(fase == 1){
            printf(">>========================================================================================<<\n"
                    "||   _____           _                         __       _ _              _____ __  __     ||\n"
                    "||  | ____|_ __ ___ (_)___  ___  _ __ __ _   _/_/___  _(_) |_ ___  ___  |  ___|  \\/  |    ||\n"
                    "||  |  _| | '_ ` _ \\| / __|/ _ \\| '__/ _` | | ____\\ \\/ / | __/ _ \\/ __| | |_  | |\\/| |    ||\n"
                    "||  | |___| | | | | | \\__ \\ (_) | | | (_| | |  _|_ >  <| | || (_) \\__ \\ |  _| | |  | | _  ||\n"
                    "||  |_____|_| |_| |_|_|___/\\___/|_|  \\__,_| |_____/_/\\_\\_|\\__\\___/|___/ |_|   |_|  |_|(_) ||\n"
                    "|| |  _ \\ _ __ ___   __ _ _ __ __ _ _ __ ___   __ _  __| | ___  _ __ __ _    __| | ___    ||\n"
                    "|| | |_) | '__/ _ \\ / _` | '__/ _` | '_ ` _ \\ / _` |/ _` |/ _ \\| '__/ _` |  / _` |/ _ \\   ||\n"
                    "|| |  __/| | | (_) | (_| | | | (_| | | | | | | (_| | (_| | (_) | | | (_| | | (_| |  __/   ||\n"
                    "|| |_|   |_|  \\___/ \\__, |_|  \\__,_|_| |_| |_|\\__,_|\\__,_|\\___/|_|  \\__,_|  \\__,_|\\___|   ||\n"
                    "||                  |___/| | | | ___  _ __ __ _ _ __(_) ___  ___                          ||\n"
                    "||                       | |_| |/ _ \\| '__/ _` | '__| |/ _ \\/ __|                         ||\n"
                    "||                       |  _  | (_) | | | (_| | |  | | (_) \\__ \\                         ||\n"
                    "||                       |_| |_|\\___/|_|  \\__,_|_|  |_|\\___/|___/                         ||\n"
                    ">>========================================================================================<<\n");
            printf("||\t\t\t\t  MENU DE CONSULTAS                                       ||\n");
            printf("||\t\t           Seleccione un dia de la semana                                 ||\n");
            printf("||\t\t  1-Lunes, 2-Martes, 3-Miercoles, 4-Jueves ...0-Salir                     ||\n");
            printf(">>========================================================================================<<\n");
            do{
                dia = leerNroEntero(); //Lectura de entrada validada por funcion
                if(dia == 0){
                    printf("Saliendo...\n");
                    return 0;
                }
                if(dia >= 1 && dia <= 7) fase = 2; //Si la entrada es valida pasamos a la 2da fase del menu
                if(dia < 1 || dia > 7) printf("Entrada invalida, ingrese un NUMERO del 1 al 7, 0 si desea salir\n");
            }
            while(dia < 1 || dia > 7);
            
        }
        else if (fase == 2){
            printf(">>========================================================================================<<\n");
            printf("|| 1. Consultar que se transmite a una hora dada                                          ||\n");
            printf("|| 2. Consultar si una cancion sonara y cuando                                            ||\n");
            printf("|| 3. Consultar si una publicidad se reproducira y cuantas veces                          ||\n");
            printf("|| 4. Mostrar programacion completa del dia                                               ||\n");
            printf("|| 5. Regresar a seleccion de dia                                                         ||\n");
            printf("|| 0. Salir                                                                               ||\n");
            printf(">>========================================================================================<<\n");
            do{
                opt = leerNroEntero();
                if(opt == 0){
                    printf("Saliendo...\n");
                    return 0;
                }
                if(opt < 1 || opt > 5) printf("Opcion invalida, por favor ingrese un NUMERO del 1 al 5, 0 si desea salir\n");
            }
            while(opt < 1 || opt > 5);
             
            switch(opt){
                case 1: 
                    consultarPorHora(eventos, nroEvent,dia);
                    break;
                case 2:
                    consultarPorCancion(eventos, nroEvent,dia);
                    break;
                case 3: 
                    consultarPublicidad(eventos,nroEvent,dia);
                    break;
                case 4:
                    mostrarProgramacionCompleta(eventos, nroEvent,dia); 
                    break;
                case 5:
                    fase = 1; //Si el usuario desea consultar otro dia puede volver pulsando 5
                    break;
            }
        }   
    }  
}

