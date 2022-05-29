#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#define MAX_VEHICLE 3
#define MAX_WAITING 4
#define TRUE        1
#define FALSE       0
#define EAST        0
#define WEST        1
#define MAX_THREADS     20

//shared variables
int current_direction;
int vehicle_count;
int cars_crossing;
int waiting[2];
int car_status[MAX_THREADS];
int car_directions[MAX_THREADS];

//mutex
pthread_mutex_t monitor_lock;
pthread_mutex_t screen_lock;
pthread_mutex_t status_lock;

//condition variables
pthread_cond_t east_west[2];
pthread_cond_t update_state;

//prototypes
void bridge_init();
int is_safe(int direction);
void arrive_bridge(int direction);
void exit_bridge(int direction);


void * status_thread_function();
void * one_car(void * void_ptr);
void print_status();
int main(){
    pthread_t   status_thread;
    pthread_t   ID[MAX_THREADS];      /*vehicles IDs*/
    int         arg[MAX_THREADS];     /*vehicle arguments*/
    int         thr;                   /*# of vehicles*/
    int         i;

    thr = 15;
    printf("Parent started....\n");
    pthread_mutex_init(&screen_lock, NULL);
    pthread_mutex_init(&status_lock , NULL);
    bridge_init();

    /*create status thread*/
    if(pthread_create(&status_thread, NULL , &status_thread_function , NULL) != 0)
        perror("Failed to create thread");

    /*create threads*/
    for(i = 0 ; i < thr ; i++){
        arg[i] = i + 1;
        if(pthread_create(ID + i , NULL , &one_car , (void *) (arg + i)) != 0)
            perror("Failed to create thread");
    }
    //join threads
    for(i = 0 ; i < thr ; i++)
        if(pthread_join(ID[i] , NULL) != 0)
            perror("Failed to join threads");
        printf("Parent exits.....\n");
        pthread_join(status_thread , NULL);
    return 0;
}

//monitor functions
void bridge_init(){
    vehicle_count = 0;
    cars_crossing = 1;
    waiting[0] = waiting[1] = 0; //no vehicle waiting
    pthread_mutex_init(&monitor_lock , NULL);
    pthread_cond_init(&east_west[0] , NULL);
    pthread_cond_init(&east_west[1] , NULL);


    int i;
    int dir = 0;
    for(i = 0 ; i < MAX_THREADS ; i++){
        car_status[i] = -1;
        car_directions[i] = dir == 0 ? 1 : 0;
        dir = dir == 0 ? 1 : 0;
    }
}

int is_safe(int direction){
    if(vehicle_count == 0)
        return TRUE;
    else if((vehicle_count < MAX_VEHICLE) && ((current_direction == direction)))
        return TRUE;
    else
        return FALSE;
}

void arrive_bridge(int direction){
    pthread_mutex_lock(&monitor_lock);
        if(!is_safe(direction) || cars_crossing >= 4){ //wait to cross
            waiting[direction]++;
            while(!is_safe(direction) || cars_crossing >= 4)
                pthread_cond_wait(&east_west[direction] , &monitor_lock);
            waiting[direction]--;
        }
        vehicle_count++;
        
        cars_crossing++;
        if(cars_crossing >= 4 && direction == 0 && waiting[1] == 0)
            cars_crossing = 1;
        else if(cars_crossing >= 4 && direction == 1 && waiting[0] == 0)
            cars_crossing = 1;

        current_direction = direction;
    pthread_mutex_unlock(&monitor_lock);
}

void exit_bridge(int direction){
    pthread_mutex_lock(&monitor_lock);
        vehicle_count--;
        if(vehicle_count > 0 && cars_crossing < 4){ // we still have vehicles on the bridge
            pthread_cond_broadcast(&east_west[direction]);
        }else{
            if(waiting[1 - direction] != 0){
                cars_crossing=1;
                pthread_cond_broadcast(&east_west[1 - direction]);
            }else
                pthread_cond_broadcast(&east_west[direction]);
        }
    pthread_mutex_unlock(&monitor_lock);
}


///threads functions
void * one_car(void * void_ptr){
    int *  int_ptr = (int *) void_ptr;
    int ID = *int_ptr;
    int direction = car_directions[ID - 1];
    int i,j;
    pthread_mutex_lock(&status_lock);
        car_status[ID - 1] = 0;
    pthread_mutex_unlock(&status_lock);
    //try crossing the bridge
    sleep(2);
    arrive_bridge(direction);
    for(i = 1 ; i <= 3 ; i++){
        sleep(2);
        pthread_mutex_lock(&status_lock);
            car_status[ID - 1] = i; //wating
        pthread_mutex_unlock(&status_lock);
    }
    exit_bridge(direction);
    pthread_mutex_lock(&status_lock);
        car_status[ID - 1] = -1; //done crossing
    pthread_mutex_unlock(&status_lock);
    printf("Car %d done crossing\n" , ID);
}

void * status_thread_function(){
    while(TRUE){
        print_status();
        sleep(2);
        system("clear");
    }
}
void create_output(){

}

//aux functions
void print_status(){
    int i;
    char * dir[2] = {"--->" , "<---"};
    pthread_mutex_lock(&status_lock);
        printf("Current direction: %s\n" , dir[current_direction]);
        for(i = 0 ; i < MAX_THREADS ; i++){
            if(car_status[i] == -1)
                continue;
            printf("Car : %d status: %d direction: %s\n" , i + 1, car_status[i], dir[car_directions[i]]);
            
        }
    pthread_mutex_unlock(&status_lock);
}