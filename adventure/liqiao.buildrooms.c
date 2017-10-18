#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
void build_connections(int m[][7]);
void printmatrix(int matrix[][7]);
void shuffle_roomnames(char* room_names[],int size);
int main(){
  int k,j,m,n;
  /*************make a directory***************************/
  pid_t myPID = getpid();
  char* myONID = "liqiao";
  char myRoomsDirName[64];
  memset(myRoomsDirName, '\0', sizeof(myRoomsDirName));
  sprintf(myRoomsDirName, "%s.rooms.%d", myONID, myPID);
  mkdir(myRoomsDirName,0770);
  //printf("Built-up rooms dir is: \"%s\"\n", myRoomsDirName);
  //printf("here\n");
  
  /**********declare roomsnames and room type*********************/
  char* room_names[10] = {"apple","book","cat","dog","egg","flower","green","hat","ice","jelly"};  
  char* room_type[3]  = {"START_ROOM","END_ROOM","MID_ROOM"};
  
  /********* build matrix and connections***********************/
  int matrix[7][7];
  build_connections(matrix);
  
  /*shuffle the room_namesa and select 7 names*/
  shuffle_roomnames(room_names,10);
  char* seven_names[7];
  for(k=0;k<7;k++){
    seven_names[k]=room_names[k];               
  }

  /*generate files for each room*/
  chdir(myRoomsDirName);
  for(j=1;j<=7;j++){
    char filePathString[20];
   // sprintf(filePathString,"room%d.txt",j);
    sprintf(filePathString,"%s",seven_names[j-1]);
    FILE* fp;
    fp = fopen(filePathString,"w+");
    fprintf(fp,"ROOM NAME: %s\n", seven_names[j-1]);
       
	int count =0;      //used to count how many connection does room# have
      for(n=0;n<7;n++){
        if(matrix[j-1][n]==1){
	  count+=1;
	  fprintf(fp,"CONNECTION %d: %s \n", count,seven_names[n]);
	}
      }
     if(j==1){
	 fprintf(fp,"ROOM TYPE: START_ROOM\n");
	 } 
     else if(j==7){
     	fprintf(fp,"ROOM TYPE: END_ROOM\n");
     }
     else{
       fprintf(fp,"ROOM TYPE: MID_ROOM\n");
     }
    fclose(fp);
  }

//  int i=0;
//  for(i = 0;i<7;i++){
//    printf("%s\n",seven_names[i]);
//  }
return 0;
}
/*********************************************************
 *Function: build_connection
 *Description:use adjacent matrix in graph to build rooms and add connections
 *
 ***********************************************************/
void build_connections(int m[][7]){
    time_t t;
    int room_num = 7;
    int i, j;
    srand((unsigned) time(&t));
    for(i = 0;i < room_num; i++){
      int count = 0;
      for( j = i; j < room_num;j++){
	int ifconnect = 0;
        if(i == j){
	  m[i][j]=0;                              //room can not connect itself
	}
	else{
	  ifconnect = rand()% 2;                 //ifconnect == 1: connect to j 
	   					 //ifconnect == 0: do not connect
	  if(ifconnect == 1){
	    count += 1;
	  }
	  if(count >=3){                          //if i has connect other room more than 3 times, the connection to other room depends on ifconnect ramdonly
	    m[i][j]=m[j][i]=ifconnect;
	  }
	  else{
	    m[i][j]=m[j][i]=1;
	  }
	}
      }
    }    
}

/*****************************************************************************8
 *Function:
 *Description
 * 
 *
 *************************************************************************** */
void printmatrix(int matrix[][7]){
  int i ,j;
  for(i=0;i<7;i++){
    for(j=0;j<7;j++){
      printf("%d	" , matrix[i][j]); 
    }
    printf("\n");
  }
}

/*****************************************************************************8
 *Function:
 *Description
 * 
 *
 *************************************************************************** */
void shuffle_roomnames(char* room_names[],int size){
 int i,j,r;
 char* temp;
// memset(temp,'\0',2);
  if(size>1){
   for(i=size-1;i>0;i-- ){
     r = rand()% i;                        
     temp = room_names[r];                      //swap cstring by swap pointer
     room_names[r]=room_names[i];
     room_names[i]= temp;
   }
 } 
}

