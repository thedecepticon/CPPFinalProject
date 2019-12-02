#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include <vector>
#include <string>

class attr{
  public:
    attr(){};
    //plants
    attr(std::string t,int regrow, int m_energy):type(t), cur_energy(m_energy),max_energy(m_energy),regrowth(regrow){}
    //herb/omni
    attr(std::string t,int m_energy, std::vector<char> f, bool move):type(t),cur_energy(m_energy), max_energy(m_energy),food(f),moveable(move){}
    int cur_energy=0;
    int max_energy=0;
    std::vector<char> food;
    std::string type ="environment";
    int regrowth=0;
    bool moveable=false;
};

#endif