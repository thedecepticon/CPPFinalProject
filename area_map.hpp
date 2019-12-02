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
    int row = 0;
    
    while(std::getline(incoming, line)){
        
        if(localMap.size() && line.size() != localMap.front().size()){
            std::cout<<"map dimension mismatch"<<std::endl;
            return std::vector<std::vector<environment*> >();
        }
        std::vector<environment*> currentrow;
        std::istringstream ss(line);
        char charIn;
        int col = 0;
        while(ss>>std::noskipws>>charIn){//don't skip whitespace
            currentrow.push_back(categorize(charIn,point(row,col)));
            ++col;
        }          
        localMap.push_back(currentrow);//push finished row into matrix
        ++row;
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
  environment* categorize(char alpha, point p){
    if(mySpecies.mySpecies.count(alpha)>0){
      return new environment(alpha,mySpecies.mySpecies.find(alpha)->second, p);
    }
    switch(alpha){
      case '~': return new environment('~', p);
      case ' ': return new environment(' ', p);
      case '#': return new environment('#', p);
    }
    return new environment('x',p);//indicate error
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

  auto find(std::vector<char>& v, char const& s){
    auto iter = v.begin();
    auto end = v.end();
    for (; iter!=end; ++iter)
      if(*iter == s) return iter;
    return iter;
  }
  //iteration function for simulator
  void live(){
    //check for plants
    for (int i = 0; i < myMap.size();++i)
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* temp = myMap[i][j];
          std::string type = temp->specs.type;
              if(type=="plant"){
                  if (temp->specs.cur_energy < 0)
                      //eaten plants cur_energy field to regrow
                      temp->specs.cur_energy += 1;
                  if (temp->specs.cur_energy==0)
                      //if plant regrown set its energy back to max
                      temp->specs.cur_energy = temp->specs.max_energy;
              }
        }
    //check for herbivores
    for (int i = 0; i < myMap.size();++i)
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* temp = myMap[i][j];
          std::string type = temp->specs.type;
              if(type=="herbivore"){
                //check adjacent cells
                  std::vector<environment*> neighbor = detect(i,j);
                
                  bool predator=false;//determine if we need to flee
                  bool free = false; //are there free spaces to move to
                  bool edible = false; //is there food nearby
                  
                  for(environment* e : neighbor){
                      if (e->specs.food.size() > 0){
                          auto iter = find(e->specs.food, temp->id);
                          if (iter != e->specs.food.end())
                              predator = true;
                      }
                          
                      if (e->id==' ')
                          //there are free spaces to move to
                          free = true;
                      std::string eType = e->specs.type;
                      if (eType == "plant"){
                          if(e->specs.cur_energy < 0)
                              free = true; //not consumable but we could overlap
                      }
                      auto iter = find(temp->specs.food, e->id);
                      if (iter != temp->specs.food.end())
                          edible = true;
                  }
                  if (predator && free){
                      //move to a free cell

                  }
              }
        }
    //check for omnivores
    for (int i = 0; i < myMap.size();++i)
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* temp = myMap[i][j];
          std::string type = temp->specs.type;
              if(type=="omnimvore"){
                  
              }
        }
  }
  //members
  //std::map<char,attr> species;
  species mySpecies;
  std::vector<std::vector<environment*> > myMap;
  
}; //end area_map

#endif