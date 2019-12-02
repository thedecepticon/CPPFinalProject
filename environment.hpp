#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "attributes.hpp"

class environment{
  public:
    environment(char id):id(id){}
    environment(char id, attr info):id(id),specs(info){}
    environment* overlap; //when an organism is in the space of something that can be restored/regrow
    char id;
    attr specs; //dynamic attributes of an organism
};

#endif