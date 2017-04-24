#include <stdlib.h>

# ifdef __cplusplus
extern "C" {
#endif
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "heap.h"
#include "dungeon.h"
# ifdef __cplusplus
}
#endif


void character_delete(character_t *c)
{
    delete c;
}

int16_t *character_get_pos(character_t *c)
{
  return c->position;
}
int8_t character_get_y(const character_t *c)
{
  return c->position[dim_y];
}
int8_t character_get_x(const character_t *c)
{
  return c->position[dim_x];
}
int8_t character_set_y(character_t *c, int8_t y)
{
  return c->position[dim_y] = y;
}
int8_t character_set_x(character_t *c, int8_t x)
{
  return c->position[dim_x] = x;
}

void character_die(character_t *c) {
    c->alive = 0;
}

uint32_t is_character_alive(const character_t *c) {
    return c->alive;
}
void character_set_alive(character_t *c, uint32_t a){
  c->alive = a;
}

char character_get_symbol(const character_t *c) {
    return c->symbol;
}
void character_set_symbol(character_t *c, char symbol){
    c->symbol = symbol;
}
void character_set_symbol_pointer(character_t *c, const char *symbol)
{
  c->symbol = *symbol;
}

uint32_t character_get_speed(const character_t *c) {
    return c->speed;
}
void character_set_speed(character_t *c, int32_t speed){
  c->speed = speed;
}
void character_set_kills(character_t *c, uint16_t type, int32_t number)
{
  c->kills[type] = number;
}
uint32_t character_get_kills(const character_t *c, uint16_t type)
{
  return c->kills[type];
}
void character_set_sequenceNumber(character_t *c, uint32_t cn)
{
  c->sequence_number = cn;
}
uint32_t character_get_sequenceNumber(const character_t *c)
{
  return c->sequence_number;
}

uint32_t can_see(dungeon_t *d, character_t *voyeur, character_t *exhibitionist)
{
  /* Application of Bresenham's Line Drawing Algorithm.  If we can draw *
   * a line from v to e without intersecting any walls, then v can see  *
   * e.  Unfortunately, Bresenham isn't symmetric, so line-of-sight     *
   * based on this approach is not reciprocal (Helmholtz Reciprocity).  *
   * This is a very real problem in roguelike games, and one we're      *
   * going to ignore for now.  Algorithms that are symmetrical are far  *
   * more expensive.                                                    */

  pair_t first, second;
  pair_t del, f;
  int16_t a, b, c, i;

  first[dim_x] = voyeur->position[dim_x];
  first[dim_y] = voyeur->position[dim_y];
  second[dim_x] = exhibitionist->position[dim_x];
  second[dim_y] = exhibitionist->position[dim_y];

  if ((abs(first[dim_x] - second[dim_x]) > VISUAL_RANGE) ||
      (abs(first[dim_y] - second[dim_y]) > VISUAL_RANGE)) {
    return 0;
  }

  /*
  mappair(first) = ter_debug;
  mappair(second) = ter_debug;
  */

  if (second[dim_x] > first[dim_x]) {
    del[dim_x] = second[dim_x] - first[dim_x];
    f[dim_x] = 1;
  } else {
    del[dim_x] = first[dim_x] - second[dim_x];
    f[dim_x] = -1;
  }

  if (second[dim_y] > first[dim_y]) {
    del[dim_y] = second[dim_y] - first[dim_y];
    f[dim_y] = 1;
  } else {
    del[dim_y] = first[dim_y] - second[dim_y];
    f[dim_y] = -1;
  }

  if (del[dim_x] > del[dim_y]) {
    a = del[dim_y] + del[dim_y];
    c = a - del[dim_x];
    b = c - del[dim_x];
    for (i = 0; i <= del[dim_x]; i++) {
      if ((mappair(first) < ter_floor) && i && (i != del[dim_x])) {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_x] += f[dim_x];
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first[dim_y] += f[dim_y];
      }
    }
    return 1;
  } else {
    a = del[dim_x] + del[dim_x];
    c = a - del[dim_y];
    b = c - del[dim_y];
    for (i = 0; i <= del[dim_y]; i++) {
      if ((mappair(first) < ter_floor) && i && (i != del[dim_y])) {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_y] += f[dim_y];
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first[dim_x] += f[dim_x];
      }
    }
    return 1;
  }

  return 1;
}
