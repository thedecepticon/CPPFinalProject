//Testing testing

#include "simulation.hpp"
#include "area_map.hpp"
#include "point.hpp"

#include "environment.hpp"
#include "species.hpp"
#include "attributes.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

#define DEBUG
//building components for class project
//link to base github announcement https://github.com/uiowa-cs-3210-0001/cs3210-assignments-fall2019/tree/master/course-project
//concepts
// https://repl.it/@Sorceror89/lab13-exercise2
// https://repl.it/@Sorceror89/lab14-exercise1
//reference to matrix homework https://repl.it/@Sorceror89/hw9-problem1
//read istream https://repl.it/@Sorceror89/hw10-problem1
//https://repl.it/@Sorceror89/hw11-problem1
//https://repl.it/@Sorceror89/stringtolist
//https://repl.it/@Sorceror89/Readline     //error reproduction
//https://repl.it/@Sorceror89/Random-number-test
//https://repl.it/@Sorceror89/enum-tests
//https://repl.it/@Sorceror89/CommandArgs   //repl.it command line usage

int main(int argc, char** argv) {
  //base testing
  // organism a1('o');
  // std::cout<<a1.id<<std::endl;

  // area_map::planta p1('a');
  // std::cout<<p1.id<<std::endl;

  // std::fstream readMap("map.txt");
  // area_map areaMap(readMap); //map initialized with organism x
  // std::cout << areaMap.at(3,3) <<std::endl;
  // readMap.close();
  
  //given map read
#ifdef DEBUG
  std::fstream readS("species.txt");
  std::fstream readM("map.txt"); 
  simulation sim(readM,readS);
  readM.close();
  readS.close();
  std::cout<<"Printing map"<<std::endl;
  sim.printMap();
  //std::cout<<"saving to file"<<std::endl;
  //sim.saveMap();//testSave.txt
  sim.mainMenu();
#else
    if (argc < 3) {
        std::cout<<"Missing arguments. Require map and species files as arguments\n";
        std::cout<<"Sample command ./main map.txt species.txt\n";
        return 1;
    }
    if (argc > 3) {
        std::cout<<"Too many arguments supplied. Require map and species files as arguments.\n";
        std::cout<<"Sample command ./main map.txt species.txt\n";
        return 1;
    }
        
    std::fstream readS(argv[2]);
    std::fstream readM(argv[1]); 
    simulation sim(readM,readS);
    readM.close();
    readS.close();
    sim.mainMenu();
#endif
  // std::cout<<sim.envMap.at(48,12)<<std::endl; //space
  // //std::cout<<sim.envMap.at(49,12)<<std::endl; //seg fault
  // std::cout<<sim.envMap.at(49,11)<<std::endl; // x 
  // std::cout<<sim.envMap.at(49,0)<<std::endl; //shows the x id

  //saved map read
//   std::fstream readS2("species.txt");
//   std::fstream read2nd("testSave.txt");
//   simulation testtwo(read2nd,readS2);
//   read2nd.close();
//   readS2.close();
  // assignment testing
  // environment* overwrite = new environment('P', point(0,1));
  // testtwo.envMap.myMap[0][1] = overwrite;
  // testtwo.envMap.myMap[1][5]->overlap = overwrite;
  // std::cout<< std::boolalpha << overwrite->specs.moveable <<std::endl;
  // std::cout<< std::boolalpha<< testtwo.envMap.myMap[1][5]->specs.type<<std::endl;
  // std::cout<<testtwo.envMap.detect(0,0).size()<<std::endl;
  // std::cout << testtwo.envMap.myMap[1][5]->position.y <<std::endl;

  //std::cout<<"Printing map"<<std::endl;
  //testtwo.printMap();
  //testtwo.printSpecies();
  //std::cout<<"saving to file"<<std::endl;
  //testtwo.saveMap("testSavingAReadSave.txt");
  // std::cout<<testtwo.envMap.at(43,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(44,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(45,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(46,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(47,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(48,0)<<std::endl;
  // std::cout<<testtwo.envMap.at(49,0)<<std::endl; //seg fault

  //std::cout<<testtwo.envMap.extent()<<std::endl;
  //random free space selection
//   for (int i = 0; i < 1000; ++i)
//     testtwo.envMap.live();
//   std::cout<<"Printing map"<<std::endl;
//   testtwo.printMap();
   //testtwo.mainMenu();

  //species file testing
//   std::cout<<std::endl;
//   std::fstream rdspecies("species.txt");
//   // std::map<char, attr> species = readSpecies(rdspecies);
//   species species(rdspecies);
//   rdspecies.close();
//   // for (auto e : species)
//   //   std::cout << e.second.type<<std::endl;

//   //species.printSpecies();
  
//   std::ofstream output("testSaveSpecies.txt");
//   species.saveSpecies(output);
//   output.close();
}