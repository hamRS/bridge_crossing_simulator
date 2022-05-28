#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<phtread.h>
#include<time.h>
#define MAX_VEHICLE 3
#define MAX_WAITING 4
#define TRUE        1
#define FALSE       0
#define EAST        0
#define WEST        1
#define MAX_THREADS     10

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


//prototypes
void bridge_init();
int is_safe(int direction);
void arrive_bridge(int direction);
void exit_bridge(int direction);




int main(){

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
    for(i = 0 ; i < MAX_THREADS ; i++){
        car_status[i] = -1;
        car_directions[i] = 0;
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
        if(!is_safe(direction) && cars_crossing >= 4){ //wait to cross
            waiting[direction]++;
            while(!is_safe(direction) && cars_crossing >= 4)
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
    int direction = car_directions[ID]; 
    int i,j;
    pthread_mutex_lock(&status_lock);
        car_status[ID - 1] = 0;
    pthread_mutex_unlock(&status_lock);

    //try crossing the bridge
    arrive_bridge(direction);
    for(i = 1 ; i <= 3 ; i++){
        sleep(1);
        pthread_mutex_lock(&status_lock);
            car_status[ID - 1] = j; //wating
        pthread_mutex_unlock(&status_lock);
    }
    exit_bridge(direction);
    pthread_mutex_lock(&status_lock);
        car_status[ID - 1] = -1; //done crossing
    pthread_mutex_unlock(&status_lock);
}
