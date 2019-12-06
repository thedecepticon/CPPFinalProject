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

//#define DEBUG
//https://repl.it/@Sorceror89/Random-number-test

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
    //make a list of creatures to work on (prevents those moving down and right from operating more than once per turn)
    std::vector<environment*> herbiList;
    for (int i = 0; i < myMap.size();++i){
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* temp = myMap[i][j];
              if(temp->specs.type=="herbivore"){
                 herbiList.push_back(temp);
              }
        }
    }
    //check for herbivores
    
    for (environment* organism: herbiList){
        environment* temp = organism;
        int i = temp->position.x;
        int j = temp->position.y;
        //check adjacent cells
        std::vector<environment*> neighbor = detect(i,j);               
        bool predator=false;//determine if we need to flee
        bool free = false; //are there free spaces to move to
        bool edible = false; //is there food nearby
        std::vector<point> freeCells; //positions we can move to
        std::vector<point> consumable; //positions with food we can eat
        for(environment* e : neighbor){
            if (e->specs.food.size() > 0){
                auto iter = find(e->specs.food, temp->id);
                if (iter != e->specs.food.end())
                    predator = true;
            }
            if (e->id==' '){ //|| e->id=='~')
                //there are free spaces to move to
                free = true;
                freeCells.push_back(e->position);
            }
            auto iter = find(temp->specs.food, e->id);
            if (iter != temp->specs.food.end()){
                edible = true;
                //list of edible neighbors
                consumable.push_back(e->position);
            }
        }
        //randomly choose a direction to move in
        std::random_device seed ;
        // generator 
        std::mt19937 engine( seed( ) ) ;
        // number distribution
        std::uniform_int_distribution<int> chooseMove(0, freeCells.size() - 1);
        //std::cout<<freeCells[chooseMove(engine)] <<std::endl;
        //flee a predator
        if (predator && free){
            //std::cout<<"FLEEEEEE"<<std::endl;
            //move to a free cell
            if(freeCells.size() != 0){
              if(temp->overlap==nullptr){
                  //no current overlap leave behind an empty space
                  myMap[i][j] = categorize(' ',point(i,j));
                  point moveTo = freeCells[chooseMove(engine)];
                  temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                  myMap[moveTo.x][moveTo.y] = temp; //commit the move
                  temp->specs.cur_energy -= 1; //energy loss on move
                  temp->position = moveTo; //update internal position

              }else{
                  myMap[i][j] = temp->overlap; //replace what we stood on
                  point moveTo = freeCells[chooseMove(engine)];
                  temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                  myMap[moveTo.x][moveTo.y] = temp; //commit the move
                  temp->specs.cur_energy -= 1; //energy loss on move
                  temp->position = moveTo; //update internal position
              }
            }else{
                temp->specs.cur_energy -= 1; //loss of energy for a no move iteration
            }
        }else{
            //check the need to feed
            if(temp->specs.cur_energy < temp->specs.max_energy && edible){
                //std::cout<<"EAT"<<std::endl;
                //try to consume
                // number distribution
                std::uniform_int_distribution<int> chooseConsume(0, consumable.size() - 1);
                point pConsume = consumable[chooseConsume(engine)]; //get the position of the food
                environment* e = myMap[pConsume.x][pConsume.y]; //get the food
                //double check food type for herbivore
                if(e->specs.type=="plant"){
                    //plants regrow, hang on to it
                    if (temp->overlap == nullptr){
                        temp->overlap = e;//set the new overlap
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = e->position; //update internal position
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //set plant to regrow mode
                        e->specs.cur_energy = -e->specs.regrowth;
                    }else{
                        //already overlapping something, put it back as we move
                        myMap[i][j] = temp->overlap;
                        temp->overlap = e; //set the new overlap
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = e->position; //update internal position
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //set plant to regrow mode
                        e->specs.cur_energy = -e->specs.regrowth;
                    } 
                }//end if food == plant
                
            }//end need to feed
            else{
                //std::cout<<"moving"<<std::endl;
                //not hungry just move
                //move to a free cell
                if(freeCells.size() != 0){
                    if(temp->overlap==nullptr){
                        //std::cout<<"new Overlap"<<std::endl;
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                        point moveTo = freeCells[chooseMove(engine)];
                        temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                        myMap[moveTo.x][moveTo.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = moveTo; //update internal position

                    }else{
                        //std::cout<<"standing on something"<<std::endl;
                        //std::cout<<freeCells.size()<<std::endl;
                        myMap[i][j] = temp->overlap; //replace what we stood on
                        point moveTo = freeCells[chooseMove(engine)];
                        temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                        myMap[moveTo.x][moveTo.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = moveTo; //update internal position
                        //std::cout<<temp->position<<std::endl;
                    }
                }else{
                    temp->specs.cur_energy -= 1; //loss of energy for a no move iteration
                }
            }//end of else case. move, no consumption
            //procreate
            //check for energy to mate
            if (temp->specs.cur_energy > temp->specs.max_energy/2){
                //check adjacent cells again position may have changed
                neighbor = detect(temp->position.x,temp->position.y);
                std::vector<environment*> mates;
                std::vector<point> babyFreeCells;
                for(environment* e : neighbor){
                    //look for new free spaces
                    if(e->id == ' ') babyFreeCells.push_back(e->position);//current org free space
                    //look for potential mates
                    if (e->id==temp->id && e->specs.cur_energy > e->specs.max_energy/2){
                        //base requirements met
                        std::vector<environment*> neighNeigh = detect(e->position.x,e->position.y);
                        for(environment* f : neighNeigh){
                            if (f->id == ' ') babyFreeCells.push_back(f->position); //mate free spaces
                        }
                        mates.push_back(e);
                    }
                }
                //process mate results
                bool baby = false;
                if (mates.size() > 0 && babyFreeCells.size() > 0) baby = true;
                if (baby){
                    std::uniform_int_distribution<int> chooseBaby(0, babyFreeCells.size() - 1);
                    point newBaby = babyFreeCells[chooseBaby(engine)];
                    environment* babyAnimal = categorize(temp->id,newBaby); //make the baby
                    babyAnimal->specs.cur_energy = temp->specs.cur_energy/2;
                    babyAnimal->overlap = myMap[newBaby.x][newBaby.y]; //baby should be overlapping ' ' (free space)
                    myMap[newBaby.x][newBaby.y] = babyAnimal; //put the baby in play
                }
                

            }
            
            
        }//end else after predator
      
        //check energy for death
        if (temp->specs.cur_energy <=0){
            //std::cout<<"herbivore died of energy loss"<<std::endl;
            if (temp->overlap == nullptr){
                //std::cout<<"not standing on anything"<<std::endl;
                myMap[temp->position.x][temp->position.y] = categorize(' ', point(temp->position.x,temp->position.y));
                delete temp;
            }else{
              // std::cout<<"was standing on something"<<std::endl;
              // std::cout<<temp->id<<" at "<<temp->position;
              // std::cout<<" standing on "<<temp->overlap->id<<" at " << temp->overlap->position<<std::endl;
              myMap[temp->position.x][temp->position.y] = temp->overlap;
              delete temp;
            }
        }
    }// end for loop on herbivores

    //make a list of omnivores to work on
    std::vector<environment*> omniList;
    for (int i = 0; i < myMap.size();++i){
        for (int j = 0; j < myMap.front().size(); ++j){
          environment* temp = myMap[i][j];
              if(temp->specs.type=="omnivore"){
                 omniList.push_back(temp);
              }
        }
    }
    //check for omnivores
    for (environment* organism: omniList){
        environment* temp = organism;
        int i = temp->position.x;
        int j = temp->position.y;
        //check adjacent cells
        std::vector<environment*> neighbor = detect(i,j);               
        bool predator=false;//determine if we need to flee
        bool free = false; //are there free spaces to move to
        bool edible = false; //is there food nearby
        std::vector<point> freeCells; //positions we can move to
        std::vector<point> consumable; //positions with food we can eat
        for(environment* e : neighbor){
            if (e->specs.food.size() > 0){
                auto iter = find(e->specs.food, temp->id);
                if (iter != e->specs.food.end())
                    predator = true;
            }
            if (e->id==' '){ //|| e->id=='~')
                //there are free spaces to move to
                free = true;
                freeCells.push_back(e->position);
            }
            auto iter = find(temp->specs.food, e->id);
            if (iter != temp->specs.food.end()){
                edible = true;
                //list of edible neighbors
                consumable.push_back(e->position);
            }
        }
        //randomly choose a direction to move in
        std::random_device seed ;
        // generator 
        std::mt19937 engine( seed( ) ) ;
        // number distribution
        std::uniform_int_distribution<int> chooseMove(0, freeCells.size() - 1);
        //std::cout<<freeCells[chooseMove(engine)] <<std::endl;
        //flee a predator
        if (predator && free){
            //std::cout<<"FLEEEEEE"<<std::endl;
            //move to a free cell
            if(freeCells.size() != 0){
              if(temp->overlap==nullptr){
                  //no current overlap leave behind an empty space
                  myMap[i][j] = categorize(' ',point(i,j));
                  point moveTo = freeCells[chooseMove(engine)];
                  temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                  myMap[moveTo.x][moveTo.y] = temp; //commit the move
                  temp->specs.cur_energy -= 1; //energy loss on move
                  temp->position = moveTo; //update internal position

              }else{
                  myMap[i][j] = temp->overlap; //replace what we stood on
                  point moveTo = freeCells[chooseMove(engine)];
                  temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                  myMap[moveTo.x][moveTo.y] = temp; //commit the move
                  temp->specs.cur_energy -= 1; //energy loss on move
                  temp->position = moveTo; //update internal position
              }
            }else{
                temp->specs.cur_energy -= 1; //loss of energy for a no move iteration
            }
        }else{
            //check the need to feed
            if(temp->specs.cur_energy < temp->specs.max_energy && edible){
            //if(edible){
                //std::cout<<"EAT"<<std::endl;
                //try to consume
                // number distribution
                std::uniform_int_distribution<int> chooseConsume(0, consumable.size() - 1);
                point pConsume = consumable[chooseConsume(engine)]; //get the position of the food
                environment* e = myMap[pConsume.x][pConsume.y]; //get the food
                //double check food type for herbivore
                if(e->specs.type=="plant"){
                    //plants regrow, hang on to it
                    if (temp->overlap == nullptr){
                        temp->overlap = e;//set the new overlap
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = e->position; //update internal position
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //set plant to regrow mode
                        e->specs.cur_energy = -e->specs.regrowth;
                    }else{
                        //already overlapping something, put it back as we move
                        myMap[i][j] = temp->overlap;
                        temp->overlap = e; //set the new overlap
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = e->position; //update internal position
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //set plant to regrow mode
                        e->specs.cur_energy = -e->specs.regrowth;
                    } 
                }//end if food == plant
                //omnivore can eat more than plants
                else{
                    if(temp->overlap == nullptr){
                        //check for an overlap in the food and transfer it
                        //if (e->overlap != nullptr)
                            temp->overlap = e->overlap;
                        //no current overlap leave behind an empty space as we consume
                        myMap[i][j] = categorize(' ',point(i,j));
                        temp->position = e->position; //update internal position
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //kill off the Animal
                        delete e;
                    }else{
                        //current standing on something
                        myMap[i][j] = temp->overlap;
                        //check for an overlap in the food and transfer it
                        // if (e->overlap != nullptr)
                            temp->overlap = e->overlap;
                        
                        temp->position = e->position; //update internal position
                        myMap[e->position.x][e->position.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        //consume the energy
                        temp->specs.cur_energy += e->specs.cur_energy;
                        //dont exceed the max
                        if (temp->specs.cur_energy > temp->specs.max_energy) 
                            temp->specs.cur_energy = temp->specs.max_energy;
                        //kill off the Animal
                        delete e;
                    }
                    
                }
            }//end need to feed
            else{
                //std::cout<<"moving"<<std::endl;
                //not hungry just move
                //move to a free cell
                if(freeCells.size() != 0){
                    if(temp->overlap==nullptr){
                        //std::cout<<"new Overlap"<<std::endl;
                        //no current overlap leave behind an empty space
                        myMap[i][j] = categorize(' ',point(i,j));
                        point moveTo = freeCells[chooseMove(engine)];
                        temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                        myMap[moveTo.x][moveTo.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = moveTo; //update internal position

                    }else{
                        //std::cout<<"standing on something"<<std::endl;
                        //std::cout<<freeCells.size()<<std::endl;
                        myMap[i][j] = temp->overlap; //replace what we stood on
                        point moveTo = freeCells[chooseMove(engine)];
                        temp->overlap = myMap[moveTo.x][moveTo.y]; //store what we will be standing on
                        myMap[moveTo.x][moveTo.y] = temp; //commit the move
                        temp->specs.cur_energy -= 1; //energy loss on move
                        temp->position = moveTo; //update internal position
                        //std::cout<<temp->position<<std::endl;
                    }
                }else{
                    temp->specs.cur_energy -= 1; //loss of energy for a no move iteration
                }
            }//end of else case. move, no consumption
            //procreate
            //check for energy to mate
            if (temp->specs.cur_energy > temp->specs.max_energy/2){
                //check adjacent cells again position may have changed
                neighbor = detect(temp->position.x,temp->position.y);
                std::vector<environment*> mates;
                std::vector<point> babyFreeCells;
                for(environment* e : neighbor){
                    //look for new free spaces
                    if(e->id == ' ') babyFreeCells.push_back(e->position);//current org free space
                    //look for potential mates
                    if (e->id==temp->id && e->specs.cur_energy > e->specs.max_energy/2){
                        //base requirements met
                        std::vector<environment*> neighNeigh = detect(e->position.x,e->position.y);
                        for(environment* f : neighNeigh){
                            if (f->id == ' ') babyFreeCells.push_back(f->position); //mate free spaces
                        }
                        mates.push_back(e);
                    }
                }
                //process mate results
                bool baby = false;
                if (mates.size() > 0 && babyFreeCells.size() > 0) baby = true;
                if (baby){
                    std::uniform_int_distribution<int> chooseBaby(0, babyFreeCells.size() - 1);
                    point newBaby = babyFreeCells[chooseBaby(engine)];
                    environment* babyAnimal = categorize(temp->id,newBaby); //make the baby
                    babyAnimal->specs.cur_energy = temp->specs.cur_energy/2;
                    babyAnimal->overlap = myMap[newBaby.x][newBaby.y]; //baby should be overlapping ' ' (free space)
                    myMap[newBaby.x][newBaby.y] = babyAnimal; //put the baby in play
                }
                

            }
            
            
        }//end else after predator
        
        //check energy for death
        if (temp->specs.cur_energy <=0){
            //std::cout<<"herbivore died of energy loss"<<std::endl;
            if (temp->overlap == nullptr){
                //std::cout<<"not standing on anything"<<std::endl;
                myMap[temp->position.x][temp->position.y] = categorize(' ', point(temp->position.x,temp->position.y));
                delete temp;
            }else{
              // std::cout<<"was standing on something"<<std::endl;
              // std::cout<<temp->id<<" at "<<temp->position;
              // std::cout<<" standing on "<<temp->overlap->id<<" at " << temp->overlap->position<<std::endl;
              myMap[temp->position.x][temp->position.y] = temp->overlap;
              delete temp;
            }
        }
    }// end for loop on omnivores
  }//end of function live()
  //members
  species mySpecies;
  std::vector<std::vector<environment*> > myMap;
  
}; //end area_map

#endif