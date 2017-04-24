#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GAMEWIDTH 160
#define GAMEHEIGHT 105
#define NUMBEROFROOMS 10
#define N GAMEWIDTH * GAMEHEIGHT;

int GAMEBOARD[GAMEWIDTH][GAMEHEIGHT];

char *GAMEMAP[GAMEWIDTH][GAMEHEIGHT];

typedef int bool;
#define true 1
#define false 0

struct Room{
  int x;
  int y;
  int width;
  int height;
};

struct CoordPair{
  int x;
  int y;
};

struct Room rooms[NUMBEROFROOMS];

void init_gameBoardArray(){
  int i,j;
  for(i=0;i<GAMEWIDTH+1;i++){
      for(j=0;j<GAMEHEIGHT+1;j++){
          /*Initialize everything to zero*/
          GAMEBOARD[i][j] = 0;
      }
  }
  for(i=0;i<GAMEWIDTH;i++){
      for(j=0;j<GAMEHEIGHT;j++){
          /*Initialize everything that is tunnable to one*/
          GAMEBOARD[i][j] = 1;
      }
  }

}

void init_roomsArray(){
  for(int i=0;i<NUMBEROFROOMS+1;i++){
    rooms[i].x = 0;
    rooms[i].y = 0;
    rooms[i].width = 0;
    rooms[i].height = 0;
  }
}

void init_dungeon(int argc, char **argv){
  if(argc>1){
    srand(atoi(argv[1]));
  }else{
    srand(time(NULL));
  }
}

int randomRange(int min, int max){
  return rand() % (max + 1 - min)+min;
}

struct Room makeNewRoom(){
  struct Room tmpRoom;
  tmpRoom.x = randomRange(1,GAMEWIDTH-25);
  tmpRoom.y = randomRange(1,GAMEHEIGHT-20);
  tmpRoom.width = randomRange(7,25);
  tmpRoom.height = randomRange(5,20);
  return tmpRoom;
}
/*Returns true if the rooms overlap*/
bool checkIfOverlapping(struct Room room1, struct Room room2){
  if (!(room1.x-1 > room2.x + room2.width+1 || room2.x-1 > room1.x + room1.width +1)&&
      !(room1.y-1 > room2.y + room2.height+1 || room2.y-1 > room1.y + room1.height +1))  {
         return true;
       }
  return false;
}

/*Returns true if the room is NOT valid and false if it is*/
bool roomIsInvalid(struct Room roomToCheck,int numberOfRoomsFound){
  //Rooms cannot touch any other rooms
  //They must be at least 1 cell away from any other room
  bool result = false;
  for(int i = 0; i < numberOfRoomsFound; i++){
    if(checkIfOverlapping(roomToCheck,rooms[i])){
          result = true;
    }
  }
    return result;
}

void printRoom(struct Room roomToPrint){
  printf("The start X of the room is: %d\n",roomToPrint.x);
  printf("The end X of the room is: %d\n",roomToPrint.x + roomToPrint.width);
  printf("The start Y of the room is: %d\n",roomToPrint.y);
  printf("The end Y of the room is: %d\n",roomToPrint.y + roomToPrint.height);
}
void createRooms(){
  int numberOfRoomsFound = 0;
  for(int i=0;i<NUMBEROFROOMS;i++){
    bool isRoomInvalid = false;
    struct Room tempRoom;
    do {
      tempRoom = makeNewRoom();
      isRoomInvalid = roomIsInvalid(tempRoom,numberOfRoomsFound);
    } while(isRoomInvalid);
    numberOfRoomsFound++;
    rooms[i].x = tempRoom.x;
    rooms[i].y = tempRoom.y;
    rooms[i].width = tempRoom.width;
    rooms[i].height = tempRoom.height;
    //printRoom(rooms[i]);
  }
}

void initPath(struct CoordPair path[500]){
  for(int i=0;i<500;i++){
    path[i].x = -1;
    path[i].y = -1;
  }
}

bool pairIsNotInRoom(struct CoordPair pair){
  for(int i=0;i<NUMBEROFROOMS;i++){
    if(pair.x>rooms[i].x && pair.x <rooms[i].x + rooms[i].width){
      if(pair.y > rooms[i].y && pair.y < rooms[i].y + rooms[i].height){
        return false;
      }
    }
  }
  return true;
}

void addToEndOfPath(struct CoordPair pair, struct CoordPair path[500], int numberOfTunnels){
    path[numberOfTunnels-1].x = pair.x;
    path[numberOfTunnels-1].y = pair.y;
}

bool isLastPairTheTarget(struct CoordPair path[500], struct CoordPair target){
  for(int i=0;i<500;i++){
    if(path[i+1].x==-1){
      if(path[i].x == target.x && path[i].y == target.y){
        return true;
      }
    }
  }
  return false;
}

int findLastPairXvalue(struct CoordPair path[500]){
  for(int i=0;i<500;i++){
    if(path[i+1].x ==-1){
      return path[i].x;
    }
  }
  return 1;
}

// void printPath(struct CoordPair path[99]){
//   for(int i=0;i<99;i++){
//     printf("Path number %d is: ( %d , %d )\n",i,path[i].x,path[i].y);
//   }
// }

void addPathToGameMap(struct CoordPair path[500]){
  printf("Adding Path to game map\n");
  for(int i=0;i<500;i++){
    if(path[i].x!=-1){
      GAMEMAP[path[i].x][path[i].y] = "#";
    }else{
      printf("The last pair in path is: ( %d , %d )",path[i-1].x,path[i-1].y);
      break;
    }
  }
  printf("Path added\n");
}

void createShortestPath(struct CoordPair source, struct CoordPair target){
  struct CoordPair path[500];
  initPath(path);
  int numberOfTunnels = 0;
  bool XisDone = false;
  bool YisDone = false;
  int startX = source.x;
  int startY = source.y;
  if(source.x < target.x && !XisDone){
    printf("Source x is less than target\n");
    while(startX != target.x+1){
      numberOfTunnels++;
      struct CoordPair temp;
      temp.x = startX;
      temp.y = source.y;
      addToEndOfPath(temp, path, numberOfTunnels);
      startX++;
    }
    XisDone = true;
  }
  else if(source.x > target.x && !XisDone){
    printf("Source x is greater than target\n");
    while(startX != target.x-1){
      numberOfTunnels++;
      struct CoordPair temp;
      temp.x = startX;
      temp.y = source.y;
      addToEndOfPath(temp, path, numberOfTunnels);
      startX--;
    }
    XisDone = true;
  }
  if(source.y < target.y && !YisDone){
    printf("Source Y is less than target\n");
    while(startY != target.y+1){
      numberOfTunnels++;
      struct CoordPair temp;
      int x = findLastPairXvalue(path);
      temp.x = x;
      temp.y = startY;
      addToEndOfPath(temp, path, numberOfTunnels);
      startY++;
    }
    printf("Y is done!");
    YisDone = true;
  }
  else if(source.y > target.y && !YisDone){
    printf("Source Y is greater than target\n");
    while(startY != target.y-1){
      numberOfTunnels++;
      struct CoordPair temp;
      int x = findLastPairXvalue(path);
      temp.x = x;
      temp.y = startY;
      addToEndOfPath(temp, path, numberOfTunnels);
      startY--;
    }
    YisDone = true;
  }
  //printf("The last point is: ( %d , %d )",startX, startY);
  //printPath(path);
  addPathToGameMap(path);
}

void createTunnels(){
  for(int i=0;i < NUMBEROFROOMS-1;i++){
    struct CoordPair source;
    source.x = randomRange(rooms[i].x,rooms[i].x+rooms[i].width);
    source.y = randomRange(rooms[i].y,rooms[i].y+rooms[i].height);
    struct CoordPair target;
    target.x = randomRange(rooms[i+1].x,rooms[i+1].x+rooms[i+1].width);
    target.y = randomRange(rooms[i+1].y,rooms[i+1].y+rooms[i+1].height);
    //Compute shortest Path between two points....
    printf("Creating path between room %d and %d\n", i,i+1);
    printf("Path from ( %d , %d ) and ( %d , %d )\n",source.x, source.y, target.x, target.y);
    createShortestPath(source, target);
    printf("Created Shortest Path\n");
  }
}

void addRoomsToGameMap(){
  for(int i=0;i<NUMBEROFROOMS+1;i++){
    for(int x=rooms[i].x;x<rooms[i].x + rooms[i].width+1;x++){
      for(int y=rooms[i].y;y<rooms[i].y + rooms[i].height+1;y++){
        GAMEMAP[x][y] = ".";
      }
    }
  }
}

void init_gameMap(){
  int i,j;
  for(i=0;i<GAMEWIDTH;i++){
      for(j=0;j<GAMEHEIGHT;j++){
          /*Initialize everything to a space*/
          GAMEMAP[i][j] = " ";
      }
  }
}

void populateGameMap(){
  addRoomsToGameMap();
}

void printDungeon(){
  populateGameMap();
  for(int j=0;j<GAMEHEIGHT;j++){
    for(int i=0;i<GAMEWIDTH;i++){
        printf("%s",GAMEMAP[i][j]);
    }
    printf("\n");
  }
}

void init_game(int argc, char **argv){
  init_gameBoardArray();
  init_roomsArray();
  init_dungeon(argc,argv);
  init_gameMap();
}


int main(int argc, char *argv[]){
  init_game(argc,argv);

  createRooms();

  printf("Creating Tunnels\n");

  createTunnels();

  printDungeon();

  return 0;
}
