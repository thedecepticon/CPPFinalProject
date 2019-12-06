#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "area_map.hpp"
#include <fstream>
#include <iostream>

class simulation{
  public:
  simulation(std::istream& inMap,std::istream& inSpecies):envMap(inMap,inSpecies){}

  void printMap(){
    std::ostringstream out;
    envMap.save(out);  //save reused
    std::cout<<out.str()<<std::endl;
  }

  void saveMap(std::string fn="testSave.txt"){
    std::ofstream output(fn);
    envMap.save(output); //save reused
    output.close();
  }

  void printSpecies(){
    std::ostringstream out;
    envMap.mySpecies.saveSpecies(out);
    std::cout<<out.str()<<std::endl;
  }

  void saveSpecies(std::string fn="testSaveSpecies.txt"){
    std::ofstream output(fn);
    envMap.mySpecies.saveSpecies(output); //save reused
    output.close();
  }

  //map
  area_map envMap; //error when trying to initialize as envMap(12,48) 

  // engine/menu once started 
  
};//end class simulation

#endif