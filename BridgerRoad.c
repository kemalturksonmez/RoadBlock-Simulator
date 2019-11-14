// Author @ Kemal Turksonmez
// To build this program:
// gcc -std=c99 BridgerRoad.c -o B2 -lpthread

/**
* This programs give the user the option to simulate a real traffic scenario by working
* with randomly timed and randomized thread declaration
**/

#include <stdio.h>
#include <stdlib.h>    // exit()
#include <pthread.h>
#include <unistd.h> // sleep()
// Directional state variables
const int TO_BRIDGER = 1;
const int TO_BOZEMAN = 0;
pthread_mutex_t mutexDrive; //<<<<<<<<<<<<<<<<<<<< mutex lock for cars that are driving
pthread_mutex_t mutexDirect; //<<<<<<<<<<<<<<<<<<<< mutex to change direction
//Maximum amount of cars that can be on the road at the same time
int  MAX_CARS; 
// cars that are going on the road
int areGoing = 0;
// cars that are on the road
int carsOn = 0;
// cars that are going to bozeman
int bozeCars = 0;
// cars that are going to bridger
int bridgeCars = 0;
//Direction the cars are going in
// 0 is TO_BOZEMAN
// 1 is TO_BRIDGER
int carDirec = 0;


void changeDirec(int direction){
	if(direction == TO_BOZEMAN){
		carDirec = TO_BRIDGER;
	} else {
		carDirec = TO_BOZEMAN;
	}
}
/**
* Cars arrive and loop in while loop until traffic in their direction starts moving
* direction: direction of car/current thread
* inp: cars direction as a string
**/
void ArriveBridgerOneWay(int direction, char* inp){
	pthread_mutex_lock(&mutexDirect);
	if (direction == TO_BOZEMAN){
		bozeCars++;
	} else{
		bridgeCars++;
	}
	printf("Car in line for %s \n", inp);
	pthread_mutex_unlock(&mutexDirect);
	while(1){
		pthread_mutex_lock(&mutexDirect);
		// if traffic is in the same direction as the car, attempt to enter road
		if (carDirec == direction){		
			if (areGoing < (MAX_CARS*2)){
				if(areGoing == 0){
					printf("\nDIRECTION: %s \n", inp);
					printf("CARS WAITING TO GO TO BOZEMAN: %d \nCARS WAITING TO GO TO BRIDGER: %d\n", bozeCars, bridgeCars);
				}
				areGoing++;
				break;
			}
		}
		// This is for final remaining cars, ensuring that they too will be able to exit
		if (carsOn == 0 && areGoing == 0 && direction != carDirec){
			pthread_mutex_lock(&mutexDrive);
			if (bozeCars == 0 && bridgeCars > 0 && carDirec == TO_BOZEMAN){
				carDirec = TO_BRIDGER;
			} else if (bridgeCars == 0 && bozeCars > 0 && carDirec == TO_BRIDGER){
				carDirec = TO_BOZEMAN;
			}
			pthread_mutex_unlock(&mutexDrive);
		}
		pthread_mutex_unlock(&mutexDirect);
	}
}

/**
* Car gets on road
* inp: cars direction as a string
**/
int OnBridgerOneWay(char* inp){
	pthread_mutex_lock(&mutexDrive);
	printf("Car for %s getting ON\n", inp);
	carsOn++;
	pthread_mutex_unlock(&mutexDirect);
	// Tracks if the car is the last possible car before a forced switch
	int lastCar = 0;
	if (carsOn == MAX_CARS){
		printf("MAXIMUM cars on road \n");
		// sets last car to be true
		lastCar = 1;
	} else {
	pthread_mutex_unlock(&mutexDrive);
	}	
	// amount of time spent on the road
	sleep(3);
	return lastCar;
}

/**
* Car exits road
* direction: direction of car/current thread
* inp: cars direction as a string
* lastCar: a variable that states whether the current thread is the last possible thread before a forced switch
**/
void ExitBridgerOneWay(int direction, char* inp, int lastCar){	
	//Car gets off road
	if (!lastCar){ 
	pthread_mutex_lock(&mutexDrive);
	}
	printf("Car for %s getting OFF\n", inp);
	carsOn--;
	// decrement amount of cars
	if (direction == TO_BOZEMAN){
		bozeCars--;
	} else{
		bridgeCars--;
	}
	pthread_mutex_unlock(&mutexDrive);

	// if road is empty, variables are reset and direction is switched
	pthread_mutex_lock(&mutexDirect);
	pthread_mutex_lock(&mutexDrive);
	// ensures cars can get out of line
	if (carsOn == 0 && carDirec == direction){
		areGoing = 0;
		carDirec = !carDirec;
	}
	pthread_mutex_unlock(&mutexDrive);
	pthread_mutex_unlock(&mutexDirect);
}

/**
* structures methods in order for threads to organize themselves in a traffic simulation
* arg: message buffer of thread, holds the direction of the car/thread 
**/
void *OneVehicle(void * arg) {
	int direction = *(int *)arg;
	char* inp = (direction == TO_BOZEMAN) ? "Bozeman" : "Bridger";
	// a car has arrived
    ArriveBridgerOneWay(direction, inp);
    // now the car is on the one-way section!
    int maxCars = OnBridgerOneWay(inp);
    // now the car is off
    ExitBridgerOneWay(direction, inp, maxCars);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
// Initialize mutex for driving
pthread_mutex_init(&mutexDrive, NULL);
// Initialize mutex for direction
pthread_mutex_init(&mutexDirect, NULL);
// checks return value of pthread operations
int rc;
// initialize for loop counter
long t;

// cars going to bozeman
int totalCars = atoi(argv[1]);
// number of cars that can be on the road at the same time
MAX_CARS = atoi(argv[2]);
// Maxmimum amount of random seconds before another car comes
int time = 0;
if (argc == 4){
	time = atoi(argv[3]);
}
time += 1;
pthread_t threads[totalCars];
// create threads pool
for(t=0; t<totalCars; t++){
   // sleeps for some random amount of time between 0 and variable time-1 before creating a new thread 
   sleep(rand()%(time));
   if (rand()%2 == 1){ 
   if (t == 0){carDirec = TO_BRIDGER;}
   rc = pthread_create(&threads[t], NULL, OneVehicle, (void *)&TO_BRIDGER);  //<<<<<<<<<<<<<<<<<<<< create threads
   } else {
   if (t == 0){carDirec = TO_BOZEMAN;}
   rc = pthread_create(&threads[t], NULL, OneVehicle, (void *)&TO_BOZEMAN); 
   }
   if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
   }
}


// wait for the threads to finish (pthread_join)
// pthread_join() blocks the calling thread until the specified thread terminates.
for(t=0; t<totalCars; t++){
   rc = pthread_join(threads[t], NULL); //<<<<<<<<<<<<<<<<<<<< wait for threads to finish
   if (rc){
      fprintf(stderr, "ERROR; return code from join() is %d\n", rc);
      exit(-2);
   }
}

printf("Done\n");

// Initialize mutex for driving
pthread_mutex_destroy(&mutexDrive);//<<<<<<<<<<<<<<<<<<<< clean up
// Initialize mutex for direction
pthread_mutex_destroy(&mutexDirect);//<<<<<<<<<<<<<<<<<<<< clean up
pthread_exit(NULL);
  return 0;
}
