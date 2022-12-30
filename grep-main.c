#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grep.h"

int rank=0, size=0;
float TOTAL_LINES = 2000.0;

int main (int argc, char * argv[])
{
  if(argc != 3) { //THIS IS 3 WHEN SEARCHING THE WORD, ONLY READING THE FILE THIS IS 2
    printf("Expected 2 inputs, got %d\n",(argc-1));
    return 0;
  }

  number_and_line* input_lines;
  number_and_line* local_lines;
  number_and_line* found_lines;
  int local_lines_number;
  int found_elems=0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank==0){
    input_lines = malloc(sizeof(number_and_line)*((int)TOTAL_LINES)); //allocating memory for struct pointers
    for(int i=0;i<TOTAL_LINES;i++){                      //allocating memory for the strings
      input_lines[i].string = malloc(sizeof(char)*LINELENGTH);
    }
  }
  else input_lines=NULL;

  //allocate memory for local pointers
  local_lines = malloc(sizeof(number_and_line)*((int)TOTAL_LINES));
  for(int i=0;i<TOTAL_LINES;i++){
    local_lines[i].string = malloc(sizeof(char)*LINELENGTH);
  }
  
  //allocate memory for strings found
  found_lines = malloc(sizeof(number_and_line)*((int)TOTAL_LINES));
  for(int i=0;i<TOTAL_LINES;i++){
    found_lines[i].string = malloc(sizeof(char)*LINELENGTH);
  }
  
  //read file only if process 0
  get_lines(input_lines,argv[2],&local_lines_number,local_lines);
  // printf("\n\nGETLINES\n");
  // for(int i=0;i<local_lines_number;i++){
  //   printf("%d) num: %d\tstring: %s",rank,local_lines[i].num,local_lines[i].string);
  // }

  search_string(local_lines, argv[1],local_lines_number,found_lines,&found_elems); //ARGV[1] IS THE WORD TO LOOK FOR (LOOK AT THE NOTEBOOK)
  // printf("\n\nFOUNDLINES\n");
  // for(int i=0;i<found_elems;i++){
  //   printf("%d) num: %d\tstring: %s",rank,found_lines[i].num,found_lines[i].string);
  // }

  print_result(found_lines,found_elems);

  //actually have to deallocate each single pointer I think
  free(input_lines);
  free(local_lines);
  free(found_lines);
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}

void get_lines(number_and_line* input_lines, char* file_name, int *local_lines_number, number_and_line* local_lines){
  //root reads the file and sends everything to other threads
  if(rank==0){
    FILE *file;

    file = fopen(file_name,"r");
    TOTAL_LINES = 0.0;

    //read file
    while(fgets(input_lines[(int) TOTAL_LINES].string,LINELENGTH+1,file) != NULL){
      input_lines[(int) TOTAL_LINES].num = TOTAL_LINES+1;
      TOTAL_LINES++;
    }
    fclose(file);

    //send total line number
    for(int i=1;i<size;i++){
      MPI_Send(&TOTAL_LINES,1,MPI_INT,i,0,MPI_COMM_WORLD);
    }

    float dest=0.0;
    for(int i=0;i<TOTAL_LINES;i++){
      if(i == (int)((TOTAL_LINES/(float) size)*(dest+1))) dest++;
      MPI_Send(&(input_lines[i].num),1,MPI_INT,dest,0,MPI_COMM_WORLD); //send line number
      MPI_Send(input_lines[i].string,LINELENGTH,MPI_CHAR,dest,0,MPI_COMM_WORLD); //send line
    }
  }
  //all other threads receive TOTAL_LINES
  else {
    MPI_Recv(&TOTAL_LINES,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);  //receive total line number
  }

  *local_lines_number = (int)((TOTAL_LINES/(float) size)*(rank+1)) - (int)((TOTAL_LINES/(float) size)*(rank));

  //all threads receive the lines
  for(int i=0;i<*local_lines_number;i++){
    MPI_Recv(&(local_lines[i].num),1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);   //receive line number
    MPI_Recv(local_lines[i].string,LINELENGTH,MPI_CHAR,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //receive line
  }

  // printf("\n\nGETLINES\n");
  // for(int i=0;i<*local_lines_number;i++){
  //   printf("%d) num: %d\tstring: %s",rank,local_lines[i].num,local_lines[i].string);
  // }

  return;
}

void search_string(number_and_line* local_lines, char* search_string, int local_lines_number, number_and_line* found_lines,int* found_elems){
  for(int i=0;i<local_lines_number;i++){
    if(strstr(local_lines[i].string,search_string)){
      found_lines[*found_elems].num = local_lines[i].num;
      strcpy(found_lines[*found_elems].string,local_lines[i].string);
      (*found_elems)++;
    }
  }

  // printf("\n\nSEARCHSTRING\n");
  // for(int i=0;i<*found_elems;i++){
  //   printf("%d) num: %d\tstring: %s",rank,found_lines[i].num,found_lines[i].string);
  // }

  return;
}

void print_result(number_and_line* found_lines, int found_elems){
  FILE* file;
  int sum=0;

  MPI_Reduce(&found_elems,&sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

  // if(rank==0){
  //   printf("\n\nPRINTRESULT\n");
  //   printf("sum: %d\n",sum);
  // }

  if(rank!=0){
    for(int i=0;i<found_elems;i++){
      MPI_Send(&(found_lines[i].num),1,MPI_INT,0,1,MPI_COMM_WORLD); //send line number
      MPI_Send(found_lines[i].string,LINELENGTH,MPI_CHAR,0,2,MPI_COMM_WORLD); //send line
    }
  } else {
    for(int i=found_elems;i<sum;i++){ //don't add again the elements already in thread 0
      MPI_Recv(&(found_lines[i].num),1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);   //receive line number
      MPI_Recv(found_lines[i].string,LINELENGTH,MPI_CHAR,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //receive line
    }
  }

  file = fopen("output.txt","w");
  if(rank==0){
    for(int i=0;i<sum;i++){
      fprintf(file,"%d:%s",found_lines[i].num,found_lines[i].string);
    }
  }
  fclose(file);
  return;
}