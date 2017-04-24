#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include "dungeon.h"
#include "utils.h"
#include "heap.h"

typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} corridor_path_t;

uint32_t get_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position[dim_x]) &&
        (x < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y >= d->rooms[i].position[dim_y]) &&
        (y < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return i;
    }
  }

  return 0;
}
void gen_monsters(dungeon_t *d){
  d->monsters = malloc(sizeof (*d->monsters) * d->num_monsters);
  int i,j,pc_room;
  for(i=0;i<d->num_monsters;i++){
    char characteristics = rand() & 0x0000000f;
    printf("The monster number %d will be %d\n",i,characteristics%16);
    if(characteristics%16>10){
      printf("Char value is %d\n",characteristics%16);
      printf("We are turning it into %d\n", characteristics%16 + 87);
    }
    if ((characteristics & NPC_SMART)) {
      printf("The monster number %d will be SMART\n",i);
    }
    if ((characteristics & NPC_TELE)) {
      d->monsters[i].destination[dim_y] = d->pc.position[dim_y];
      d->monsters[i].destination[dim_x] = d->pc.position[dim_x];
      printf("The monster number %d will be TELEPATHIC\n",i);
    }
    if ((characteristics & NPC_TUNNEL)) {
      printf("The monster number %d can TUNNEL\n",i);
    }
    if ((characteristics & NPC_ERRATIC)) {
      printf("The monster number %d is ERRATIC\n",i);
    }
    d->monsters[i].characteristics = characteristics;
    pc_room = get_room(d, d->pc.position[dim_y],d->pc.position[dim_x]);
    printf("PC is in room %d\n", pc_room);
    j = rand() % d->num_rooms;
    while(j==pc_room){
      printf("Tried to put the monster into the PC's room\n");
      j = rand() % d->num_rooms;
    }
    d->monsters[i].position[dim_x] = (d->rooms[j].position[dim_x] +
                            (rand() % d->rooms[j].size[dim_x]));
    d->monsters[i].position[dim_y] = (d->rooms[j].position[dim_y] +
                            (rand() % d->rooms[j].size[dim_y]));
    d->monsters[i].speed = rand_range(5,20);
    d->monsters[i].nTicks = 1000/d->monsters[i].speed;
    d->monsters[i].alive = 1;
    printf("Monster number %d is at (y, x): %d, %d\n",i,
     d->monsters[i].position[dim_y], d->monsters[i].position[dim_x]);
  }
}

static int32_t speed_cmp(const void *key, const void *with) {
  if(((events_t *) key)->time - ((events_t *) with)->time==0){
    return ((events_t *) key)->seqNum - ((events_t *) with)->seqNum;
  }else{
    return ((events_t *) key)->time - ((events_t *) with)->time;
  }
}

uint32_t to_printable_character(char givenChar){
  if(givenChar%16<=9){
    return givenChar%16 + 48;
  }else{
    return givenChar%16 + 87;
  }
}

void init_characters(dungeon_t *d){
  int i;
  d->events = malloc(sizeof(*d->events) * (d->num_monsters+1));
  //PC
  d->events[0].seqNum = 0;
  d->events[0].time = d->pc.nTicks;
  d->events[0].type = player;
  d->events[0].p.pc = &d->pc;
  printf("The number of ticks for PC is %d\n", d->pc.nTicks);

  //Monsters
  for(i=0;i<d->num_monsters;i++){
    d->events[i+1].time = d->monsters[i].nTicks;
    d->events[i+1].seqNum = i+1;
    d->events[i+1].type = monster;
    d->events[i+1].p.monster = &d->monsters[i];
  }
}

void make_heap(dungeon_t *d){
  heap_t h;
  int i;

  heap_init(&h,speed_cmp,NULL);

  for(i=0;i<d->num_monsters+1;i++){
    heap_insert(&h, &d->events[i]);
  }

  d->heap = h;
}

static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position[dim_x]) &&
        (x < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y >= d->rooms[i].position[dim_y]) &&
        (y < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return 1;
    }
  }

  return 0;
}

int checkAndReduceTunnelValue(dungeon_t *d,int y, int x){
  pair_t p;
  p[dim_y] = y;
  p[dim_x] = x;
  if(mappair(p) == ter_wall){
    if(d->hardness[y][x]<85){
      return 0;
    }else{
      return d->hardness[y][x] - 85;
    }
  }
  return d->hardness[y][x];
}

void update(dungeon_t *d){
  events_t *e;
  int erratic,tunneling,done,smart,move,x,y,cost;
  pair_t p,dest,lowestCost;

    e = heap_remove_min(&d->heap);
    if(e->type == player){
      e->time = e->time + e->p.pc->nTicks;
        //printf("We are moving the @ character\n");
        int moveCase = rand() % 4;
        switch(moveCase){
          case 0:
            if(in_room(d,e->p.pc->position[dim_y]-1,e->p.pc->position[dim_x])){
              e->p.pc->position[dim_y] = e->p.pc->position[dim_y] - 1;
            }else{
              e->p.pc->position[dim_y] = e->p.pc->position[dim_y] + 1;
            }
            break;
          case 1:
            if(in_room(d,e->p.pc->position[dim_y]+1,e->p.pc->position[dim_x])){
              e->p.pc->position[dim_y] = e->p.pc->position[dim_y] + 1;
            }else{
              e->p.pc->position[dim_y] = e->p.pc->position[dim_y] - 1;
            }
            break;
          case 2:
            if(in_room(d,e->p.pc->position[dim_y],e->p.pc->position[dim_x]-1)){
              e->p.pc->position[dim_x] = e->p.pc->position[dim_x] - 1;
            }else{
              e->p.pc->position[dim_x] = e->p.pc->position[dim_x] + 1;
            }
            break;
          case 3:
            if(in_room(d,e->p.pc->position[dim_y],e->p.pc->position[dim_x]+1)){
              e->p.pc->position[dim_x] = e->p.pc->position[dim_x] + 1;
            }else{
              e->p.pc->position[dim_x] = e->p.pc->position[dim_x] - 1;
            }
            break;
        }
        int loopMonsters=0;
        for(loopMonsters = 0; loopMonsters< d->num_monsters;loopMonsters++){
          if((d->pc.position[dim_x] + 2 == d->monsters[loopMonsters].position[dim_x] ||
             d->pc.position[dim_x] - 2 == d->monsters[loopMonsters].position[dim_x]) &&
             (d->pc.position[dim_y] + 2 == d->monsters[loopMonsters].position[dim_y] ||
             d->pc.position[dim_y] - 2 == d->monsters[loopMonsters].position[dim_y])){
               d->monsters[loopMonsters].alive = 0;
             }
        }
        if(e->p.pc->alive != 0 ){
          heap_insert(&d->heap,e);
        }
    }else{
      printf("We have a monster\n");
      e->time = e->time + e->p.monster->nTicks;
      smart = 0;
      erratic = 0;
      done = 1;
      tunneling = 0;
      move = 0;
      dest[dim_y] = 500;
      dest[dim_x] = 500;
      printf("Player Room is: %d\n",  get_room(d,d->pc.position[dim_y],d->pc.position[dim_x]));
      printf("Monster Room is: %d\n", get_room(d,e->p.monster->position[dim_y],e->p.monster->position[dim_x]));
      printf("Dest is (y,x): %d , %d\n",dest[dim_y], dest[dim_x] );
      if(get_room(d,e->p.monster->position[dim_y],e->p.monster->position[dim_x]) ==
         get_room(d,d->pc.position[dim_y],d->pc.position[dim_x])){
           printf("We are in the same room!\n");
           dest[dim_y] = d->pc.position[dim_y];
           dest[dim_x] = d->pc.position[dim_x];
         }
        //printf("We are moving the %c character\n", to_printable_character(e->p.monster->characteristics));
        if ((e->p.monster->characteristics & NPC_SMART)) {
          if(get_room(d,e->p.monster->position[dim_y],e->p.monster->position[dim_x]) ==
             get_room(d,d->pc.position[dim_y],d->pc.position[dim_x])){
               e->p.monster->destination[dim_y] = d->pc.position[dim_y];
               e->p.monster->destination[dim_x] = d->pc.position[dim_x];
             }
          smart = 1;
        }
        if ((e->p.monster->characteristics & NPC_TELE)) {
          e->p.monster->destination[dim_y] = d->pc.position[dim_y];
          e->p.monster->destination[dim_x] = d->pc.position[dim_x];
        }
        if ((e->p.monster->characteristics & NPC_TUNNEL)) {
          tunneling = 1;
        }
        if ((e->p.monster->characteristics & NPC_ERRATIC)) {
          erratic = 1;
          done = 0;
        }
        if(erratic){
          if(rand() % 2 < 1){
            while(!done){
            //Move Randomly
            int moveCase = rand() % 4;
            switch(moveCase){
              case 0:
                p[dim_y] = e->p.monster->position[dim_y] - 1;
                p[dim_x] = e->p.monster->position[dim_x];
                switch (mappair(p)) {
                  case ter_wall:
                    if(tunneling){
                      e->p.monster->position[dim_y] = e->p.monster->position[dim_y] - 1;
                      done = 1;
                    }else{
                      break;
                    }
                  break;
                  case ter_wall_immutable:
                    break;
                  case ter_floor:
                  case ter_floor_hall:
                  case ter_floor_room:
                  e->p.monster->position[dim_y] = e->p.monster->position[dim_y] - 1;
                  done = 1;
                  break;
                  case ter_debug:
                  break;
                }
                break;
              case 1:
              p[dim_y] = e->p.monster->position[dim_y] + 1;
              p[dim_x] = e->p.monster->position[dim_x];
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_y] = e->p.monster->position[dim_y] + 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_hall:
                case ter_floor_room:
                e->p.monster->position[dim_y] = e->p.monster->position[dim_y] + 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
              break;
              case 2:
              p[dim_y] = e->p.monster->position[dim_y];
              p[dim_x] = e->p.monster->position[dim_x] - 1;
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_x] = e->p.monster->position[dim_x] - 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_room:
                case ter_floor_hall:
                e->p.monster->position[dim_x] = e->p.monster->position[dim_x] - 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
              break;
              case 3:
              p[dim_y] = e->p.monster->position[dim_y];
              p[dim_x] = e->p.monster->position[dim_x] + 1;
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_x] = e->p.monster->position[dim_x] + 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_hall:
                case ter_floor_room:
                e->p.monster->position[dim_x] = e->p.monster->position[dim_x] + 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
            }
          }
        }else{
          //Move according to other characteristics
          move = 1;
        }
      }else{
        printf("Im not erratic\n");
        move = 1;
      }
      if(move == 1){
        printf("Lets boogie!\n");
        //Move according to other characteristics
        //Smart and doesnt know where the PC is (Non-Telepathic), dont move
        //Not smart, and doesnt know where the PC is (Non-Telepathic), move erratic
        if(smart){
          if(!e->p.monster->destination[dim_y]){
            printf("I'm not telepathic\n");
            if(dest[dim_x] != 500){
              printf("Im in the same room\n");
              if(tunneling){
                x = dest[dim_x];
                y = dest[dim_y];
                lowestCost[dim_y] = y;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_tunnel[y][x + 1] + checkAndReduceTunnelValue(d,y,x + 1);
                if(d->pc_tunnel[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1)<cost){
                  lowestCost[dim_y] = y;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_tunnel[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1);
                }
                if(d->pc_tunnel[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x;
                  cost = d->pc_tunnel[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x);
                }
                if(d->pc_tunnel[y - 1][x] +  checkAndReduceTunnelValue(d,y - 1,x)<cost){
                  lowestCost[dim_y] = y -1;
                  lowestCost[dim_x] = x;
                  cost = d->pc_tunnel[y - 1][x] + checkAndReduceTunnelValue(d,y - 1,x);
                }
                if(d->pc_tunnel[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1)<cost){
                  lowestCost[dim_y] = y - 1;
                  lowestCost[dim_x] = x + 1;
                  cost = d->pc_tunnel[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1);
                }
                if(d->pc_tunnel[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x + 1;
                  cost = d->pc_tunnel[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1);
                }
                if(d->pc_tunnel[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1)<cost){
                  lowestCost[dim_y] = y - 1;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_tunnel[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1);
                }
                if(d->pc_tunnel[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_tunnel[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1);
                }
                e->p.monster->position[dim_x] = lowestCost[dim_x];
                e->p.monster->position[dim_y] = lowestCost[dim_y];
              }else{
                x = dest[dim_x];
                y = dest[dim_y];
                lowestCost[dim_y] = y;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_distance[y][x + 1] + checkAndReduceTunnelValue(d,y,x + 1);
                if(d->pc_distance[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1)<cost){
                  lowestCost[dim_y] = y;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_distance[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1);
                }
                if(d->pc_distance[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x;
                  cost = d->pc_distance[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x);
                }
                if(d->pc_distance[y - 1][x] +  checkAndReduceTunnelValue(d,y - 1,x)<cost){
                  lowestCost[dim_y] = y -1;
                  lowestCost[dim_x] = x;
                  cost = d->pc_distance[y - 1][x] + checkAndReduceTunnelValue(d,y - 1,x);
                }
                if(d->pc_distance[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1)<cost){
                  lowestCost[dim_y] = y - 1;
                  lowestCost[dim_x] = x + 1;
                  cost = d->pc_distance[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1);
                }
                if(d->pc_distance[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x + 1;
                  cost = d->pc_distance[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1);
                }
                if(d->pc_distance[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1)<cost){
                  lowestCost[dim_y] = y - 1;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_distance[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1);
                }
                if(d->pc_distance[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1)<cost){
                  lowestCost[dim_y] = y + 1;
                  lowestCost[dim_x] = x - 1;
                  cost = d->pc_distance[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1);
                }
                e->p.monster->position[dim_x] = lowestCost[dim_x];
                e->p.monster->position[dim_y] = lowestCost[dim_y];
              }
            }
          }else{
            printf("I know where the PC is, it is at (x,y): %d , %d !\n", d->pc.position[dim_x], d->pc.position[dim_y]);
            if(tunneling){
              printf("I can tunnel\n");
              printf("I am at (x,y): %d , %d \n", e->p.monster->position[dim_x], e->p.monster->position[dim_y]);
              x = e->p.monster->position[dim_x];
              y = e->p.monster->position[dim_y];
              lowestCost[dim_y] = y;
              lowestCost[dim_x] = x + 1;
              cost = d->pc_tunnel[y][x + 1] + checkAndReduceTunnelValue(d,y,x + 1);
              if(d->pc_tunnel[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1)<cost){
                lowestCost[dim_y] = y;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_tunnel[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1);
              }
              if(d->pc_tunnel[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x;
                cost = d->pc_tunnel[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x);
              }
              if(d->pc_tunnel[y - 1][x] +  checkAndReduceTunnelValue(d,y - 1,x)<cost){
                lowestCost[dim_y] = y -1;
                lowestCost[dim_x] = x;
                cost = d->pc_tunnel[y - 1][x] + checkAndReduceTunnelValue(d,y - 1,x);
              }
              if(d->pc_tunnel[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1)<cost){
                lowestCost[dim_y] = y - 1;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_tunnel[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1);
              }
              if(d->pc_tunnel[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_tunnel[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1);
              }
              if(d->pc_tunnel[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1)<cost){
                lowestCost[dim_y] = y - 1;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_tunnel[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1);
              }
              if(d->pc_tunnel[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_tunnel[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1);
              }
              printf("I am putting the monster at (x,y) : %d , %d\n", lowestCost[dim_x],lowestCost[dim_y] );
              e->p.monster->position[dim_x] = lowestCost[dim_x];
              e->p.monster->position[dim_y] = lowestCost[dim_y];
            }else{
              x = e->p.monster->position[dim_x];
              y = e->p.monster->position[dim_y];
              lowestCost[dim_y] = y;
              lowestCost[dim_x] = x + 1;
              cost = d->pc_distance[y][x + 1] + checkAndReduceTunnelValue(d,y,x + 1);
              if(d->pc_distance[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1)<cost){
                lowestCost[dim_y] = y;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_distance[y][x - 1] +  checkAndReduceTunnelValue(d,y,x - 1);
              }
              if(d->pc_distance[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x;
                cost = d->pc_distance[y + 1][x] +  checkAndReduceTunnelValue(d,y + 1,x);
              }
              if(d->pc_distance[y - 1][x] +  checkAndReduceTunnelValue(d,y - 1,x)<cost){
                lowestCost[dim_y] = y -1;
                lowestCost[dim_x] = x;
                cost = d->pc_distance[y - 1][x] + checkAndReduceTunnelValue(d,y - 1,x);
              }
              if(d->pc_distance[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1)<cost){
                lowestCost[dim_y] = y - 1;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_distance[y - 1][x + 1] +  checkAndReduceTunnelValue(d,y - 1,x + 1);
              }
              if(d->pc_distance[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x + 1;
                cost = d->pc_distance[y + 1][x + 1] +  checkAndReduceTunnelValue(d,y + 1,x + 1);
              }
              if(d->pc_distance[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1)<cost){
                lowestCost[dim_y] = y - 1;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_distance[y - 1][x - 1] +  checkAndReduceTunnelValue(d,y - 1,x - 1);
              }
              if(d->pc_distance[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1)<cost){
                lowestCost[dim_y] = y + 1;
                lowestCost[dim_x] = x - 1;
                cost = d->pc_distance[y + 1][x - 1] +  checkAndReduceTunnelValue(d,y + 1,x - 1);
              }
              e->p.monster->position[dim_x] = lowestCost[dim_x];
              e->p.monster->position[dim_y] = lowestCost[dim_y];
            }
        }
      }else{
        printf("Im not smart!\n");
          if(!e->p.monster->destination[dim_y] && dest[dim_y] ==500){
            printf("I dont know where anything is.\n");
            done = 0;
            while(!done){
            //Move Randomly
            int moveCase = rand() % 4;
            switch(moveCase){
              case 0:
                p[dim_y] = e->p.monster->position[dim_y] - 1;
                p[dim_x] = e->p.monster->position[dim_x];
                switch (mappair(p)) {
                  case ter_wall:
                    if(tunneling){
                      e->p.monster->position[dim_y] = e->p.monster->position[dim_y] - 1;
                      done = 1;
                    }else{
                      break;
                    }
                  break;
                  case ter_wall_immutable:
                    break;
                  case ter_floor:
                  case ter_floor_hall:
                  case ter_floor_room:
                  e->p.monster->position[dim_y] = e->p.monster->position[dim_y] - 1;
                  done = 1;
                  break;
                  case ter_debug:
                  break;
                }
                break;
              case 1:
              p[dim_y] = e->p.monster->position[dim_y] + 1;
              p[dim_x] = e->p.monster->position[dim_x];
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_y] = e->p.monster->position[dim_y] + 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_hall:
                case ter_floor_room:
                e->p.monster->position[dim_y] = e->p.monster->position[dim_y] + 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
              break;
              case 2:
              p[dim_y] = e->p.monster->position[dim_y];
              p[dim_x] = e->p.monster->position[dim_x] - 1;
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_x] = e->p.monster->position[dim_x] - 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_room:
                case ter_floor_hall:
                e->p.monster->position[dim_x] = e->p.monster->position[dim_x] - 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
              break;
              case 3:
              p[dim_y] = e->p.monster->position[dim_y];
              p[dim_x] = e->p.monster->position[dim_x] + 1;
              switch (mappair(p)) {
                case ter_wall:
                  if(tunneling){
                    e->p.monster->position[dim_x] = e->p.monster->position[dim_x] + 1;
                    done = 1;
                  }else{
                    break;
                  }
                break;
                case ter_wall_immutable:
                  break;
                case ter_floor:
                case ter_floor_hall:
                case ter_floor_room:
                e->p.monster->position[dim_x] = e->p.monster->position[dim_x] + 1;
                done = 1;
                break;
                case ter_debug:
                break;
              }
            }
          }
          }
          if(dest[dim_y] != 500){
            printf("I can see the PC!\n");
            //Move in a straight line because im in the same room as the PC
            x = dest[dim_x];
            y = dest[dim_y];
            if(d->pc.position[dim_y] < y){
              e->p.monster->position[dim_y] = e->p.monster->position[dim_y] - 1;
            }else{
              e->p.monster->position[dim_y] = e->p.monster->position[dim_y] + 1;
            }
            if(d->pc.position[dim_x] < x){
              e->p.monster->position[dim_x] = e->p.monster->position[dim_x] + 1;
            }else{
              e->p.monster->position[dim_x] = e->p.monster->position[dim_x] - 1;
            }
          }
          if(e->p.monster->destination[dim_y]){
            printf("Im telepathic!\n");
            //Move in a straight line because im telepathic but not smart
            if(tunneling){
              printf("I can tunnel!\n");
              x = e->p.monster->position[dim_x];
              y = e->p.monster->position[dim_y];
              if(d->pc.position[dim_y] < y){
                lowestCost[dim_y] = e->p.monster->position[dim_y] - 1;
              }else if(d->pc.position[dim_y] > y){
                lowestCost[dim_y] = e->p.monster->position[dim_y] + 1;
              }
              if(d->pc.position[dim_x] < x){
                lowestCost[dim_x] = e->p.monster->position[dim_x] + 1;
              }else if (d->pc.position[dim_x] > x){
                lowestCost[dim_x] = e->p.monster->position[dim_x] - 1;
              }
              e->p.monster->position[dim_y] = lowestCost[dim_y];
              e->p.monster->position[dim_x] = lowestCost[dim_x];
            }else{
              x = e->p.monster->position[dim_x];
              y = e->p.monster->position[dim_y];
              if(d->pc.position[dim_y] < y){
                lowestCost[dim_y] = e->p.monster->position[dim_y] - 1;
              }else if(d->pc.position[dim_y] > y){
                lowestCost[dim_y] = e->p.monster->position[dim_y] + 1;
              }
              if(d->pc.position[dim_x] < x){
                lowestCost[dim_x] = e->p.monster->position[dim_x] - 1;
              }else if (d->pc.position[dim_x] > x){
                lowestCost[dim_x] = e->p.monster->position[dim_x] + 1;
              }
              if(mappair(lowestCost) != ter_wall){
                e->p.monster->position[dim_x] = lowestCost[dim_x];
                e->p.monster->position[dim_y] = lowestCost[dim_y];
              }
            }
          }
        }
      }
      if((e->p.monster->position[dim_x] + 2 == d->pc.position[dim_x] ||
         e->p.monster->position[dim_x] - 2 == d->pc.position[dim_x]) &&
         (e->p.monster->position[dim_y] + 2 == d->pc.position[dim_y] ||
         e->p.monster->position[dim_y] - 2 == d->pc.position[dim_y])){
           d->pc.alive = 0;
         }
       if(e->p.monster->alive !=0 ){
         heap_insert(&d->heap, e);
       }
    }
}

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

/* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
static void dijkstra_corridor_inv(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

#define hardnesspair_inv(p) (in_room(d, p[dim_y], p[dim_x]) ? \
                             224                            : \
                             (255 - hardnesspair(p)))

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
  pair_t e1, e2;

  e1[dim_y] = rand_range(r1->position[dim_y],
                         r1->position[dim_y] + r1->size[dim_y] - 1);
  e1[dim_x] = rand_range(r1->position[dim_x],
                         r1->position[dim_x] + r1->size[dim_x] - 1);
  e2[dim_y] = rand_range(r2->position[dim_y],
                         r2->position[dim_y] + r2->size[dim_y] - 1);
  e2[dim_x] = rand_range(r2->position[dim_x],
                         r2->position[dim_x] + r2->size[dim_x] - 1);

  /*  return connect_two_points_recursive(d, e1, e2);*/
  dijkstra_corridor(d, e1, e2);

  return 0;
}

static int create_cycle(dungeon_t *d)
{
  /* Find the (approximately) farthest two rooms, then connect *
   * them by the shortest path using inverted hardnesses.      */

  int32_t max, tmp, i, j, p, q;
  pair_t e1, e2;

  for (i = max = 0; i < d->num_rooms - 1; i++) {
    for (j = i + 1; j < d->num_rooms; j++) {
      tmp = (((d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])  *
              (d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])) +
             ((d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])  *
              (d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])));
      if (tmp > max) {
        max = tmp;
        p = i;
        q = j;
      }
    }
  }

  /* Can't simply call connect_two_rooms() because it doesn't *
   * use inverse hardnesses, so duplicate it here.            */
  e1[dim_y] = rand_range(d->rooms[p].position[dim_y],
                         (d->rooms[p].position[dim_y] +
                          d->rooms[p].size[dim_y] - 1));
  e1[dim_x] = rand_range(d->rooms[p].position[dim_x],
                         (d->rooms[p].position[dim_x] +
                          d->rooms[p].size[dim_x] - 1));
  e2[dim_y] = rand_range(d->rooms[q].position[dim_y],
                         (d->rooms[q].position[dim_y] +
                          d->rooms[q].size[dim_y] - 1));
  e2[dim_x] = rand_range(d->rooms[q].position[dim_x],
                         (d->rooms[q].position[dim_x] +
                          d->rooms[q].size[dim_x] - 1));

  dijkstra_corridor_inv(d, e1, e2);

  return 0;
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
  }

  create_cycle(d);

  return 0;
}

int gaussian[5][5] = {
  {  1,  4,  7,  4,  1 },
  {  4, 16, 26, 16,  4 },
  {  7, 26, 41, 26,  7 },
  {  4, 16, 26, 16,  4 },
  {  1,  4,  7,  4,  1 }
};

typedef struct queue_node {
  int x, y;
  struct queue_node *next;
} queue_node_t;

static int smooth_hardness(dungeon_t *d)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
#ifdef DUMP_HARDNESS_IMAGES
  FILE *out;
#endif
  memset(&hardness, 0, sizeof (hardness));

  /* Seed with some values */
  for (i = 1; i < 255; i += 5) {
    do {
      x = rand() % DUNGEON_X;
      y = rand() % DUNGEON_Y;
    } while (hardness[y][x]);
    hardness[y][x] = i;
    if (i == 1) {
      head = tail = malloc(sizeof (*tail));
    } else {
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

#ifdef DUMP_HARDNESS_IMAGES
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);
#endif

  /* Diffuse the vaules to fill the space */
  while (head) {
    x = head->x;
    y = head->y;
    i = hardness[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !hardness[y - 1][x - 1]) {
      hardness[y - 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !hardness[y][x - 1]) {
      hardness[y][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness[y + 1][x - 1]) {
      hardness[y + 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !hardness[y - 1][x]) {
      hardness[y - 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < DUNGEON_Y && !hardness[y + 1][x]) {
      hardness[y + 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness[y - 1][x + 1]) {
      hardness[y - 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < DUNGEON_X && !hardness[y][x + 1]) {
      hardness[y][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness[y + 1][x + 1]) {
      hardness[y + 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  /* And smooth it a bit with a gaussian convolution */
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
            s += gaussian[p][q];
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      d->hardness[y][x] = t / s;
    }
  }
  /* Let's do it again, until it's smooth like Kenny G. */
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
            s += gaussian[p][q];
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      d->hardness[y][x] = t / s;
    }
  }

#ifdef DUMP_HARDNESS_IMAGES
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&d->hardness, sizeof (d->hardness), 1, out);
  fclose(out);
#endif

  return 0;
}

static int empty_dungeon(dungeon_t *d)
{
  uint8_t x, y;

  smooth_hardness(d);
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = ter_wall;
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = ter_wall_immutable;
        hardnessxy(x, y) = 255;
      }
    }
  }

  return 0;
}

static int place_rooms(dungeon_t *d)
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint32_t x, y;
  struct timeval tv, start;

  /* Placing rooms is 2D bin packing.  Bin packing (2D or otherwise) is NP-  *
   * Complete, meaning (among other things) that there is no known algorithm *
   * to solve the problem in less than exponential time.  There are          *
   * hacks and approximation algorithms that can function more efficiently,  *
   * but we're going to forgoe those in favor of using a timeout.  If we     *
   * can't place all of our rooms in 1 second (and some change), we'll abort *
   * this attempt and start over.                                            */
  gettimeofday(&start, NULL);

  memcpy(&hardness, &d->hardness, sizeof (hardness));

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
      r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
      for (p[dim_y] = r->position[dim_y] - 1;
           success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
           p[dim_y]++) {
        for (p[dim_x] = r->position[dim_x] - 1;
             success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
             p[dim_x]++) {
          if (mappair(p) >= ter_floor) {
            gettimeofday(&tv, NULL);
            if ((tv.tv_sec - start.tv_sec) > 1) {
              memcpy(&d->hardness, &hardness, sizeof (hardness));
              return 1;
            }
            success = 0;
            /* empty_dungeon() regenerates the hardness map, which   *
             * is prohibitively expensive to do in a loop like this, *
             * so instead, we'll use a copy.                         */
            memcpy(&d->hardness, &hardness, sizeof (hardness));
            for (y = 1; y < DUNGEON_Y - 1; y++) {
              for (x = 1; x < DUNGEON_X - 1; x++) {
                mapxy(x, y) = ter_wall;
              }
            }
          } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                     (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                     (p[dim_x] != r->position[dim_x] - 1)              &&
                     (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
            mappair(p) = ter_floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }

  return 0;
}

static int make_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++)
    ;
  d->num_rooms = i;
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size[dim_x] = ROOM_MIN_X;
    d->rooms[i].size[dim_y] = ROOM_MIN_Y;
    while (rand_under(3, 4) && d->rooms[i].size[dim_x] < ROOM_MAX_X) {
      d->rooms[i].size[dim_x]++;
    }
    while (rand_under(3, 4) && d->rooms[i].size[dim_y] < ROOM_MAX_Y) {
      d->rooms[i].size[dim_y]++;
    }
  }

  return 0;
}

int gen_dungeon(dungeon_t *d)
{
  do {
    make_rooms(d);
  } while (place_rooms(d));
  connect_rooms(d);

  return 0;
}

void render_dungeon(dungeon_t *d)
{
  int i,MONSTER_PLACED;
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
    for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
      MONSTER_PLACED = 0;
      for(i=0;i<d->num_monsters;i++){
        if(p[dim_x] == d->monsters[i].position[dim_x] &&
          p[dim_y] == d->monsters[i].position[dim_y]){
            MONSTER_PLACED = 1;
            putchar(to_printable_character(d->monsters[i].characteristics));
          }
      }
      if(MONSTER_PLACED==0){
        if (p[dim_x] ==  d->pc.position[dim_x] &&
            p[dim_y] ==  d->pc.position[dim_y]) {
          putchar('@');
        } else {
          switch (mappair(p)) {
          case ter_wall:
          case ter_wall_immutable:
            putchar(' ');
            break;
          case ter_floor:
          case ter_floor_room:
            putchar('.');
            break;
          case ter_floor_hall:
            putchar('#');
            break;
          case ter_debug:
            putchar('*');
            fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
            break;
          }
        }
      }
    }
    putchar('\n');
  }
}

void delete_dungeon(dungeon_t *d)
{
  free(d->rooms);
  free(d->monsters);
  free(d->events);
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);
}

int write_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fwrite(&d->hardness[y][x], sizeof (unsigned char), 1, f);
    }
  }

  return 0;
}

int write_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint8_t p;

  for (i = 0; i < d->num_rooms; i++) {
    /* write order is xpos, ypos, width, height */
    p = d->rooms[i].position[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].position[dim_y];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_x];
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size[dim_y];
    fwrite(&p, 1, 1, f);
  }

  return 0;
}

uint32_t calculate_dungeon_size(dungeon_t *d)
{
  return (20 /* The semantic, version, and size */     +
          (DUNGEON_X * DUNGEON_Y) /* The hardnesses */ +
          (d->num_rooms * 4) /* Four bytes per room */);
}

int write_dungeon(dungeon_t *d, char *file)
{
  char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/", home, SAVE_DIR);
    makedirectory(filename);
    strcat(filename, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "w"))) {
      perror(filename);
      free(filename);

      return 1;
    }
    free(filename);
  } else {
    if (!(f = fopen(file, "w"))) {
      perror(file);
      exit(-1);
    }
  }

  /* The semantic, which is 12 bytes, 0-11 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, strlen(DUNGEON_SAVE_SEMANTIC), f);

  /* The version, 4 bytes, 12-15 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 16-19 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The dungeon map, 16800 bytes, 20-16819 */
  write_dungeon_map(d, f);

  /* And the rooms, num_rooms * 4 bytes, 16820-end */
  write_rooms(d, f);

  fclose(f);

  return 0;
}

int read_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fread(&d->hardness[y][x], sizeof (d->hardness[y][x]), 1, f);
      if (d->hardness[y][x] == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        d->map[y][x] = ter_floor_hall;
      } else if (d->hardness[y][x] == 255) {
        d->map[y][x] = ter_wall_immutable;
      } else {
        d->map[y][x] = ter_wall;
      }
    }
  }


  return 0;
}

int read_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint32_t x, y;
  uint8_t p;

  for (i = 0; i < d->num_rooms; i++) {
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].position[dim_y] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_x] = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size[dim_y] = p;
    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = d->rooms[i].position[dim_y];
         y < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y];
         y++) {
      for (x = d->rooms[i].position[dim_x];
           x < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x];
           x++) {
        mapxy(x, y) = ter_floor_room;
      }
    }
  }

  return 0;
}

int calculate_num_rooms(uint32_t dungeon_bytes)
{
  return ((dungeon_bytes -
          (20 /* The semantic, version, and size */       +
           (DUNGEON_X * DUNGEON_Y) /* The hardnesses */)) /
          4 /* Four bytes per room */);
}

int read_dungeon(dungeon_t *d, char *file)
{
  char semantic[13];
  uint32_t be32;
  FILE *f;
  char *home;
  size_t len;
  char *filename;
  struct stat buf;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "r"))) {
      perror(filename);
      free(filename);
      exit(-1);
    }

    if (stat(filename, &buf)) {
      perror(filename);
      exit(-1);
    }

    free(filename);
  } else {
    if (!(f = fopen(file, "r"))) {
      perror(file);
      exit(-1);
    }
    if (stat(file, &buf)) {
      perror(file);
      exit(-1);
    }
  }

  d->num_rooms = 0;

  fread(semantic, sizeof(semantic) - 1, 1, f);
  semantic[12] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC, 12)) {
    fprintf(stderr, "Not an RLG327 save file.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    fprintf(stderr, "File version mismatch.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (buf.st_size != be32toh(be32)) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }
  read_dungeon_map(d, f);
  d->num_rooms = calculate_num_rooms(buf.st_size);
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);
  read_rooms(d, f);

  fclose(f);

  return 0;
}

int read_pgm(dungeon_t *d, char *pgm)
{
  FILE *f;
  char s[80];
  uint8_t gm[103][158];
  uint32_t x, y;
  uint32_t i;

  if (!(f = fopen(pgm, "r"))) {
    perror(pgm);
    exit(-1);
  }

  if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
    fprintf(stderr, "Expected P5\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || s[0] != '#') {
    fprintf(stderr, "Expected comment\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "158 103", 5)) {
    fprintf(stderr, "Expected 158 103\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "255", 2)) {
    fprintf(stderr, "Expected 255\n");
    exit(-1);
  }

  fread(gm, 1, 158 * 103, f);

  fclose(f);

  /* In our gray map, treat black (0) as corridor, white (255) as room, *
   * all other values as a hardness.  For simplicity, treat every white *
   * cell as its own room, so we have to count white after reading the  *
   * image in order to allocate the room array.                         */
  for (d->num_rooms = 0, y = 0; y < 103; y++) {
    for (x = 0; x < 158; x++) {
      if (!gm[y][x]) {
        d->num_rooms++;
      }
    }
  }
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0, y = 0; y < 103; y++) {
    for (x = 0; x < 158; x++) {
      if (!gm[y][x]) {
        d->rooms[i].position[dim_x] = x + 1;
        d->rooms[i].position[dim_y] = y + 1;
        d->rooms[i].size[dim_x] = 1;
        d->rooms[i].size[dim_y] = 1;
        i++;
        d->map[y + 1][x + 1] = ter_floor_room;
        d->hardness[y + 1][x + 1] = 0;
      } else if (gm[y][x] == 255) {
        d->map[y + 1][x + 1] = ter_floor_hall;
        d->hardness[y + 1][x + 1] = 0;
      } else {
        d->map[y + 1][x + 1] = ter_wall;
        d->hardness[y + 1][x + 1] = gm[y][x];
      }
    }
  }

  for (x = 0; x < 160; x++) {
    d->map[0][x] = ter_wall_immutable;
    d->hardness[0][x] = 255;
    d->map[104][x] = ter_wall_immutable;
    d->hardness[104][x] = 255;
  }
  for (y = 1; y < 104; y++) {
    d->map[y][0] = ter_wall_immutable;
    d->hardness[y][0] = 255;
    d->map[y][159] = ter_wall_immutable;
    d->hardness[y][159] = 255;
  }

  return 0;
}

void render_distance_map(dungeon_t *d)
{
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
    for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
      if (p[dim_x] ==  d->pc.position[dim_x] &&
          p[dim_y] ==  d->pc.position[dim_y]) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case ter_wall:
        case ter_wall_immutable:
          putchar(' ');
          break;
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
          putchar('0' + d->pc_tunnel[p[dim_y]][p[dim_x]] % 10);
          break;
        case ter_debug:
          fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}

void render_tunnel_distance_map(dungeon_t *d)
{
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
    for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
      if (p[dim_x] ==  d->pc.position[dim_x] &&
          p[dim_y] ==  d->pc.position[dim_y]) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case ter_wall_immutable:
          putchar(' ');
          break;
        case ter_wall:
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
          putchar('0' + d->pc_tunnel[p[dim_y]][p[dim_x]] % 10);
          break;
        case ter_debug:
          fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}
