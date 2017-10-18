#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#define NUM_THREADS  2

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

struct Room{
  char  name[256];
  char* connection[256];
  char  type[256];
};

struct Directory{         //this structre is used to store the startroom , endroom,and mid_room of the game
  char start_room[20];
  char end_room[20];
  char* mid_room[20];
};

void play(char* last_dir,struct Directory* Dir,struct Room* room_array);
void create_thread();
void print_startroom(char* startroom);
void read_file(struct Directory* Dir,struct Room* room_array,char* last_dir);
void read_room(char* last_dir,char* input);
void shuffle_roomnames(char* room_names[],int size);
void* write_time();
void read_time();

int main(){
  /*********find most recently room directory*************/
 // FILE* infofp;
  struct stat attr;
  DIR           *d;
  struct dirent *dir;
  char mystring[256];
  char* last_dir;
  memset(mystring,'\0',sizeof(mystring));	
  //this block is used to find the directory that the user created recently
  d = opendir(".");
 // infofp=fopen("directoryinfo","w");
    if (d){
      time_t temp_last=0;
      while ((dir = readdir(d)) != NULL){
        memset(mystring,'\0',sizeof(mystring));  //initialize the c-string array by \0
	stat(dir->d_name, &attr);
        //printf("%s \n",dir->d_name);	
        //printf("mystring: %s",mystring);
	sprintf(mystring, dir->d_name);
	if(strstr(mystring,"liqiao.room")!=NULL){     //find a directory that has substring= liqiao.room
	    if(difftime(attr.st_mtime,temp_last)>0){  //in order to find correct direcory to play game
	        temp_last= attr.st_mtime;
	         last_dir = (dir->d_name);   
	    }   
	}
      }
//	printf("lastdir: %s\n", last_dir );      
    }
      closedir(d);
   //   fclose(infofp);
      chdir(last_dir);          //go to the directory that last created


  /************game start*******************/
  struct Room*  room_array = (struct Room*) malloc(7*sizeof(struct Room));
  struct Directory* Dir;  

  read_file(Dir,room_array,last_dir);
  /*main function to play game*/
  play(last_dir,Dir,room_array);                          // this is my main function to play this game
 
  free(room_array);
return 0;
}
/**********************************************************************
 *                               Play
 * This function is the main program of this adventure game, which take care
 * of the prompt and control whether the user type invalid roomname and whether 
 * the user find the end room. also contains the time keeping part
 * ******************************************************************/

void play(char* last_dir,struct Directory* Dir,struct Room* room_array){
 // printf("check last: %s",last_dir );
  char input[8];
  char* input_arr[64];
  memset(input,'\0',sizeof(input));	
  FILE* fp5;
  int reach_end =0;
  char step_path[16][64];
  int step = 0;
  int i = 0;
  int k =0;
  int pre =0;
  char* pos;
  /*time_keeping*/
  print_startroom(Dir->start_room);
  
  pthread_mutex_lock(&myMutex);     //the main thread lock the mutex at the first

    do{
    create_thread();
    printf("\nWhere To? >");
    fgets(input,8,stdin);

    if((pos=strchr(input,'\n'))!=NULL){
     // printf("POS: %c",&pos);
      *pos = '\0';
    }
   
    if(strcmp(input,"time")==0){         //when user input time, then unlock the main thread , the 2nd thread will be lock, and write time to file
     
    pthread_mutex_unlock(&myMutex);
    read_time();
    pthread_mutex_lock(&myMutex);    //after read time, come back to main thread
    }
    
    else if(fopen(input,"r")==NULL){          //when user types a invalid room names
      
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGRAIN\n");
        if(step!=0){
        pre = step - 1; 
        read_room(last_dir,step_path[pre]);  //if user type invalid roomname, then repeat the current location and prompt.
        }
        else{ 
	  read_room(last_dir,Dir->start_room);
        }
    }
    else if(fopen(input,"r") != NULL && strcmp(input,"time")!=0){
    strcpy(step_path[step],input);
   // int p = 0;
   // for(p=0;p<step;p++){
   // 	printf("[STEP]: %s\n",&step_path[p]);
   // }
    step +=1;
    read_room(last_dir,input);               //get the data from room file, such as current location and possible connections
    }
    if(strcmp(input,Dir->end_room)==0){        //if user type the end room name, game over and user win.
      printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
      printf("YOU TOOK %d STEPS.YOUR PATH TO VICTORY WAS:\n",step);
      for(i=0;i<step;i++){
        printf("%s \n", step_path[i]);
      }
      reach_end =1;
    }  //if equal
  }while(reach_end == 0); 
}

/*******************************************************************************************
 *                            create_thread
 * this function is used to create a second thread to write time to a file
 *
 * ****************************************************************************************/
void create_thread(){
  pthread_t writethread;
  int result_code;
  result_code = pthread_create(&writethread,NULL,write_time,NULL);    //create a second thread and write time 
  assert(0 == result_code);
  
}

/***************************************************
 *
 * 			read_room 
 * this function read the room data where user want to go,
 * and printout the data such as current location and Possible
 * connections.
 *               
 * ************************************************/
void read_room(char* last_dir,char* input){
  FILE* fp0;
  char cbuffer[64];
  char possible_arr[32][64];
  int i = 0,j=0;
  char possible[64];
  int len =0;
  //check current workin dir
 // if(getcwd(cwd,sizeof(cwd))!=NULL){
 // fprintf(stdout,"current workding dir: %s\n",cwd);
 // }
  //else
   // perror("get error ");
//open file for read data 
  fp0 = fopen(input,"r");
   if(fp0 ==NULL){
      perror("in read room,HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGRAIN\n");
   }
   else{

    printf("\nCURRENT LOCATION: %s \n", input);
    printf("POSSIBLE CONNECTION: ");
    while(fgets(cbuffer,sizeof(cbuffer),fp0)!=NULL){
     
     if(strstr(cbuffer,"CON")!=NULL){
        char* token = strtok(cbuffer,":");
        token = strtok(NULL," \n");
//	printf("%s,",token);
       sprintf(possible,token);
	strcpy(possible_arr[i],possible);
	i+=1;
	
       }
   }

    for(j=0;j<i;j++){
      if(j==i-1){
        printf("%s.",possible_arr[j]);
      }
      else{
      printf("%s,",possible_arr[j]);
      }
    }
   }
   fclose(fp0);
}

    
/********************************************************************************
 *                      read file
 * This function find start room and end room and store it into corresponding struct.
 * ******************************************************************************/
void read_file(struct Directory* Dir, struct Room* room_array,char* last_dir){
  DIR *current_dir;
  struct dirent *dp;
  char* file_name;
  FILE* roomfp;
  FILE*fp;
  int step = 0;
  char cbuffer[64];
  char content[64];
  char startroom[64];
  char endroom[64];
  current_dir = opendir(".");
  int i = 0;
  while((dp = readdir(current_dir)) != NULL){
    file_name = dp->d_name;
    roomfp = fopen(file_name,"r");
   //find start room 
    while(fscanf(roomfp,"%s",content)!=EOF ){      //find the start room,and store into struct
    if(strstr(content,"START")){
      sprintf(startroom,file_name);
      strcpy(Dir->start_room,startroom);
     // printf("startroom: %s\n",Dir->start_room);
      }
    }
    fclose(roomfp);

    fp = fopen(file_name,"r");
    while(fscanf(fp,"%s",content)!=EOF ){       //find end rom and store into struct
    if(strstr(content,"END")){
      sprintf(endroom,file_name);
      strcpy(Dir->end_room,endroom);
     // printf("endroom: %s\n",Dir->end_room);
      }
    }

    fclose(fp);
  }
}

/*************************************************************
 *
 *           print_startroom
 *
 *this function is used to  print out information about start room
 *
 * **********************************************************/
void print_startroom(char* startroom){
  FILE* fp3;
  char connect[64];
  char array[12][24];
  int i=0,j=0;
   fp3 = fopen(startroom,"r");
    printf("CURRENT LOCATION: %s \n",startroom); 
    printf("POSSIBLE CONNECTIONS: ");
    
   if(fp3 == NULL){
      perror("Error opening file");
      exit(-1);
   }
    while(fgets(connect,sizeof(connect),fp3)!=NULL){
      
     if(strstr(connect,"CONNECTION")!=NULL){   //use strtok to get the string after connection
        char* token = strtok(connect,":");
        token = strtok(NULL," \n");
        strcpy(array[i],token);
	i++;
       }
    }
    for(j=0;j<i;j++){
      if(j==i-1){
        printf("%s.",array[j]);
      }
      else{
      printf("%s,",array[j]);
    }
    }
    fclose(fp3);
}

/**************************************************88
 *     write_time
 *
 * this function is used to write time into a file named
 * currentTime.txt in the upper directory.
 * ***********************************************/
 
void* write_time(){
  pthread_mutex_trylock(&myMutex);  //keep trying to lock until the main thread unlock the mutex
  time_t rawtime;
  struct tm *info;
  char buffer[80];
  
 // pthread_mutex_lock(&myMutex);
  FILE* timefp;
  time(&rawtime);
  info = localtime(&rawtime);
  strftime(buffer,80,"%I:%M%p,%A,%B %d,%Y",info);  //write time to the file named currentTime
  timefp = fopen("../currentTime.txt","w+");        //create file in upper directory and write time to it
  fprintf(timefp,"%s",buffer);
 // char cwd[64];
 // if(getcwd(cwd,sizeof(cwd))!=NULL){
 // fprintf(stdout,"current workding dir: %s\n",cwd);
 // }
 // printf("in write time function");
  fclose(timefp);
  pthread_mutex_unlock(&myMutex);                 //after finishing write time,then unlock
}

/*****************************************************
 *   read_time
 *this function is used to write content of a file named
 *currentTime.txt and print it out.
 *
 * ****************************************************/
void read_time(){                                  
  FILE* readfp;
  char timeline[80];
  readfp =fopen("../currentTime.txt","r");
    while(fgets(timeline,sizeof(timeline),readfp)!=NULL){
    printf("%s\n",timeline);
  }

}
