#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "area_map.hpp"
#include <fstream>
#include <iostream>

class simulation{
    public:
    simulation(std::istream& inMap,std::istream& inSpecies):envMap(inMap,inSpecies){}

    enum menuOptions{A, B, E, S, Y, N, Unknown};
    std::map<std::string, menuOptions> options{
        {"A", A},
        {"B", B},
        {"E", E},
        {"S", S},
        {"Y", Y},
        {"N", N}
    };

    menuOptions lookupOption(std::string opt){
        auto iter = options.begin();
        auto end = options.end();
        for ( ; iter!= end; ++iter)
            if (iter->first == opt)
                return iter->second;
        return Unknown;
    }

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

    void iterations(int j){
        for (int i = 0; i < j; ++i)
            envMap.live();
        std::cout<<"Iteration(s) complete. Current environment"<<std::endl;
        printMap();
    }

    void optionSave(){
        std::string fn = "default.txt";
        std::cout<<"Enter a filename: ";
        std::cin>>fn;
        std::cout<<"Saving...\n";
        saveMap(fn);
        std::cout<<"Save complete"<<std::endl;
    }

    void mainMenu(){
        bool b = true;
        std::cout<<"Welcome to species simulation"<<std::endl;
        if(envMap.myMap.size()==0 || envMap.myMap.front().size()==0){
            std::cout<<"Error reading map data, please check the map file and try again.\n";
            b = false;
        }
        if(envMap.mySpecies.mySpecies.size()==0){
            std::cout<<"Error reading species data, none found. Please check the species file and try again.\n";
            b = false;
        }
        bool firstRun = true;
        while(b){
            std::string choice = "";
            if (firstRun != false){
                std::cout<<"Would you like to run a single iteration or a batch of 10?\n";
                std::cout<<"Enter A for single, B for Batch, E to exit: ";
            }else{
                std::cout<<"Would you like to run a single iteration or a batch of 10?\n";
                std::cout<<"You may also choose to save the current map.\n";
                std::cout<<"Enter A for single, B for Batch, S to save, E to exit: ";
            }
            std::cout.flush();
            std::cin>>choice;
            switch(lookupOption(choice)){
                case 0: iterations(1); firstRun = false; break;
                case 1: iterations(10); firstRun = false; break;
                case 2: {
                    if (firstRun!= true){
                        bool saving=true;
                        while(saving){
                            std::string choice = "";
                            std::cout<<"Map has been altered would you like to save? Y/N: ";
                            std::cout.flush();
                            std::cin>>choice;
                            switch(lookupOption(choice)){
                                case 4: optionSave(); b = false; saving = false; break;
                                case 5: b = false; saving = false; break;
                                default: std::cout<<"Choice unknown, please try again."<<std::endl;
                            }
                        }
                        break;
                    }else{
                        b = false; break;
                    }
                }
                case 3: {
                    if(firstRun!= true){
                        optionSave();
                        b = false;
                        break;
                    }
                }
                default: std::cout<<"Choice unknown, please try again."<<std::endl;
            }
            
            
        }
        std::cout<<"Simulation terminating. Good-Bye."<<std::endl;
    }

    //map
    area_map envMap; //error when trying to initialize as envMap(12,48) 

    // engine/menu once started 
  
};//end class simulation

#endif