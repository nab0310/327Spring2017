#include <stdlib.h>
#include <ncurses.h>

#include "string.h"

#include "dungeon.h"
#include "npc.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void pc_delete(pc_t *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
  d->pc.position[dim_y] = rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1));
  d->pc.position[dim_x] = rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1));
}

void config_pc(dungeon_t *d)
{
  memset(&d->pc, 0, sizeof (d->pc));
  d->pc.symbol = '@';

  place_pc(d);

  d->pc.speed = PC_SPEED;
  d->pc.alive = 1;
  d->pc.sequence_number = 0;
  d->pc.pc = calloc(1, sizeof (*d->pc.pc));
  d->pc.npc = NULL;
  d->pc.kills[kill_direct] = d->pc.kills[kill_avenged] = 0;

  d->character[d->pc.position[dim_y]][d->pc.position[dim_x]] = &d->pc;

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  int inLookMode = 0;
  int keyPress = 0;

  dir[dim_y] = dir[dim_x] = 0;

  /* First, eat anybody standing next to us. */
  if (charxy(d->pc.position[dim_x] - 1, d->pc.position[dim_y] - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = -1;
  } else if (charxy(d->pc.position[dim_x], d->pc.position[dim_y] - 1)) {
    dir[dim_y] = -1;
  } else if (charxy(d->pc.position[dim_x] + 1, d->pc.position[dim_y] - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = 1;
  } else if (charxy(d->pc.position[dim_x] - 1, d->pc.position[dim_y])) {
    dir[dim_x] = -1;
  } else if (charxy(d->pc.position[dim_x] + 1, d->pc.position[dim_y])) {
    dir[dim_x] = 1;
  } else if (charxy(d->pc.position[dim_x] - 1, d->pc.position[dim_y] + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = -1;
  } else if (charxy(d->pc.position[dim_x], d->pc.position[dim_y] + 1)) {
    dir[dim_y] = 1;
  } else if (charxy(d->pc.position[dim_x] + 1, d->pc.position[dim_y] + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = 1;
   }

   while(!keyPress){
     do {
     int userInput = getch();
    switch(userInput) {
        case 'y':
        case '7':
          keyPress = 1;
          //upper left
          dir[dim_x] = -1;
          dir[dim_y] = -1;
          break;
        case 'k':
        case '8':
        keyPress = 1;
          // code for arrow up
          if(inLookMode){
            //Move Window Up
            d->renderStart[dim_x] = d->renderStart[dim_x];
            d->renderStart[dim_y] = d->renderStart[dim_y] - 10;
            d->renderEnd[dim_x] = d->renderEnd[dim_x];
            d->renderEnd[dim_y] = d->renderEnd[dim_y] - 10;
            clear();
            render_dungeon(d);
            refresh();
          }else{
            dir[dim_x] = 0;
            dir[dim_y] = -1;
          }
          break;
        case 'u':
        case '9':
        keyPress = 1;
          //upper right
          dir[dim_x] = 1;
          dir[dim_y] = -1;
          break;
        case 'n':
        case '3':
        keyPress = 1;
          //lower right
          dir[dim_x] = 1;
          dir[dim_y] = 1;
          break;
        case 'j':
        case '2':
        keyPress = 1;
          // code for arrow down
          if(inLookMode){
            //Move window Down
            d->renderStart[dim_x] = d->renderStart[dim_x];
            d->renderStart[dim_y] = d->renderStart[dim_y] + 10;
            d->renderEnd[dim_x] = d->renderEnd[dim_x];
            d->renderEnd[dim_y] = d->renderEnd[dim_y] + 10;
            clear();
            render_dungeon(d);
            refresh();
          }else{
            dir[dim_x] = 0;
            dir[dim_y] = 1;
          }
          break;
        case '6':
        case 'l':
        keyPress = 1;
          // code for arrow right
          if(inLookMode){
            //Move window right
            d->renderStart[dim_x] = d->renderStart[dim_x]+30;
            d->renderStart[dim_y] = d->renderStart[dim_y];
            d->renderEnd[dim_x] = d->renderEnd[dim_x]+30;
            d->renderEnd[dim_y] = d->renderEnd[dim_y];
            clear();
            render_dungeon(d);
            refresh();
          }else{
            dir[dim_x] = 1;
            dir[dim_y] = 0;
          }
          break;
        case '1':
        case 'b':
        keyPress = 1;
          //lower left
          dir[dim_x] = -1;
          dir[dim_y] = 1;
          break;
        case '4':
        case 'h':
        keyPress = 1;
          // code for arrow left
          if(inLookMode){
            //Move window left
            d->renderStart[dim_x] = d->renderStart[dim_x]-30;
            d->renderStart[dim_y] = d->renderStart[dim_y];
            d->renderEnd[dim_x] = d->renderEnd[dim_x]-30;
            d->renderEnd[dim_y] = d->renderEnd[dim_y];
            clear();
            render_dungeon(d);
            refresh();
          }else{
            dir[dim_x] = -1;
            dir[dim_y] = 0;
          }
          break;
        case '5':
        case ' ':
        keyPress = 1;
          break;
        case '<':
        keyPress = 1;
          if(mappair(d->pc.position) == ter_stairs_up){
            clear();
            delete_dungeon(d);
            pc_delete(d->pc.pc);
            init_dungeon(d);
            gen_dungeon(d);
            config_pc(d);
            place_stairs(d);
            gen_monsters(d);
          }
          break;
        case '>':
        keyPress = 1;
          if(mappair(d->pc.position) == ter_stairs_down){
            clear();
            delete_dungeon(d);
            pc_delete(d->pc.pc);
            init_dungeon(d);
            gen_dungeon(d);
            config_pc(d);
            place_stairs(d);
            gen_monsters(d);
          }
          break;
        case 'Q':
        keyPress = 1;
        delete_dungeon(d);
        clear();
        endwin();
        exit(1);
          break;
        case 'L':
          keyPress = 1;
          inLookMode = 1;
          break;
        case 27:
          inLookMode = 0;
          break;
    }
  }while(inLookMode);
}

  return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms)                                     &&
      (d->pc.position[dim_x] >= d->rooms[room].position[dim_x]) &&
      (d->pc.position[dim_x] < (d->rooms[room].position[dim_x] +
                                d->rooms[room].size[dim_x]))    &&
      (d->pc.position[dim_y] >= d->rooms[room].position[dim_y]) &&
      (d->pc.position[dim_y] < (d->rooms[room].position[dim_y] +
                                d->rooms[room].size[dim_y]))) {
    return 1;
  }

  return 0;
}
