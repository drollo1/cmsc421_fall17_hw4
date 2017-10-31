//***********************************************************
//File Name: hw3.c
//Author Name: Dominic Rollo
//Assignment: Homework 3
//
//  A simulation of groups of children collecting candy. The
//	 simulation uses threads for each group and the restocking
//   the houses. The args passed in are file name followed buy
//   number of seconds to run.
//
//**************Outside Help*********************************
//  https://www.tutorialspoint.com/c_standard_library/c_function_rand.htm
//
//***********************************************************
//**************Questions************************************
//  1. A group might get to a house alittle bit after the main 
//   thread's second count.  If it does, the amount of candy will
//   change while the old number was being displayed.
//  2. The program could calculate when a group will get to a 
//   at the second mark and wait for it to aquire candy before
//   main thread prints.
//***********************************************************
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>


typedef struct House{
	int location[2];
	int candy;
}House;

typedef struct Group{
	int start_house, cur_house, size, candy, going_to;
}Group;

typedef struct Instruction{
	int house;
	int candy;
} Instruction;

House neighborhood[10];
int G, T, end, start_sim, end_sim, total_candy;
Group *lil_monsters;
Instruction *refill_lst;
int lst_size, monster_size, instr_size;
pthread_mutex_t prnt_lock, thread_lock, house_lock[10];


#define START_SIZE 10

// Puts line from stream
//**************************************************************
static int get_number(FILE *infile){
	char input[5];
	for (int i=0; i<5; i++)
		input[i]='\0';
	char c_val =fgetc(infile);
	int i=0;
	do{
		input[i]=c_val;
		c_val=fgetc(infile);
		i++;
	}while (c_val!='\n'&& c_val!=' '&& c_val!=EOF);	
	if(c_val==EOF)
		end = 1;
	int result = atoi(input); 
	return result;
}

// Parses out and saves houses
//**************************************************************
static void house_parser(FILE *infile){
		for (int i=0; i<10; i++){
			neighborhood[i].location[0]=get_number(infile);
			neighborhood[i].location[1]=get_number(infile);
			neighborhood[i].candy=get_number(infile);
		}
}

// Parse out the group variable and store in an array
static void group_parser(FILE *infile){
	monster_size=START_SIZE;
	for (int i=0; i<G;i++){
		if (i>=monster_size){
			printf("resize\n");
			monster_size=START_SIZE+monster_size;
			lil_monsters=realloc(lil_monsters, sizeof(Group)*monster_size);
		}
		lil_monsters[i].start_house=get_number(infile);
		lil_monsters[i].cur_house=lil_monsters[i].start_house;
		lil_monsters[i].size=get_number(infile);
	}
}

// Parses out and saves instrustions in an array
//**************************************************************
static void instruction_parser(FILE *infile){
	lst_size=START_SIZE;
	int i=0;
	do{
		if (i>=lst_size){
			printf("list resize\n");
			lst_size=START_SIZE+lst_size;
			refill_lst=realloc(refill_lst, sizeof(Instruction)*lst_size);
		}
		refill_lst[i].house=get_number(infile);
		refill_lst[i].candy=get_number(infile);
		
		i++;
	}while(!end);
	instr_size = i-1;
}

// Prints out houses the 10 houses in order with there locations
// and amount of current candy.
//**************************************************************
static void print_house(void){
	printf("  House statuses:\n");
	for (int i=0;i<10;i++){
		printf("    %d @ ", i);
		printf("(%d, %d): ", neighborhood[i].location[0], neighborhood[i].location[1]);
		printf("%d available\n", neighborhood[i].candy);
	}
	printf("Total candy: %d\n", total_candy);
}

// Prints out group size, where they are going and 
static void print_group(void){
	printf("  Group statuses:\n");
	for (int i=0;i<G;i++){
		printf("    %d:    ", i);
		printf("size %d, ", lil_monsters[i].size);
		printf("going to %d, ",lil_monsters[i].going_to );
		printf("collected %d\n", lil_monsters[i].candy);
	}

}

// Calls the methods that parse and store house, group and
// instruction variables.
static void parser(FILE *infile){
	G =get_number(infile);
	house_parser(infile);
	group_parser(infile);
	instruction_parser(infile);
}

//use thread to restock houses
//************************************************************
static void *restock(void *args) {
	int i=0;
	pthread_mutex_unlock(&thread_lock);
	while(!start_sim){
			usleep(1);
		}
	usleep(250000);
	while(!end_sim && i<lst_size){
		pthread_mutex_lock(&house_lock[refill_lst[i].house]);
		neighborhood[refill_lst[i].house].candy+=refill_lst[i].candy;
		pthread_mutex_unlock(&house_lock[refill_lst[i].house]);
		pthread_mutex_lock(&prnt_lock);
		printf("Neighborhood: added %d to house %d\n", refill_lst[i].candy, refill_lst[i].house);
		pthread_mutex_unlock(&prnt_lock);
		i++;
		usleep(250000);
	}
	return NULL;
}

// Picks a random house by doing some math to a random number
int random_house(int size, int current){
	time_t t;
	srand((unsigned) time(&t));
	int result = rand()/size;
	result = result*(current+1);
	result = result%10;
	result = abs(result);
	return result;
}

int move_time(int house1, int house2){
	int x1, x2, y1, y2;
	x1 = neighborhood[house1].location[0];
	y1 = neighborhood[house1].location[1];
	x2 = neighborhood[house2].location[0];
	y2 = neighborhood[house2].location[1];

	int result = abs(x1-x2)+abs(y1-y2);
	result = result * 250000;
	return result;
}

static void collect_candy(int group, int house){
	pthread_mutex_lock(&house_lock[house]);
	if(neighborhood[house].candy>=lil_monsters[group].size){
		neighborhood[house].candy=neighborhood[house].candy - lil_monsters[group].size;
		lil_monsters[group].candy += lil_monsters[group].size;
		total_candy+=lil_monsters[group].size;
		//printf("*****group %d collected %d candy\n", group, lil_monsters[group].size);
	}
	else{
		lil_monsters[group].candy += neighborhood[house].candy;
		total_candy+= neighborhood[house].candy;
		//printf("*****group %d collected %d candy\n", group, neighborhood[house].candy);
		neighborhood[house].candy= 0;

	}
	pthread_mutex_unlock(&house_lock[house]);
}

//Sets up the groups when thier threads start
//*************************************************************
static void *group_up(void *args){
	int group_num = *(int *)(args);
	int move_to, time;
	while(!end_sim){
		do{
			move_to = random_house(lil_monsters[group_num].size, lil_monsters[group_num].cur_house);
		}while(move_to == lil_monsters[group_num].cur_house || move_to == lil_monsters[group_num].start_house);
		time = move_time(lil_monsters[group_num].cur_house, move_to);
		lil_monsters[group_num].going_to=move_to;
		pthread_mutex_lock(&prnt_lock);
		printf("Group %d: ", group_num); 
		printf("from house %d ", lil_monsters[group_num].cur_house);
		printf("to %d", move_to);
		printf(" (Travel time = %d ms)\n", time/1000);
		pthread_mutex_unlock(&prnt_lock);
		if(!start_sim)
			pthread_mutex_unlock(&thread_lock);
		while(!start_sim){
			usleep(1);
		}
		usleep(time);
		lil_monsters[group_num].cur_house = move_to;
		collect_candy(group_num, move_to);
	}
	return NULL;
}

static void run_sim(){
	end_sim=0;
	start_sim=0;
	total_candy=0;
	pthread_mutex_init(&prnt_lock, NULL);
	pthread_mutex_init(&thread_lock, NULL);
	for( int k=0; k<10;k++)
		pthread_mutex_init(&house_lock[k], NULL);
	pthread_t threads[G+1];
	int number[G];
	for(int j=0; j<G;j++){
		number[j]=j;
		pthread_mutex_lock(&thread_lock);
		pthread_create(threads + j, NULL, group_up, number+j);
	}
	int i=0;
	pthread_mutex_lock(&thread_lock);
	pthread_create(threads+G, NULL, restock, NULL);
	pthread_mutex_lock(&thread_lock);
	while(i<T){
		pthread_mutex_lock(&prnt_lock);
		printf("After %d seconds:\n", i);
		print_group();
		print_house();
		pthread_mutex_unlock(&prnt_lock);
		start_sim=1;
		usleep(1000000);
		i++;
	}
	printf("After %d seconds:\n", i);
	print_group();
	print_house();
	end_sim=1;
	for(int i=0; i<=G;i++)
		pthread_join(threads[i], NULL);
}

int main(int argc, char *argv[]){

	if (argc==1){
		fprintf(stderr, "There are not enough arguments to run program.");
		fprintf(stderr, "\nFix to look like: ./hw3 trick1.data 4\n");
		return 1;
	}
	FILE *data;
	if ((data= fopen(argv[1], "r"))==0){
			fprintf(stderr, "File could not open\n");
			return 1;
		}

	T=atoi(argv[2]);
	monster_size=START_SIZE;
	lst_size=START_SIZE;
	lil_monsters=malloc(sizeof(Group)*monster_size);
	refill_lst=malloc(sizeof(Instruction)*lst_size);
	parser(data);
	run_sim();
	
	//clean up
	fclose(data);
	free(lil_monsters);
	free(refill_lst);
	return 0;
}