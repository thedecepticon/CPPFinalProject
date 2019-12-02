#ifndef AREA_MAP_HPP
#define AREA_MAP_HPP

#include "species.hpp"
#include "attributes.hpp"

#include "environment.hpp"
#include "point.hpp"
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

//#define DEBUG

struct area_map{
  area_map(std::istream& inMap,std::istream& inSpec):mySpecies(inSpec){
    myMap = read(inMap); //dependent on species to read first
  }
  std::vector<std::vector<environment*> > read(std::istream& incoming){
      std::vector<std::vector<environment*> > localMap;
      std::string line;

    while(std::getline(incoming, line)){
        if(localMap.size() && line.size() != localMap.front().size()){
            std::cout<<"map dimension mismatch"<<std::endl;
            return std::vector<std::vector<environment*> >();
        }
        std::vector<environment*> currentrow;
        std::istringstream ss(line);
        char charIn;
        while(ss>>std::noskipws>>charIn){//don't skip whitespace
            currentrow.push_back(categorize(charIn));
        }          
        localMap.push_back(currentrow);//push finished row into matrix
    }
    return localMap;
   
  }//end read()

  void save(std::ostream& out) const{

      for ( auto j = 0; j < extent().y; ++j ){
            for ( auto i = 0; i < extent().x; ++i )
                out<< at( i, j );
            if (j !=extent().y-1) out <<"\n";//tag new lines minus last row
      }            
  }
  environment* categorize(char alpha){
    if(mySpecies.mySpecies.count(alpha)>0){
      return new environment(alpha,mySpecies.mySpecies.find(alpha)->second);
    }
    switch(alpha){
      case '~': return new environment('~');
      case ' ': return new environment(' ');
      case '#': return new environment('#');
    }
    return new environment('x');//indicate error
  }
 
  
  point extent() const {
    if (myMap.size()){ //captures seg fault if map not init
      return point (myMap.front().size(),myMap.size());
    }
    else{
      return point();
    }
  }
  
  //use at for printing id's
  const char& at(int i, int j) const {return myMap[j][i]->id; }  
  
  //check the area of the map around a living organism from the vector and try to decide if it is going to move, eat, grow, regrow, or flee.
  std::vector<environment*> detect(int i, int j){
      std::vector<environment*> neighbors;
      //up
      if(i-1>=0){
        neighbors.push_back(myMap[i-1][j]);
      }
      //down
      if(i+1 < extent().y){
        neighbors.push_back(myMap[i+1][j]);
      }
      //left
      if(j-1>=0){
        neighbors.push_back(myMap[i][j-1]);
      }
      //right
      if(j+1 < extent().x){
        neighbors.push_back(myMap[i][j+1]);
      }
      return neighbors;
  }
  //members
  //std::map<char,attr> species;
  species mySpecies;
  std::vector<std::vector<environment*> > myMap;
  
}; //end area_map

#endif