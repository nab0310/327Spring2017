#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <string.h>
#include "npc.h"

#define SAVE_DIR               ".rlg327"
#define MONSTER_SAVE_FILE      "monster_desc.txt"

using namespace std;

int main(int argc, char const *argv[]) {
  char *home;
  char *filename;
  size_t len;
  int place =0;

  if (!(home = getenv("HOME"))) {
    fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
    home = (char *) ".";
  }

  len = (strlen(home) + strlen(SAVE_DIR) + strlen(MONSTER_SAVE_FILE) +
         1 /* The NULL terminator */                                 +
         2 /* The slashes */);

  filename = (char *) malloc(len * sizeof (*filename));
  sprintf(filename, "%s/%s/", home, SAVE_DIR);
  strcat(filename, MONSTER_SAVE_FILE);

  ifstream in(filename);

  if(in == NULL){
    cout << "Couldn't open path "+ *filename <<endl;
  }

  string line;
  int lineCount;

  while (getline(in,line) != NULL) {
    lineCount++;
    if(lineCount==1 && line != "RLG327 MONSTER DESCRIPTION 1"){
      cout << "Error in Line 1" << endl;
      exit(-1);
    }

    vector<string> lineToBeParsed;
    vector<string> foundFields;
    string name;
    string description;
    string color;
    string symbol;
    string speed;
    string ability;
    string hp;
    string dmg;
    vector<string>::iterator it;

    if(line == "BEGIN MONSTER"){
      string monsterLine;
      getline(in, monsterLine);
      bool nameFlag = false;
      bool descriptionFlag = false;
      bool colorFlag = false;
      bool symbFlag = false;
      bool speedFlag = false;
      bool abilityFlag = false;
      bool hpFlag = false;
      bool dmgFlag = false;
      bool missingEnd = false;

      cout<< '\n';
      int lastLength = 0;

      while(monsterLine != "END"){
        lineToBeParsed.clear();

        if(monsterLine == "BEGIN MONSTER"){
          missingEnd = true;
          in.seekg(place);
          break;
        }
        place = in.tellg();

        string temp;
        for (stringstream s(monsterLine); s >> temp;) {
          lineToBeParsed.push_back(temp);
        };

        int i=0;
        for(it = lineToBeParsed.begin(); it != lineToBeParsed.end(); it++){
          if(*it == "NAME"){
            descriptionFlag = false;
            colorFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = false;
            nameFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("NAME");
          }else if(*it == "DESC"){
            nameFlag = false;
            colorFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = false;
            descriptionFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("DESC");
          }else if(*it == "COLOR"){
            nameFlag = false;
            descriptionFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = false;
            colorFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("COLOR");
          }else if(*it == "SYMB"){
            nameFlag = false;
            descriptionFlag = false;
            colorFlag = false;
            speedFlag = false;
            abilityFlag = false;
            symbFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("SYMB");
          }else if(*it == "SPEED"){
            nameFlag = false;
            descriptionFlag = false;
            colorFlag = false;
            symbFlag = false;
            abilityFlag = false;
            speedFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("SPEED");
          }else if(*it == "ABIL"){
            nameFlag = false;
            descriptionFlag = false;
            colorFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = true;
            hpFlag = false;
            dmgFlag = false;
            foundFields.push_back("ABIL");
          }else if(*it == "HP"){
            nameFlag = false;
            descriptionFlag = false;
            colorFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = false;
            hpFlag = true;
            dmgFlag = false;
            foundFields.push_back("HP");
          }else if(*it == "DAM"){
            nameFlag = false;
            descriptionFlag = false;
            colorFlag = false;
            symbFlag = false;
            speedFlag = false;
            abilityFlag = false;
            hpFlag = false;
            dmgFlag = true;
            foundFields.push_back("DAM");
          }
          else{
            //If it is not one of the flag values, then it is the values
            if(nameFlag){
              name += *it + " ";
            }else if(descriptionFlag){
              description += *it + " ";
            }else if(colorFlag){
              color += *it + " ";
            }else if(symbFlag){
              symbol += *it + " ";
            }else if(speedFlag){
              speed += *it + " ";
            }else if(abilityFlag){
              ability += *it + " ";
            }else if(hpFlag){
              hp += *it + " ";
            }else if(dmgFlag){
              dmg += *it + " ";
            }
          }
        }
        if(foundFields.back() == "DESC"){
          description += '\n';
          if(description.size() - lastLength > 79){
            cout << "Description is too wide.." << endl;
            break;
          }else{
            lastLength = description.size();
          }
        }
        getline(in, monsterLine);
      }
      if(missingEnd){
        cout<< "Missing the end symbol" << endl;
      }
      //Check if all 6 fields have been filled out
      else if(foundFields.size() > 8){
        //We dont have all the values of the fields, abort..
        cout << "We are missing a field" << endl;
      }
      else if(foundFields.size() < 8){
        //Duplicate fields
        cout << "We have a duplicate field" << endl;
      }else{
        //print values
        cout << name << endl;
        cout << description.substr(0, description.size()-4) << endl;
        cout << symbol << endl;
        cout << color << endl;
        cout << speed << endl;
        cout << ability << endl;
        cout << hp << endl;
        cout << dmg << endl;
        cout << '\n';
      }
    }
  }

  return 0;
}
