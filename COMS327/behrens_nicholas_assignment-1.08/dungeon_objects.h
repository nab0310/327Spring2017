#ifndef DUNGEON_OBJECTS_H
#define DUNGEON_OBJECTS_H

# include <stdint.h>

# include "dims.h"
# include <vector>
# include <string>
# include "dice.h"
# include "descriptions.h"

typedef struct dungeon dungeon_t;

void gen_objects(dungeon_t *d);

class dungeon_object {
private:
  std::string name;
  object_type_t type;
public:
  pair_t position;
  char symbol;
  int hit, dodge, defence, weight, speed, attribute, value;
  dice damage;
  uint32_t have_seen_pc;
  uint32_t color;
  dungeon_object() {};
  dungeon_object(const std::string &name,
           const object_type_t type,
           const uint32_t color,
           const uint32_t hit,
           const dice &damage,
           const uint32_t dodge,
           const uint32_t defence,
           const uint32_t weight,
           const uint32_t speed,
           const uint32_t attrubute,
           const uint32_t value,
           const char symbol){
             this->name = name;
             this->type = type;
             this->color = color;
             this->hit = hit;
             this->damage = damage;
             this->dodge = dodge;
             this->defence = defence;
             this->weight = weight;
             this->speed = speed;
             this->attribute = attrubute;
             this->value = value;
             this->symbol = symbol;
           };
   inline const std::string &get_name() const { return name; }
   inline const object_type_t get_type() const { return type; }
   inline const uint32_t get_color() const { return color; }
   inline const uint32_t get_hit() const { return hit; }
   inline const dice &get_damage() const { return damage; }
   inline const uint32_t get_dodge() const { return dodge; }
   inline const uint32_t get_defence() const { return defence; }
   inline const uint32_t get_weight() const { return weight; }
   inline const uint32_t get_speed() const { return speed; }
   inline const uint32_t get_attribute() const { return attribute; }
   inline const uint32_t get_value() const { return value; }
   inline const void set_type(object_type_t type) { this->type = type; }
   inline const void set_color(uint32_t color) { this->color = color; }
   inline const void set_hit(uint32_t hit) { this->hit = hit; }
   inline const void set_damage(dice & damage) { this->damage = damage; }
   inline const void set_dodge(uint32_t dodge) { this->dodge = dodge; }
   inline const void set_defence(uint32_t defence) { this->defence = defence; }
   inline const void set_weight(uint32_t weight) { this->weight = weight; }
   inline const void set_speed(uint32_t speed) { this->speed = speed; }
   inline const void set_attribute(uint32_t attribute) { this->attribute = attribute; }
   inline const void set_value(uint32_t value) { this->value = value; }
};

int16_t *object_get_postition(dungeon_object *d_o);
char object_get_symbol(dungeon_object *d_o);
uint32_t object_get_color(dungeon_object *d_o);
// inline const void &object_get_name(dungeon_object *d_o) const { return d_o->name;}
// inline const void object_get_type(dungeon_object *d_o) const { d_o->type; }
// inline const void object_get_color(dungeon_object *d_o) const { d_o->color; }
// inline const void object_get_hit(dungeon_object *d_o) const {d_o-> d_o->hit; }
// inline const void object_get_damage(dungeon_object *d_o) const { d_o->damage; }
// inline const void object_get_dodge(dungeon_object *d_o) const { d_o->dodge; }
// inline const void object_get_defence(dungeon_object *d_o) const { d_o->defence = defence; }
// inline const void object_get_weight(dungeon_object *d_o) const { d_o->weight = weight; }
// inline const void object_get_speed(dungeon_object *d_o) const { d_o->speed = speed; }
// inline const void object_get_attribute(dungeon_object *d_o) const { d_o->attribute = attribute; }
// inline const void object_get_value(dungeon_object *d_o) const { d_o->value = value; }

#endif
