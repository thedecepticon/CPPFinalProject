#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "attributes.hpp"
#include "point.hpp"

class environment{
  public:
    environment(char id, point p):id(id), position(p){}
    environment(char id, attr info, point p):id(id),specs(info), position(p){}
    ~environment(){
      //std::cout<<id<<" died."<<std::endl;
    }
    environment* overlap; //when an organism is in the space of something that can be restored/regrow
    char id;
    attr specs; //dynamic attributes of an organism
    point position;
};

#endif