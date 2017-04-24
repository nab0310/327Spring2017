# ifdef __cplusplus
extern "C" {
#endif
#include "utils.h"
#include "move.h"
#include "path.h"
#include "io.h"
#include "pc.h"

# ifdef __cplusplus
}
#endif
pc_t::pc_t() {}

pc_t::~pc_t()
{

}

void pc_delete(pc_t *p){
  delete p;
}
uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc->alive;
}
void config_pc(dungeon_t *d)
{
  memset(d->pc, 0, sizeof (&d->pc));
  character_set_symbol(d->pc, '@');

  place_pc(d);

  character_set_speed(d->pc, PC_SPEED);
  character_set_alive(d->pc, 1);
  character_set_sequenceNumber(d->pc, 0);
  d->pc = new pc_t();
  character_set_kills(d->pc, kill_direct, 0);
  character_set_kills(d->pc, kill_avenged, 0);

  d->character[character_get_y(d->pc)][character_get_x(d->pc)] = d->pc;

  dijkstra(d);
  dijkstra_tunnel(d);

  io_calculate_offset(d);
}
uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  static uint32_t have_seen_corner = 0;
  static uint32_t count = 0;

  dir[dim_y] = dir[dim_x] = 0;

  if (in_corner(d, d->pc)) {
    if (!count) {
      count = 1;
    }
    have_seen_corner = 1;
  }

  /* First, eat anybody standing next to us. */
  if (charxy(character_get_x(d->pc) - 1, character_get_y(d->pc) - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->pc), character_get_y(d->pc) - 1)) {
    dir[dim_y] = -1;
  } else if (charxy(character_get_x(d->pc) + 1, character_get_y(d->pc) - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = 1;
  } else if (charxy(character_get_x(d->pc) - 1, character_get_y(d->pc))) {
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->pc) + 1, character_get_y(d->pc))) {
    dir[dim_x] = 1;
  } else if (charxy(character_get_x(d->pc) - 1, character_get_y(d->pc) + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->pc), character_get_y(d->pc) + 1)) {
    dir[dim_y] = 1;
  } else if (charxy(character_get_x(d->pc) + 1, character_get_y(d->pc) + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = 1;
  } else if (!have_seen_corner || count < 250) {
    /* Head to a corner and let most of the NPCs kill each other off */
    if (count) {
      count++;
    }
    if (!against_wall(d, d->pc) && ((rand() & 0x111) == 0x111)) {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    } else {
      dir_nearest_wall(d, d->pc, dir);
    }
  }else {
    /* And after we've been there, let's head toward the center of the map. */
    if (!against_wall(d, d->pc) && ((rand() & 0x111) == 0x111)) {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    } else {
      dir[dim_x] = ((character_get_x(d->pc) > DUNGEON_X / 2) ? -1 : 1);
      dir[dim_y] = ((character_get_y(d->pc) > DUNGEON_Y / 2) ? -1 : 1);
    }
  }

  /* Don't move to an unoccupied location if that places us next to a monster */
  if (!charxy(character_get_x(d->pc) + dir[dim_x],
              character_get_y(d->pc) + dir[dim_y]) &&
      ((charxy(character_get_x(d->pc) + dir[dim_x] - 1,
               character_get_y(d->pc) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] - 1,
                character_get_y(d->pc) + dir[dim_y] - 1) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x] - 1,
               character_get_y(d->pc) + dir[dim_y]) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] - 1,
                character_get_y(d->pc) + dir[dim_y]) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x] - 1,
               character_get_y(d->pc) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] - 1,
                character_get_y(d->pc) + dir[dim_y] + 1) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x],
               character_get_y(d->pc) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x],
                character_get_y(d->pc) + dir[dim_y] - 1) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x],
               character_get_y(d->pc) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x],
                character_get_y(d->pc) + dir[dim_y] + 1) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
               character_get_y(d->pc) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
                character_get_y(d->pc) + dir[dim_y] - 1) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
               character_get_y(d->pc) + dir[dim_y]) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
                character_get_y(d->pc) + dir[dim_y]) != d->pc)) ||
       (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
               character_get_y(d->pc) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->pc) + dir[dim_x] + 1,
                character_get_y(d->pc) + dir[dim_y] + 1) != d->pc)))) {
    dir[dim_x] = dir[dim_y] = 0;
  }

  return 0;
}
void place_pc(dungeon_t *d)
{
  character_set_y(d->pc, rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1)));
  character_set_x(d->pc, rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1)));
}
uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms)                                     &&
      (character_get_x(d->pc) >= d->rooms[room].position[dim_x]) &&
      (character_get_x(d->pc) < (d->rooms[room].position[dim_x] +
                                d->rooms[room].size[dim_x]))    &&
      (character_get_y(d->pc) >= d->rooms[room].position[dim_y]) &&
      (character_get_y(d->pc) < (d->rooms[room].position[dim_y] +
                                d->rooms[room].size[dim_y]))) {
    return 1;
  }

  return 0;
}
