#ifndef DUNGEON_H
# define DUNGEON_H

# include "heap.h"
# include "macros.h"
# include "dims.h"

#define DUNGEON_X              160
#define DUNGEON_Y              105
#define MIN_ROOMS              25
#define MAX_ROOMS              40
#define ROOM_MIN_X             7
#define ROOM_MIN_Y             5
#define ROOM_MAX_X             20
#define ROOM_MAX_Y             15
#define SAVE_DIR               ".rlg327"
#define DUNGEON_SAVE_FILE      "dungeon"
#define DUNGEON_SAVE_SEMANTIC  "RLG327-S2017"
#define DUNGEON_SAVE_VERSION   0U
#define NPC_SMART 0x00000001 //0001
#define NPC_TELE 0x00000002 //0010
#define NPC_TUNNEL 0x00000004 //0100
#define NPC_ERRATIC 0x00000008 //1000

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])
#define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

typedef struct pc {
  pair_t position;
  uint32_t speed;
  uint32_t nTicks;
  int alive;
} pc_t;

typedef struct monster{
  char characteristics;
  pair_t destination;
  pair_t position;
  uint32_t speed;
  uint32_t nTicks;
  int alive;
}monster_t;

typedef enum __attribute__ ((__packed__)) event_type{
  monster,
  player,
}event_type_t;

typedef struct events{
  uint32_t time;
  uint32_t seqNum;
  event_type_t type;
  union{
    pc_t *pc;
    monster_t *monster;
  }p;
}events_t;

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
} terrain_type_t;

typedef struct room {
  pair_t position;
  pair_t size;
} room_t;

typedef struct dungeon {
  uint32_t num_rooms;
  uint32_t num_monsters;
  room_t *rooms;
  monster_t *monsters;
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  /* Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performanace  *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.                                    */
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_distance[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_tunnel[DUNGEON_Y][DUNGEON_X];
  pc_t pc;
  events_t *events;
  heap_t heap;
} dungeon_t;

void init_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
int gen_dungeon(dungeon_t *d);
void render_dungeon(dungeon_t *d);
int write_dungeon(dungeon_t *d, char *file);
int read_dungeon(dungeon_t *d, char *file);
int read_pgm(dungeon_t *d, char *pgm);
void render_distance_map(dungeon_t *d);
void render_tunnel_distance_map(dungeon_t *d);
void gen_monsters(dungeon_t *d);
void update(dungeon_t *d);
void init_characters(dungeon_t *d);
void make_heap(dungeon_t *d);

#endif
