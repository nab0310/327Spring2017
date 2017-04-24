#ifndef CHARACTER_H
# define CHARACTER_H

# include <stdint.h>

# ifdef __cplusplus
extern "C"{
# endif

# include "dims.h"
#include "heap.h"

# ifdef __cplusplus
}
# endif

typedef enum kill_type {
  kill_direct,
  kill_avenged,
  num_kill_types
} kill_type_t;

#ifdef __cplusplus
class character_t {
public:
  char symbol;
  pair_t position;
  int32_t speed;
  uint32_t alive;
  uint32_t sequence_number;
  uint32_t kills[num_kill_types];
};
extern "C" {
#else
typedef void character_t;
#endif

typedef struct dungeon dungeon_t;

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2);
uint32_t can_see(dungeon_t *d, character_t *voyeur, character_t *exhibitionist);
void character_delete(character_t *c);
int16_t *character_get_pos(character_t *c);
int8_t character_get_y(const character_t *c);
int8_t character_set_y(character_t *c, int8_t y);
int8_t character_get_x(const character_t *c);
int8_t character_set_x(character_t *c, int8_t x);
uint32_t character_get_next_turn(const character_t *c);
void character_die(character_t *c);
uint32_t is_character_alive(const character_t *c);
void character_set_alive(character_t *c, uint32_t a);
void character_next_turn(character_t *c);
void character_reset_turn(character_t *c);
char character_get_symbol(const character_t *c);
void character_set_symbol(character_t *c, char symbol);
void character_set_symbol_pointer(character_t *c, const char *symbol);
uint32_t character_get_speed(const character_t *c);
void character_set_speed(character_t *c, int32_t speed);
void character_set_kills(character_t *c, uint16_t type, int32_t number);
uint32_t character_get_kills(const character_t *c, uint16_t type);
void character_set_sequenceNumber(character_t *c, uint32_t cn);
uint32_t character_get_sequenceNumber(const character_t *c);

# ifdef __cplusplus
}
# endif
#endif
