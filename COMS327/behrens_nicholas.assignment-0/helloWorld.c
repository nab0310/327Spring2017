#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int WIDTH = 24;
int LENGTH = 80;

void init_array(int array[WIDTH][LENGTH]){
    int i,j;
    for(i=0;i<WIDTH;i++){
        for(j=0;j<LENGTH;j++){
            /*Initialize everything to zero*/
            array[i][j] = 0;
        }
    }
}

void print_array(int array[WIDTH][LENGTH]){
    int k,l;

    for(k=0;k<WIDTH;k++){
        for(l=0;l<LENGTH;l++){
            if(array[k][l]==1){
                printf("O");
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }
}
/*Copies array to preserve values for life caluclations*/
void copy(int array1[WIDTH][LENGTH], int array2[WIDTH][LENGTH]){
    int i,j;
    for(i=0;i<WIDTH;i++){
        for(j=0;j<LENGTH;j++){
            array2[i][j] = array1[i][j];
        }
    }
}

void life(int array[WIDTH][LENGTH]){
    int tmp[WIDTH][LENGTH];
    int h,j;
    copy(array,tmp);
    for(h=0;h<WIDTH;h++){
        for(j=0;j<LENGTH;j++){
            /*Take the value of the neighbors*/
            int count =-1;
            /*If we do have an edge case we will create count and change it
            from -1*/
            if(h-1<0){
                count = array[WIDTH-1][j] +
                array[h+1][j]+
                array[h][j-1]+
                array[h][j+1]+
                array[WIDTH-1][j-1]+
                array[WIDTH-1][j+1]+
                array[h+1][j-1]+
                array[h+1][j+1];
            }
            else if(h+1==WIDTH){
                count = array[h-1][j] +
                array[0][j]+
                array[h][j-1]+
                array[h][j+1]+
                array[h-1][j-1]+
                array[h-1][j+1]+
                array[0][j-1]+
                array[0][j+1];
            }
            if(j-1<0){
                count = array[h-1][j] +
                array[h+1][j]+
                array[h][LENGTH-1]+
                array[h][j+1]+
                array[h-1][LENGTH-1]+
                array[h-1][j+1]+
                array[h+1][LENGTH-1]+
                array[h+1][j+1];
            }
            else if(j+1==LENGTH){
                count = array[h-1][j] +
                array[h+1][j]+
                array[h][j-1]+
                array[h][0]+
                array[h-1][j-1]+
                array[h-1][0]+
                array[h+1][j-1]+
                array[h+1][0];
            }
            if(count==-1){
                count = array[h-1][j] +
                array[h+1][j]+
                array[h][j-1]+
                array[h][j+1]+
                array[h-1][j-1]+
                array[h-1][j+1]+
                array[h+1][j-1]+
                array[h+1][j+1];
            }
            /*Game of life logic*/
            if(tmp[h][j] == 1){
                if(count == 2 || count == 3){
                    tmp[h][j] = 1;
                }else{
                    tmp[h][j] = 0;
                }
            }
            if(tmp[h][j] == 0){
                if(count == 3){
                    tmp[h][j] = 1;
                }else{
                    tmp[h][j] = 0;
                }
            }
        }
    }
    copy(tmp, array);
}

void populateArray(int array[WIDTH][LENGTH],int argc, char **argv){
  int i;
  for(i=1;i<argc;i+=2){
    int xval = atoi(argv[i]);
    int yval = atoi(argv[i+1]);
    printf("Xval passed in: %d, Yval passed in: %d",xval,yval);
    array[yval][xval] = 1;
  }
}

int main(int argc, char **argv){
  int array[WIDTH][LENGTH];

  printf("Hello World!\n");

  init_array(array);

  populateArray(array,argc,argv);

  int numberOfCycles = 0;

  while(1){
      print_array(array);
      life(array);
      usleep(83333);
      numberOfCycles++;
      printf("Time: %d\n", numberOfCycles);
  }

  return 0;
}
