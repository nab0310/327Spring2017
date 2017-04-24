#include <string>

#include "dungeon_objects.h"
#include "dungeon.h"
#include "utils.h"
# include "dims.h"

char object_get_symbol(dungeon_object *d_o){
  return d_o->symbol;
}

uint32_t object_get_color(dungeon_object *d_o){
  return d_o->color;
}

void gen_objects(dungeon_t *d){
  dungeon_object *d_o;
  uint32_t room;
  pair_t p;

  std::vector<dungeon_object*>::iterator oi;

  std::vector<dungeon_object*> &obj = d->dungeon_objects;

  for (oi = obj.begin(); oi != obj.end(); oi++) {
    d_o = new dungeon_object;
    memset(d_o, 0, sizeof (*d_o));

    room = rand_range(1, d->num_rooms - 1);
    p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                          (d->rooms[room].position[dim_y] +
                           d->rooms[room].size[dim_y] - 1));
    p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                          (d->rooms[room].position[dim_x] +
                           d->rooms[room].size[dim_x] - 1));
    d_o->position[dim_y] = p[dim_y];
    d_o->position[dim_x] = p[dim_x];
    d->object_map[p[dim_y]][p[dim_x]] = d_o;
    d_o->symbol = object_get_symbol(*oi);
    d_o->set_color(object_get_color(*oi));
    d_o->have_seen_pc = 0;
    d->object_map[p[dim_y]][p[dim_x]] = d_o;

    std::cout << "object position is: " << p[dim_y] << ","<< p[dim_x] << std::endl;
    std::cout << "object symbol is: " << d->object_map[p[dim_y]][p[dim_x]]->symbol << std::endl;
  }
}

int16_t *object_get_postition(dungeon_object *d_o){
  return d_o->position;
}
