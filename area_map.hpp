#ifndef AREA_MAP_HPP
#define AREA_MAP_HPP

#include "species.hpp"
#include "attributes.hpp"

#include "environment.hpp"
#include "point.hpp"
#include <random>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>


struct area_map{
  area_map(std::istream& inMap,std::istream& inSpec):mySpecies(inSpec){
    myMap = read(inMap); //dependent on species to read first
  }
  ~area_map(){
      //free stack mem, overlap handled by environment destructor
      //std::cout<<"cleansing map data"<<std::endl;
      for (auto v : myMap)
        for (auto e : v)
            delete e;
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
    std::cout<<"Undefined species added to map. Definition not in species file.\n";
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
      //std::cout<<point(i,j)<<std::endl;
      //up
      if(i-1 >= 0){
        neighbors.push_back(myMap[i-1][j]);
      }
      //down
      if(i+1 < extent().y){
        neighbors.push_back(myMap[i+1][j]);
      }
      //left
      if(j-1 >= 0){
        neighbors.push_back(myMap[i][j-1]);
      }
      //right
      if(j+1 < extent().x){
        neighbors.push_back(myMap[i][j+1]);
      }
      return neighbors;
  }


    //iterator to search food chain (flee or edible)
  auto find(std::vector<char>& v, char const& s){
    auto iter = v.begin();
    auto end = v.end();
    for (; iter!=end; ++iter)
      if(*iter == s) return iter;
    return iter;
  }

  //iteration function for simulator
  void live(){
    //function setup
    std::vector<environment*> herbiList;
    std::vector<environment*> omniList;
    //randomly number generator
    std::random_device seed ;
    std::mt19937 engine( seed( ) ) ;
    //check/work on plants and fill lists to work on later in the same pass
    //(prevents those moving down and right from operating more than once per turn)
    for (int i = 0; i < myMap.size();++i)
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* curAnimal = myMap[i][j];
              if(curAnimal->specs.type=="plant"){
                  if (curAnimal->specs.cur_energy < 0)
                      //eaten plants cur_energy field to regrow
                      curAnimal->specs.cur_energy += 1;
                  if (curAnimal->specs.cur_energy==0)
                      //if plant regrown set its energy back to max
                      curAnimal->specs.cur_energy = curAnimal->specs.max_energy;
              }
              if(curAnimal->specs.type=="herbivore"){
                 herbiList.push_back(curAnimal);
              }
              if(curAnimal->specs.type=="omnivore"){
                 omniList.push_back(curAnimal);
              }
        }

    //check for herbivores
    for (environment* curAnimal: herbiList){
        int i = curAnimal->position.x;
        int j = curAnimal->position.y;
        //check adjacent cells
        std::vector<environment*> neighbor = detect(i,j);               
        bool predator = false;//determine if we need to flee
        bool free = false; //are there free spaces to move to
        bool edible = false; //is there food nearby
        std::vector<point> freeCells; //positions we can move to
        std::vector<point> consumable; //positions with food we can eat
        //classify the neighbors
        for(environment* curNeighbor : neighbor){
            if (curNeighbor->specs.food.size() > 0){
                auto iter = find(curNeighbor->specs.food, curAnimal->id);
                if (iter != curNeighbor->specs.food.end())
                    predator = true;
            }
            if (curNeighbor->id==' '){ //|| curNeighbor->id=='~')
                //there are free spaces to move to
                free = true;
                freeCells.push_back(curNeighbor->position);
            }
            auto iter = find(curAnimal->specs.food, curNeighbor->id);
            if (iter != curAnimal->specs.food.end()){
                if(curNeighbor->specs.cur_energy > 0){ //plants may be regrowing
                    edible = true;
                    //list of edible neighbors
                    consumable.push_back(curNeighbor->position);
                }
            }
        }

        std::uniform_int_distribution<int> chooseMove(0, freeCells.size() - 1);
        //std::cout<<freeCells[chooseMove(engine)] <<std::endl;
        //flee a predator
        if (predator && free){
            //std::cout<<"FLEEEEEE"<<std::endl;
            //move to a free cell
            if(freeCells.size() != 0){
                if(curAnimal->overlap==nullptr){
                    //no current overlap leave behind an empty space
                    myMap[i][j] = categorize(' ',point(i,j));
                }else{
                    myMap[i][j] = curAnimal->overlap; //replace what we stood on
                }
                point moveTo = freeCells[chooseMove(engine)];
                curAnimal->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                myMap[moveTo.x][moveTo.y] = curAnimal; //commit the move
                curAnimal->specs.cur_energy -= 1; //energy loss on move
                curAnimal->position = moveTo; //update internal position
            }else{
                curAnimal->specs.cur_energy -= 1; //loss of energy for a no move iteration
            }
        }else{
            //check the need to feed
            if(curAnimal->specs.cur_energy < curAnimal->specs.max_energy && edible){
                //std::cout<<"EAT"<<std::endl;
                //try to consume
                // number distribution
                std::uniform_int_distribution<int> chooseConsume(0, consumable.size() - 1);
                point pConsume = consumable[chooseConsume(engine)]; //get the position of the food
                environment* food = myMap[pConsume.x][pConsume.y]; //get the food
                //double check food type for herbivore
                if(food->specs.type=="plant"){
                    //plants regrow, hang on to it
                    if (curAnimal->overlap == nullptr){
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                    }else{
                        //already overlapping something, put it back as we move
                        myMap[i][j] = curAnimal->overlap;
                    }
                    curAnimal->overlap = food; //set the new overlap
                    myMap[food->position.x][food->position.y] = curAnimal; //commit the move
                    curAnimal->specs.cur_energy -= 1; //energy loss on move
                    curAnimal->position = food->position; //update internal position
                    //consume the energy
                    curAnimal->specs.cur_energy += food->specs.cur_energy;
                    //dont exceed the max
                    if (curAnimal->specs.cur_energy > curAnimal->specs.max_energy) 
                        curAnimal->specs.cur_energy = curAnimal->specs.max_energy;
                    //set plant to regrow mode
                    food->specs.cur_energy = -food->specs.regrowth;
                }//end if food == plant
            }//end need to feed
            else{
                //std::cout<<"moving"<<std::endl;
                //not hungry just move
                //move to a free cell
                if(freeCells.size() != 0){
                    if(curAnimal->overlap==nullptr){
                        //std::cout<<"new Overlap"<<std::endl;
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                    }else{
                        //std::cout<<"standing on something"<<std::endl;
                        //std::cout<<freeCells.size()<<std::endl;
                        myMap[i][j] = curAnimal->overlap; //replace what we stood on
                        //std::cout<<curAnimal->position<<std::endl;
                    }
                    point moveTo = freeCells[chooseMove(engine)];
                    curAnimal->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                    myMap[moveTo.x][moveTo.y] = curAnimal; //commit the move
                    curAnimal->specs.cur_energy -= 1; //energy loss on move
                    curAnimal->position = moveTo; //update internal position
                }else{
                    curAnimal->specs.cur_energy -= 1; //loss of energy for a no move iteration
                }
            }
            //procreate
            //check for energy to mate
            if (curAnimal->specs.cur_energy > curAnimal->specs.max_energy/2){
                //check adjacent cells again position may have changed
                neighbor = detect(curAnimal->position.x,curAnimal->position.y);
                std::vector<environment*> mates;
                std::vector<point> babyFreeCells;
                for(environment* curNeighbor : neighbor){
                    //look for new free spaces
                    if(curNeighbor->id == ' ') babyFreeCells.push_back(curNeighbor->position);//current org free space
                    //look for potential mates
                    if (curNeighbor->id==curAnimal->id && curNeighbor->specs.cur_energy > curNeighbor->specs.max_energy/2){
                        //base requirements met
                        std::vector<environment*> neighNeigh = detect(curNeighbor->position.x,curNeighbor->position.y);
                        for(environment* free : neighNeigh){
                            if (free->id == ' ') babyFreeCells.push_back(free->position); //mate free spaces
                        }
                        mates.push_back(curNeighbor);
                    }
                }
                //process mate results
                bool baby = false;
                if (mates.size() > 0 && babyFreeCells.size() > 0) baby = true;
                if (baby){
                    //choose the free spot to place the new animal
                    std::uniform_int_distribution<int> chooseBaby(0, babyFreeCells.size() - 1);
                    point newBaby = babyFreeCells[chooseBaby(engine)];
                    environment* babyAnimal = categorize(curAnimal->id,newBaby); //make the baby
                    babyAnimal->specs.cur_energy = curAnimal->specs.cur_energy/2; //babies cannot make babies right away
                    babyAnimal->overlap = myMap[newBaby.x][newBaby.y]; //baby should be overlapping ' ' (free space)
                    myMap[newBaby.x][newBaby.y] = babyAnimal; //put the baby in play
                }
            }
        }//end else after predator
        //check energy for death
        if (curAnimal->specs.cur_energy <=0){
            //std::cout<<"herbivore died of energy loss"<<std::endl;
            if (curAnimal->overlap == nullptr){
                //std::cout<<"not standing on anything"<<std::endl;
                myMap[curAnimal->position.x][curAnimal->position.y] = categorize(' ', point(curAnimal->position.x,curAnimal->position.y));
                curAnimal->overlap = nullptr;
                delete curAnimal;
            }else{
              // std::cout<<"was standing on something"<<std::endl;
              // std::cout<<curAnimal->id<<" at "<<curAnimal->position;
              // std::cout<<" standing on "<<curAnimal->overlap->id<<" at " << curAnimal->overlap->position<<std::endl;
              myMap[curAnimal->position.x][curAnimal->position.y] = curAnimal->overlap;
              curAnimal->overlap = nullptr;
              delete curAnimal;
            }
        }
    }// end for loop on herbivores

    //check for omnivores
    for (environment* curAnimal: omniList){
        int i = curAnimal->position.x;
        int j = curAnimal->position.y;
        //check adjacent cells
        std::vector<environment*> neighbor = detect(i,j);               
        bool predator=false;//determine if we need to flee
        bool free = false; //are there free spaces to move to
        bool edible = false; //is there food nearby
        std::vector<point> freeCells; //positions we can move to
        std::vector<point> consumable; //positions with food we can eat
        for(environment* curNeighbor : neighbor){
            if (curNeighbor->specs.food.size() > 0){
                auto iter = find(curNeighbor->specs.food, curAnimal->id);
                if (iter != curNeighbor->specs.food.end())
                    predator = true;
            }
            if (curNeighbor->id==' '){ //|| curNeighbor->id=='~')
                //there are free spaces to move to
                free = true;
                freeCells.push_back(curNeighbor->position);
            }
            auto iter = find(curAnimal->specs.food, curNeighbor->id);
            if (iter != curAnimal->specs.food.end()){
                if(curNeighbor->specs.cur_energy > 0){ // plants may be regrowing
                    edible = true;
                    //list of edible neighbors
                    consumable.push_back(curNeighbor->position);
                }
            }
        }
        
        // number distribution
        std::uniform_int_distribution<int> chooseMove(0, freeCells.size() - 1);
        //std::cout<<freeCells[chooseMove(engine)] <<std::endl;
        //flee a predator
        if (predator && free){
            //std::cout<<"FLEEEEEE"<<std::endl;
            //move to a free cell
            if(freeCells.size() != 0){
                if(curAnimal->overlap==nullptr){
                    //no current overlap leave behind an empty space
                    myMap[i][j] = categorize(' ',point(i,j));
                }else{
                    myMap[i][j] = curAnimal->overlap; //replace what we stood on
                }
                point moveTo = freeCells[chooseMove(engine)];
                curAnimal->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                myMap[moveTo.x][moveTo.y] = curAnimal; //commit the move
                curAnimal->specs.cur_energy -= 1; //energy loss on move
                curAnimal->position = moveTo; //update internal position
            }else{
                curAnimal->specs.cur_energy -= 1; //loss of energy for a no move iteration
            }
        }else{
            //check the need to feed
            if(curAnimal->specs.cur_energy < curAnimal->specs.max_energy && edible){
            //if(edible){
                //std::cout<<"EAT"<<std::endl;
                //try to consume
                // number distribution
                std::uniform_int_distribution<int> chooseConsume(0, consumable.size() - 1);
                point pConsume = consumable[chooseConsume(engine)]; //get the position of the food
                environment* food = myMap[pConsume.x][pConsume.y]; //get the food
                //double check food type for herbivore
                if(food->specs.type=="plant"){
                    //plants regrow, hang on to it
                    if (curAnimal->overlap == nullptr){
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                    }else{
                        //already overlapping something, put it back as we move
                        myMap[i][j] = curAnimal->overlap;
                    } 
                    curAnimal->overlap = food; //set the new overlap
                    myMap[food->position.x][food->position.y] = curAnimal; //commit the move
                    curAnimal->specs.cur_energy -= 1; //energy loss on move
                    curAnimal->position = food->position; //update internal position
                    //consume the energy
                    curAnimal->specs.cur_energy += food->specs.cur_energy;
                    //dont exceed the max
                    if (curAnimal->specs.cur_energy > curAnimal->specs.max_energy) 
                        curAnimal->specs.cur_energy = curAnimal->specs.max_energy;
                    //set plant to regrow mode
                    food->specs.cur_energy = -food->specs.regrowth;
                }else{
                    //omnivore can eat more than plants
                    if(curAnimal->overlap == nullptr){
                        //no current overlap leave behind an empty space as we consume
                        myMap[i][j] = categorize(' ',point(i,j));
                    }else{
                        //current standing on something put it back as we move
                        myMap[i][j] = curAnimal->overlap;
                        
                    }
                    //transfer foods overlap
                    curAnimal->overlap = food->overlap;
                    food->overlap = nullptr; 
                    curAnimal->position = food->position; //update internal position
                    myMap[food->position.x][food->position.y] = curAnimal; //commit the move
                    curAnimal->specs.cur_energy -= 1; //energy loss on move
                    //consume the energy
                    curAnimal->specs.cur_energy += food->specs.cur_energy;
                    //dont exceed the max
                    if (curAnimal->specs.cur_energy > curAnimal->specs.max_energy) 
                        curAnimal->specs.cur_energy = curAnimal->specs.max_energy;
                    //kill off the Animal
                    delete food;
                }
            }//end need to feed
            else{
                //std::cout<<"moving"<<std::endl;
                //not hungry just move
                //move to a free cell
                if(freeCells.size() != 0){
                    if(curAnimal->overlap==nullptr){
                        //std::cout<<"new Overlap"<<std::endl;
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                    }else{
                        //std::cout<<"standing on something"<<std::endl;
                        //std::cout<<freeCells.size()<<std::endl;
                        myMap[i][j] = curAnimal->overlap; //replace what we stood on
                        //std::cout<<curAnimal->position<<std::endl;
                    }
                    point moveTo = freeCells[chooseMove(engine)];
                    curAnimal->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                    myMap[moveTo.x][moveTo.y] = curAnimal; //commit the move
                    curAnimal->specs.cur_energy -= 1; //energy loss on move
                    curAnimal->position = moveTo; //update internal position
                }else{
                    curAnimal->specs.cur_energy -= 1; //loss of energy for a no move iteration
                }
            }//end of else case. move, no consumption
            //procreate
            //check for energy to mate
            if (curAnimal->specs.cur_energy > curAnimal->specs.max_energy/2){
                //check adjacent cells again position may have changed
                neighbor = detect(curAnimal->position.x,curAnimal->position.y);
                std::vector<environment*> mates;
                std::vector<point> babyFreeCells;
                for(environment* curNeighbor : neighbor){
                    //look for new free spaces
                    if(curNeighbor->id == ' ') babyFreeCells.push_back(curNeighbor->position);//current org free space
                    //look for potential mates
                    if (curNeighbor->id==curAnimal->id && curNeighbor->specs.cur_energy > curNeighbor->specs.max_energy/2){
                        //base requirements met
                        std::vector<environment*> neighNeigh = detect(curNeighbor->position.x,curNeighbor->position.y);
                        for(environment* free : neighNeigh){
                            if (free->id == ' ') babyFreeCells.push_back(free->position); //mate free spaces
                        }
                        mates.push_back(curNeighbor);
                    }
                }
                //process mate results
                bool baby = false;
                if (mates.size() > 0 && babyFreeCells.size() > 0) baby = true;
                if (baby){
                    std::uniform_int_distribution<int> chooseBaby(0, babyFreeCells.size() - 1);
                    point newBaby = babyFreeCells[chooseBaby(engine)];
                    environment* babyAnimal = categorize(curAnimal->id,newBaby); //make the baby
                    babyAnimal->specs.cur_energy = curAnimal->specs.cur_energy/2; //babies cannot make babies rightaway
                    babyAnimal->overlap = myMap[newBaby.x][newBaby.y]; //baby should be overlapping ' ' (free space)
                    myMap[newBaby.x][newBaby.y] = babyAnimal; //put the baby in play
                }
            }
        }//end else after predator
        //check energy for death
        if (curAnimal->specs.cur_energy <=0){
            //std::cout<<"herbivore died of energy loss"<<std::endl;
            if (curAnimal->overlap == nullptr){
                //std::cout<<"not standing on anything"<<std::endl;
                myMap[curAnimal->position.x][curAnimal->position.y] = categorize(' ', point(curAnimal->position.x,curAnimal->position.y));
                curAnimal->overlap = nullptr;
                delete curAnimal;
            }else{
              // std::cout<<"was standing on something"<<std::endl;
              // std::cout<<curAnimal->id<<" at "<<curAnimal->position;
              // std::cout<<" standing on "<<curAnimal->overlap->id<<" at " << curAnimal->overlap->position<<std::endl;
              myMap[curAnimal->position.x][curAnimal->position.y] = curAnimal->overlap;
              curAnimal->overlap = nullptr;
              delete curAnimal;
            }
        }
    }// end for loop on omnivores
  }//end of function live()
  //members
  species mySpecies;
  std::vector<std::vector<environment*> > myMap;
  
}; //end area_map

#endif