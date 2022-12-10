#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<semaphore.h>

#define NUM_FILOSOFOS 5


#define maxEstomago 5

#define comidaMax 10

void *comer (void *arg);

void printAccion(int acc, char* nom, int posi);

int posicion(char* nom);

void tomarTenedor(char* nom, int pos);

void dejarTenedor(char* nom, int pos);

void pensar(char *nom, int pos);

void printST(int ca, int c, int cc, int es, int est);


int accion_Filo[NUM_FILOSOFOS];

int energia[NUM_FILOSOFOS];

int contEnergia[NUM_FILOSOFOS];

int comida = comidaMax;

int contComida = 0;

int estomagos[NUM_FILOSOFOS];

int estadoF[NUM_FILOSOFOS];

pthread_mutex_t mutex;

pthread_mutex_t palillos[NUM_FILOSOFOS];

sem_t comida_M;

sem_t energia_M;

char nomFilo[10][20] = {"Confucio", "Pitágoras", "Platón", "Sócrates", "Epicurio", "Tales", "Heráclito", "Diógenes", "Sófocles", "Zenón"};

int main(void){
	
	int i;
	int j;

	srand(time(NULL));
	printf("Total de Comida: %d \n", comida);
	
	//Hilos de los filósofos
	pthread_t filosofos[NUM_FILOSOFOS];
		

	sem_init(&comida_M, 0, 1);
	sem_init(&energia_M, 0, 1);

	for (i = 0; i < NUM_FILOSOFOS; i++){
		accion_Filo[i] = 0;
		energia[i] = (rand() % maxEstomago) + 1;
		contEnergia[i] = energia[i];
		estadoF[i] = 0;
		estomagos[i] = 1;
		pthread_mutex_init(&palillos[i], NULL);
	}

	for (i = 0; i < NUM_FILOSOFOS; i++){
		pthread_create(&filosofos[i], NULL, &comer, &nomFilo[i]);
		printf("CREANDO FILOSOFOS %s ..... Energía %d \n", nomFilo[i], energia[i]);
		sleep(1);
	}

	printf("Se crearon todos los filosofos\n");
	
	for (j = 0; j < NUM_FILOSOFOS; j++){

		pthread_join(filosofos[j], NULL);	
	}
	
	pthread_mutex_destroy(&mutex);
	sem_destroy(&comida_M);
	sem_destroy(&energia_M);

	return 0;
}

void printAccion(int acc, char* nom, int posi){
	if (acc == 0){
		printf("%d Filósofo %s está pensando\n", posi, nom);
	}else if(acc == 1){
		printf("%d Filósofo %s levanta tenedor derecho\n", posi, nom);
	}else if (acc == 2){
		printf("%d Filósofo %s levanta tenedor izquierdo\n", posi, nom);
	}else if (acc == 3){
		printf("%d |||||   Filósofo %s debe comer    |||||\n", posi, nom);
	}
}

int posicion(char* nom){
	int pos = 0;
	for (int i = 0; i < NUM_FILOSOFOS; i++){
		if (nomFilo[i] == nom){
			pos = i;
			break;
		}
	}
	return pos;
}

void tomarTenedor(char* nom, int pos){
	
	if (pos%2 == 0){
		pthread_mutex_lock(&palillos[pos]);
		printAccion(1, nom, pos);
		pthread_mutex_lock(&palillos[(pos+1)%5]);
		printAccion(2, nom, pos);

	} else {
		pthread_mutex_lock(&palillos[(pos+1)%5]);
		printAccion(2, nom, pos);
		pthread_mutex_lock(&palillos[pos]);
		printAccion(1, nom, pos);
	}	
}

void dejarTenedor(char* nom, int pos){

	pthread_mutex_unlock(&palillos[pos]);
	pthread_mutex_unlock(&palillos[(pos + 1) % 5]);	
	
	printAccion(0, nom, pos);
}

void pensar (char *nom, int pos){
	estomagos[pos] -= 1;
	contEnergia[pos] -= 1;
	printf(".........%s está pensando........ estomago: %d\n		energía:  %d\n", nom, estomagos[pos], contEnergia[pos]);
	
	if (contEnergia[pos] <= 0 || estomagos[pos] == 0){
		estadoF[pos] = 3;
		printAccion(3, nom, pos);
	}
}

void printST(int ca, int c, int cc, int est){
	printf("\n		------------------------------\n");
	printf("		   Comida Anterior: %d \n", ca);
	printf("		   Comida Actual: %d\n", c);
	printf("		   Diferencia: %d \n", ca-c);
	printf("		   Veces que comió: %d\n", cc);
	printf("		   Diferencia Estómago: %d", est);
	printf("\n		------------------------------\n");
}


void *comer (void *arg){

	char *nombre = (char *)arg;
	int pos = posicion(nombre);

	for(int i = 0; true; i++){
		
		if (estadoF[pos] == 0){
			sem_wait(&energia_M);
			pensar(nombre, pos);
			sem_post(&energia_M);
			sleep(4);
		} else {
		
			tomarTenedor(nombre, pos);
			
			sem_wait(&comida_M);
			int comidaActual = comida;
			int estomagoActual = estomagos[pos];
			int cont = 0;
			while (estomagos[pos] != maxEstomago || contEnergia[pos] != energia[pos]){

				if (comida <= 0){
					printf("\nSe terminó la comida, filósfo %s respone", nombre);
					comida = comidaMax;
					contComida++;
					printf("\n se restauró la comida %d veces\n\n", contComida);
				}
				estomagos[pos] += 1;
				if (contEnergia[pos] != energia[pos]){
					contEnergia[pos] += 1;
				}
				comida--;
				printf("		%s estómago: %d\n", nombre, estomagos[pos]);
				printf("		%s energía: %d\n", nombre, contEnergia[pos]);	
				cont++;
			}
			printST(comidaActual, comida, cont, estomagos[pos]-estomagoActual);

			sem_post(&comida_M);
			if (estomagos[pos] == maxEstomago){
				printf("	Filósofo %s lleno\n", nombre);
			}
			if (contEnergia[pos] == energia[pos]){
				printf("	Filósofo %s con energía", nombre);
			}
			printf("\n----------Comida %d----------\n", comida);

			dejarTenedor(nombre, pos);

            estadoF[pos] = 0;
			sleep(8);
		}
	}
		
	return NULL;
}
