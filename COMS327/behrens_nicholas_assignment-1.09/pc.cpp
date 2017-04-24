#include <stdlib.h>
#include <ncurses.h>
#include <string>

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"
#include "io.h"
#include "object.h"

const char *eq_slot_name[num_eq_slots] = {
  "weapon",
  "offhand",
  "ranged",
  "light",
  "armor",
  "helmet",
  "cloak",
  "gloves",
  "boots",
  "amulet",
  "lring",
  "rring"
};

pc::pc(){
  uint32_t i;

  for(i=0; i< PC_EQUIPMENT_SLOTS;i++){
    equipment_slot[i] = 0;
  }

  for(i=0; i< PC_CARRY_SLOTS;i++){
    carry_slot[i] = 0;
  }

}

pc::~pc(){
  uint32_t i;

  for(i=0; i< PC_EQUIPMENT_SLOTS;i++){
    delete equipment_slot[i];
    equipment_slot[i] = NULL;
  }

  for(i=0; i< PC_CARRY_SLOTS;i++){
    delete carry_slot[i];
    carry_slot[i] = NULL;
  }

}

void pc_delete(pc *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->PC->alive;
}

void place_pc(dungeon_t *d)
{
  character_set_y(d->PC, rand_range(d->rooms->position[dim_y],
                                   (d->rooms->position[dim_y] +
                                    d->rooms->size[dim_y] - 1)));
  character_set_x(d->PC, rand_range(d->rooms->position[dim_x],
                                   (d->rooms->position[dim_x] +
                                    d->rooms->size[dim_x] - 1)));
  io_calculate_offset(d);
  io_update_offset(d);

  pc_init_known_terrain(d->PC);
  pc_observe_terrain(d->PC,d);
  io_display(d);
}

void config_pc(dungeon_t *d)
{
  static dice pc_dice(0, 1, 4);

  d->PC = new pc();

  memset(d->PC, 0, sizeof (*d->PC));
  d->PC->symbol = '@';

  place_pc(d);

  d->PC->speed = PC_SPEED;
  d->PC->alive = 1;
  d->PC->sequence_number = 0;
  d->PC->kills[kill_direct] = d->PC->kills[kill_avenged] = 0;
  d->PC->color.push_back(COLOR_WHITE);
  d->PC->damage = &pc_dice;
  d->PC->hp = PC_HP;
  d->PC->name = "Isabella Garcia-Shapiro";

  d->character_map[character_get_y(d->PC)][character_get_x(d->PC)] = d->PC;

  dijkstra(d);
  dijkstra_tunnel(d);

  io_calculate_offset(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  static uint32_t have_seen_corner = 0;
  static uint32_t count = 0;

  dir[dim_y] = dir[dim_x] = 0;

  if (in_corner(d, d->PC)) {
    if (!count) {
      count = 1;
    }
    have_seen_corner = 1;
  }

  /* First, eat anybody standing next to us. */
  if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC) - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->PC), character_get_y(d->PC) - 1)) {
    dir[dim_y] = -1;
  } else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC) - 1)) {
    dir[dim_y] = -1;
    dir[dim_x] = 1;
  } else if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC))) {
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC))) {
    dir[dim_x] = 1;
  } else if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC) + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = -1;
  } else if (charxy(character_get_x(d->PC), character_get_y(d->PC) + 1)) {
    dir[dim_y] = 1;
  } else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC) + 1)) {
    dir[dim_y] = 1;
    dir[dim_x] = 1;
  } else if (!have_seen_corner || count < 250) {
    /* Head to a corner and let most of the NPCs kill each other off */
    if (count) {
      count++;
    }
    if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111)) {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    } else {
      dir_nearest_wall(d, d->PC, dir);
    }
  }else {
    /* And after we've been there, let's head toward the center of the map. */
    if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111)) {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    } else {
      dir[dim_x] = ((character_get_x(d->PC) > DUNGEON_X / 2) ? -1 : 1);
      dir[dim_y] = ((character_get_y(d->PC) > DUNGEON_Y / 2) ? -1 : 1);
    }
  }

  /* Don't move to an unoccupied location if that places us next to a monster */
  if (!charxy(character_get_x(d->PC) + dir[dim_x],
              character_get_y(d->PC) + dir[dim_y]) &&
      ((charxy(character_get_x(d->PC) + dir[dim_x] - 1,
               character_get_y(d->PC) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] - 1,
                character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x] - 1,
               character_get_y(d->PC) + dir[dim_y]) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] - 1,
                character_get_y(d->PC) + dir[dim_y]) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x] - 1,
               character_get_y(d->PC) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] - 1,
                character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x],
               character_get_y(d->PC) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x],
                character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x],
               character_get_y(d->PC) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x],
                character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
               character_get_y(d->PC) + dir[dim_y] - 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
                character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
               character_get_y(d->PC) + dir[dim_y]) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
                character_get_y(d->PC) + dir[dim_y]) != d->PC)) ||
       (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
               character_get_y(d->PC) + dir[dim_y] + 1) &&
        (charxy(character_get_x(d->PC) + dir[dim_x] + 1,
                character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)))) {
    dir[dim_x] = dir[dim_y] = 0;
  }

  return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms)                                     &&
      (character_get_x(d->PC) >= d->rooms[room].position[dim_x]) &&
      (character_get_x(d->PC) < (d->rooms[room].position[dim_x] +
                                d->rooms[room].size[dim_x]))    &&
      (character_get_y(d->PC) >= d->rooms[room].position[dim_y]) &&
      (character_get_y(d->PC) < (d->rooms[room].position[dim_y] +
                                d->rooms[room].size[dim_y]))) {
    return 1;
  }

  return 0;
}

void pc_learn_terrain(pc *p, pair_t pos, terrain_type_t ter)
{
  p->known_terrain[pos[dim_y]][pos[dim_x]] = ter;
  p->visible[pos[dim_y]][pos[dim_x]] = 1;
}

void pc_reset_visibility(pc *p)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      p->visible[y][x] = 0;
    }
  }
}

terrain_type_t pc_learned_terrain(pc *p, int16_t y, int16_t x)
{
  if (y < 0 || y >= DUNGEON_Y || x < 0 || x >= DUNGEON_X) {
    io_queue_message("Invalid value to %s: %d, %d", __FUNCTION__, y, x);
  }

  return p->known_terrain[y][x];
}

void pc_init_known_terrain(pc *p)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      p->known_terrain[y][x] = ter_unknown;
      p->visible[y][x] = 0;
    }
  }
}

void pc_observe_terrain(pc *p, dungeon_t *d)
{
  pair_t where;
  int16_t y_min, y_max, x_min, x_max;

  y_min = p->position[dim_y] - PC_VISUAL_RANGE;
  if (y_min < 0) {
    y_min = 0;
  }
  y_max = p->position[dim_y] + PC_VISUAL_RANGE;
  if (y_max > DUNGEON_Y - 1) {
    y_max = DUNGEON_Y - 1;
  }
  x_min = p->position[dim_x] - PC_VISUAL_RANGE;
  if (x_min < 0) {
    x_min = 0;
  }
  x_max = p->position[dim_x] + PC_VISUAL_RANGE;
  if (x_max > DUNGEON_X - 1) {
    x_max = DUNGEON_X - 1;
  }

  for (where[dim_y] = y_min; where[dim_y] <= y_max; where[dim_y]++) {
    where[dim_x] = x_min;
    can_see(d, p->position, where, 1, 1);
    where[dim_x] = x_max;
    can_see(d, p->position, where, 1, 1);
  }
  /* Take one off the x range because we alreay hit the corners above. */
  for (where[dim_x] = x_min - 1; where[dim_x] <= x_max - 1; where[dim_x]++) {
    where[dim_y] = y_min;
    can_see(d, p->position, where, 1, 1);
    where[dim_y] = y_max;
    can_see(d, p->position, where, 1, 1);
  }
}

int32_t is_illuminated(pc *p, int16_t y, int16_t x)
{
  return p->visible[y][x];
}

void pc_see_object(character *the_pc, object *o)
{
  if (o) {
    o->has_been_seen();
  }
}
void pc::recaluclateSpeed(){
  int i;
  pc::speed = PC_SPEED;
  for(i=0;i<PC_EQUIPMENT_SLOTS;i++){
    if(equipment_slot[i]){
      pc::speed= pc::speed + equipment_slot[i]->get_speed();
    }
  }
  if(pc::speed <=0){
    pc::speed = 1;
  }
}
uint32_t pc::has_open_carry_slot(){
  int i;

  for(i=0;i<PC_CARRY_SLOTS;i++){
    if(!carry_slot[i]){
      return 1;
    }
  }
  return 0;
}
uint32_t pc::get_first_open_carry_slot(){
  int i;

  for(i=0;i<PC_CARRY_SLOTS;i++){
    if(!carry_slot[i]){
      return i;
    }
  }
  return -1;
}
uint32_t pc::drop_carry(dungeon_t *d,uint32_t slot){
  if(!carry_slot[slot]){
    return 1;
  }
  io_queue_message("You dropped %s",carry_slot[slot]->get_name());

  carry_slot[slot]->to_pile(d, d->PC->position);
  carry_slot[slot] = NULL;

  return 0;
}
uint32_t pc::wear_carry(uint32_t slot){
  object *tmp;
  int i;
  if(!carry_slot[slot]){
    return 1;
  }
  i = carry_slot[slot]->get_slot_index();

  if(equipment_slot[i] &&
      equipment_slot[i]->get_type() == objtype_RING &&
      !equipment_slot[i+1]){
        i++;
      }

  tmp = carry_slot[slot];
  carry_slot[slot] = equipment_slot[i];
  equipment_slot[i] = tmp;

  io_queue_message("You wear %s.",equipment_slot[i]->get_name());
  recaluclateSpeed();

  return 0;
}
uint32_t pc::destroy_carry(uint32_t slot){
  if(!carry_slot[slot]){
    io_queue_message("Cannot destory something that doesn't exist.");
    return 1;
  }
  io_queue_message("You destory %s",carry_slot[slot]->get_name());

  delete carry_slot[slot];
  carry_slot[slot] = NULL;

  return 0;
}
uint32_t pc::remove_equipment(uint32_t slot){
  if(!equipment_slot[slot] ||
      !has_open_carry_slot()){
        io_queue_message("You cannot remove %s, because you have nowhere to put it.", equipment_slot[slot]);
        return 1;
      }
  io_queue_message("You remove %s",equipment_slot[slot]->get_name());

  carry_slot[get_first_open_carry_slot()] = equipment_slot[slot];
  equipment_slot[slot] = NULL;

  recaluclateSpeed();

  return 0;
}
uint32_t pc::pick_up_object(dungeon_t *d){
  object* o;

  while(has_open_carry_slot() &&
        d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]]){
          io_queue_message("You have picked up %s",d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]]->get_name());
          carry_slot[get_first_open_carry_slot()] = from_pile(d, d->PC->position);
        }

  for(o=d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]];o;o= o->get_next()){
    io_queue_message("You have no room for %s",o->get_name());
  }
  return 0;
}
object * pc::from_pile(dungeon_t *d, int16_t *position){
  object *o;

  if((o = (object *)d->objmap[position[dim_y]][position[dim_x]])){
    d->objmap[position[dim_y]][position[dim_x]] = o->get_next();
    o->set_next(0);
  }
  return o;
}
