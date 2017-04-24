#ifndef DUNGEON_H
# define DUNGEON_H

# include "heap.h"
# include "macros.h"
# include "dims.h"
# include "character.h"

#define DUNGEON_X              160
#define DUNGEON_Y              105
#define MIN_ROOMS              25
#define MAX_ROOMS              40
#define ROOM_MIN_X             7
#define ROOM_MIN_Y             5
#define ROOM_MAX_X             20
#define ROOM_MAX_Y             15
#define VISUAL_RANGE           20
#define PC_SPEED               10
#define MAX_MONSTERS           25
#define SAVE_DIR               ".rlg327"
#define DUNGEON_SAVE_FILE      "dungeon"
#define DUNGEON_SAVE_SEMANTIC  "RLG327-S2017"
#define DUNGEON_SAVE_VERSION   0U

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])
#define charpair(pair) (d->character[pair[dim_y]][pair[dim_x]])
#define charxy(x, y) (d->character[y][x])

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_stairs,
  ter_stairs_up,
  ter_stairs_down
} terrain_type_t;

typedef struct room {
  pair_t position;
  pair_t size;
} room_t;

#ifdef __cplusplus

class pc_t;

#else
typedef void pc_t;
#endif

typedef struct dungeon {
  uint32_t num_rooms;
  room_t *rooms;
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_distance[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_tunnel[DUNGEON_Y][DUNGEON_X];
  character_t *character[DUNGEON_Y][DUNGEON_X];
  pc_t *pc;
  heap_t events;
  uint16_t num_monsters;
  uint16_t max_monsters;
  uint32_t character_sequence_number;
  uint32_t time;
  uint32_t quit;
  pair_t io_offset;
} dungeon_t;

# ifdef __cplusplus
extern "C" {
#endif

void init_dungeon(dungeon_t *d);
void new_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
int gen_dungeon(dungeon_t *d);
void render_dungeon(dungeon_t *d);
int write_dungeon(dungeon_t *d, char *file);
int read_dungeon(dungeon_t *d, char *file);
int read_pgm(dungeon_t *d, char *pgm);
void render_distance_map(dungeon_t *d);
void render_tunnel_distance_map(dungeon_t *d);

# ifdef __cplusplus
}
#endif

#endif
