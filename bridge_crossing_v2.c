#include<stdio.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<string.h>
#define MAX_VEHICLE 3
#define MAX_WAITING 4
#define TRUE        1
#define FALSE       0
#define EAST        0
#define WEST        1
#define MAX_THREADS     30


//shared variables
int current_direction;
int vehicle_count;
int cars_crossing;
int east_west_crossing[2];
int waiting[2];
int car_status[MAX_THREADS];
int car_directions[MAX_THREADS];
int start_state;

/*max number of threads*/
pthread_t   input_cars_thread, add_cars_threads;
pthread_t   ID[MAX_THREADS];      /*vehicles IDs*/
int         arg[MAX_THREADS];     /*vehicle arguments*/
int car_thread_index;



//mutex
pthread_mutex_t monitor_lock;
pthread_mutex_t screen_lock;
pthread_mutex_t status_lock;

//condition variables
pthread_cond_t east_west[2];
pthread_cond_t start_cond;

//prototypes
void commnad_add_car(char * dir);
void bridge_init();
int is_safe(int direction);
void arrive_bridge(int direction);
void exit_bridge(int direction);
void add_car(int direction);
void show_car_status();
void start_command();
void * add_car_thread_command();

void * status_thread_function();
void * one_car(void * void_ptr);
void print_status();


int main(){
    //system("clear");
    pthread_t status_thread;
    int thr = 0;
    int i;
    char input[10];
    printf("Parent started....\n");
    pthread_mutex_init(&screen_lock, NULL);
    pthread_mutex_init(&status_lock , NULL);
    bridge_init();

    //create status thread
    if(pthread_create(&status_thread , NULL , &status_thread_function , NULL) != 0)
        perror("Failed to create thread");
    printf(">");
    scanf("%[^\n]" , input);
    while(strcmp(input, "start") != 0){
        if(strcmp(input, "car izq") == 0) // add car to west
            add_car(1);
        else if(strcmp(input , "car der") == 0) // add car to east
            add_car(0);
        else if(strcmp(input, "car status") == 0)
            show_car_status();
        getchar();
        printf(">");
        scanf("%[^\n]" , input);
    }
    //begin simulation
    start_command();
    if(pthread_join(status_thread , NULL))
        perror("Failed to join thread");
    return 0;
}
//aux functions
void show_car_status(){
    printf("**Autos en espera a atravezar el puente**\n");
    int i = 0;
    char * dir[] ={"=>" , "<="};
    for(i = 0; i < car_thread_index ; i++ ){
        if(i < 10)
            printf("%s auto0%d\n" ,dir[car_directions[i]] , i + 1);
        else
            printf("%s auto%d\n" ,dir[car_directions[i]] , i + 1);
    }
}
void add_car(int direction){
    system("clear");
    arg[car_thread_index] = car_thread_index + 1;
    car_directions[car_thread_index] = direction;
    car_status[car_thread_index] = -1;
    car_thread_index++;
}   

void start_command(){
    pthread_mutex_lock(&status_lock);
        start_state = 1;
        pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&status_lock);

    int i;
    for(i = 0 ; i < car_thread_index ; i++){
        arg[i] = i + 1;
        if(pthread_create(ID + i , NULL , &one_car , (void *) (arg + i)))
            perror("Failed to create thread");
    }
    //create add_car thread
    if(pthread_create(&add_cars_threads , NULL , &add_car_thread_command, NULL) != 0)
        perror("Failed to create thread");

    for(i = 0 ; i < car_thread_index ; i++)
        if(pthread_join(ID[i] , NULL) != 0)
            perror("Failed to join threads");

    if(pthread_join(add_cars_threads , NULL) != 0)
        perror("Failed to create thread");
    printf("Parent exits.....\n");
}

void * status_thread_function(){
    while(TRUE){
        print_status();
        sleep(2);
        system("clear");
    }
}

/*
    >>>>>>>>>>>
    >>auto 10>>
    >>>>>>>>>>>

    <<<<<<<<<<<
    <<auto 11<<
    <<<<<<<<<<<
*/

//aux functions 2
void print_status(){
    int i,count,last;
    char * dir[2] = {"--->" , "<---"};
    char * dir_top_bottom_car[] = {">>>>>>>>>>>" , "<<<<<<<<<<<"};
    char * dir_middle_car[] = {">>" , "<<"};
    int flag = 0;
    pthread_mutex_lock(&status_lock);
        while(start_state == 0)
            pthread_cond_wait(&start_cond , &status_lock);
        printf("Current direction: %s\n" , dir[current_direction]);
        for(i = 0 ; i < car_thread_index ; i++){
            if(car_status[i] == -1)
                continue;
            printf("Car : %d status: %d direction: %s\n" , i + 1, car_status[i], dir[car_directions[i]]);
            flag = 1;
            
        }
        last = -1;
        int j;
        count = 0;
        if(flag == 1){
            printf("=======================================================================================\n");
            //print top
            for(i = 0 ; i < car_thread_index ; i++){
                if(car_status[i] <= 0)
                    continue;
                for( j = 0 ; j < 5 ; j++ )
                    printf("====");
                printf("%s" , dir_top_bottom_car[car_directions[i]]);
                for( j = 0 ; j < 5 ; j++ )
                count++;
                if(count == 3)
                    break;
            }
            printf("============================"); 
            printf("\n");
            ///print middle
            count = 0;
            for(i = 0 ; i < car_thread_index ; i++){
                if(car_status[i] <= 0)
                    continue;
                for( j = 0 ; j < 5 ; j++ )
                    printf("====");
                printf("%s" , dir_middle_car[car_directions[i]]);
                if(i + 1 < 10)
                    printf("auto0%d", i + 1);
                else
                    printf("auto%d", i + 1);
                printf("%s" , dir_middle_car[car_directions[i]]);
                count++;
                if(count == 3)
                    break;
            }
            printf("============================");  
            printf("\n");
            count= 0;
            //print botton
            for(i = 0 ; i < car_thread_index ; i++){
                if(car_status[i] <= 0)
                    continue;
                for( j = 0 ; j < 10 ; j++ )
                    printf("==");
                printf("%s" , dir_top_bottom_car[car_directions[i]]);
                count++;
                if(count == 3)
                    break;
            }
            printf("============================");  
            printf("\n");
        }
    pthread_mutex_unlock(&status_lock);
}


//monitor functions
void bridge_init(){
    vehicle_count = 0;
    cars_crossing = 1;
    waiting[0] = waiting[1] = 0; //no vehicle waiting
    pthread_mutex_init(&monitor_lock , NULL);
    pthread_cond_init(&east_west[0] , NULL);
    pthread_cond_init(&east_west[1] , NULL);
    car_thread_index = 0;
    east_west_crossing[0] = 0;
    east_west_crossing[1] = 0;
    int i;
    int dir = 0;
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
        if(!is_safe(direction) || east_west_crossing[direction] >= 4){ //wait to cross
            waiting[direction]++;
            while(!is_safe(direction) || east_west_crossing[direction] >= 4)
                pthread_cond_wait(&east_west[direction] , &monitor_lock);
            waiting[direction]--;
        }
        vehicle_count++;
        
        east_west_crossing[direction]++;
        if(east_west_crossing[direction] >= 4 && direction == 0 && waiting[1] == 0)
            east_west_crossing[direction] = 0;
        else if(east_west_crossing[direction] >= 4 && direction == 1 && waiting[0] == 0)
            east_west_crossing[direction] = 0;
        /*if(cars_crossing > 4 && direction == 0 && waiting[1] == 0)
            cars_crossing = 1;
        else if(cars_crossing > 4 && direction == 1 && waiting[0] == 0)
            cars_crossing = 1;*/

        current_direction = direction;
    pthread_mutex_unlock(&monitor_lock);
}

void exit_bridge(int direction){
    pthread_mutex_lock(&monitor_lock);
        vehicle_count--;
        if(vehicle_count > 0 && east_west_crossing[direction] < 4){ // we still have vehicles on the bridge
            pthread_cond_broadcast(&east_west[direction]);
        }else{
            if(waiting[1 - direction] != 0){
                east_west_crossing[1 - direction] = 0;
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
    int tmp = east_west_crossing[direction];
    for(i = 1 ; i <= 3 ; i++){
        sleep(1 + tmp);
        pthread_mutex_lock(&status_lock);
            car_status[ID - 1] = i; //wating
        pthread_mutex_unlock(&status_lock);
    }
    sleep(1);
    exit_bridge(direction);
    pthread_mutex_lock(&status_lock);
        car_status[ID - 1] = -1; //done crossing
    pthread_mutex_unlock(&status_lock);
    printf("Car %d done crossing\n" , ID);
}

void * add_car_thread_command(){   
    char input[10]; 
    printf(">");
    scanf("%[^\n]" , input);
    while(strcmp(input, "start") != 0){
        if(strcmp(input, "car izq") == 0) // add car to west
            add_car(1);
        else if(strcmp(input , "car der") == 0) // add car to east
            add_car(0);
        if(pthread_create(ID + car_thread_index - 1 , NULL , &one_car , (void *) (arg + car_thread_index - 1)) != 0)
            perror("Failed to create thread");
        
        getchar();
        printf(">");
        scanf("%[^\n]" , input);
    }   
}