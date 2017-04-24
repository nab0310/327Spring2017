#ifndef PC_H
# define PC_H

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
#endif

# include "dims.h"
# include "dungeon.h"
# include "character.h"

# ifdef __cplusplus
}
#endif

# ifdef __cplusplus

class pc_t : public character_t {
public:
  pc_t();
  ~pc_t();
};

extern "C" {
# else
typedef void pc_t;
# endif

typedef struct dungeon dungeon_t;
#include "dungeon.h"

uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);
void pc_delete(pc_t *p);

# ifdef __cplusplus
}
# endif
#endif
