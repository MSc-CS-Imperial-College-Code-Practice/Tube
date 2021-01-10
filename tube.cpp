#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char **allocate_2D_array(int rows, int columns) {
  char **m = new char *[rows];
  assert(m);
  for (int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char **m, int rows) {
  for (int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char *filename, int &height, int &width) {
  char line[512];
  
  ifstream input(filename);

  height = width = 0;

  input.getline(line,512);  
  while (input) {
    if ( (int) strlen(line) > width)
      width = strlen(line);
    height++;
    input.getline(line,512);  
  }

  if (height > 0)
    return true;
  return false;
}

/* pre-supplied function to load a tube map from a file*/
char **load_map(const char *filename, int &height, int &width) {

  bool success = get_map_dimensions(filename, height, width);
  
  if (!success)
    return NULL;

  char **m = allocate_2D_array(height, width);
  
  ifstream input(filename);

  char line[512];
  char space[] = " ";

  for (int r = 0; r<height; r++) {
    input.getline(line, 512);
    strcpy(m[r], line);
    while ( (int) strlen(m[r]) < width )
      strcat(m[r], space);
  }
  
  return m;
}

/* pre-supplied function to print the tube map */
void print_map(char **m, int height, int width) {
  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    if (c && (c % 10) == 0) 
      cout << c/10;
    else
      cout << " ";
  cout << endl;

  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    cout << (c % 10);
  cout << endl;

  for (int r=0; r<height; r++) {
    cout << setw(2) << r << " ";    
    for (int c=0; c<width; c++) 
      cout << m[r][c];
    cout << endl;
  }
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char *error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID: 
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(const char *token) {
  const char *strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for (int n=0; n<8; n++) {
    if (!strcmp(token, strings[n])) 
      return (Direction) n;
  }
  return INVALID_DIRECTION;
}

bool get_symbol_position(char **map, int height, int width, 
    const char target,int &r,int &c){
      
  for(int row = 0; row < height; row++){
    for(int col = 0; col < width; col++){
      if(map[row][col] == target){
        r = row;
        c = col;
        return true;
      }
    }
  }
  r = -1;
  c = -1;
  return false;
}

char get_symbol_for_station_or_line(const char* name){

  ifstream in;
  char reader[512];
  char station_or_line_symbol;
  char station_or_line_name[512];
  in.open("stations.txt");
  if (!in)
    return ' ';
  while(!in.eof()){
    in.getline(reader,512);
    station_or_line_symbol = reader[0];
    strcpy(station_or_line_name, reader+2);
    if(strcmp(name,station_or_line_name)==0)
      return station_or_line_symbol;
  }
  in.close();

  in.open("lines.txt");
  if (!in)
    return ' ';
  while(!in.eof()){
    in.getline(reader,512);
    station_or_line_symbol = reader[0];
    strcpy(station_or_line_name, reader+2);
    if(strcmp(name,station_or_line_name)==0)
      return station_or_line_symbol;
  }
  in.close();

  return ' ';

}

void get_station_or_line_from_symbol(char symbol, char* name){

  ifstream in;
  char reader[512];
  char station_or_line_symbol;
  char station_or_line_name[512];
  in.open("stations.txt");
  if (!in)
    return;
  while(!in.eof()){
    in.getline(reader,512);
    station_or_line_symbol = reader[0];
    strcpy(station_or_line_name, reader+2);
    if(station_or_line_symbol == symbol)
      strcpy(name, station_or_line_name);
  }
  in.close();

  in.open("lines.txt");
  if (!in)
    return;
  while(!in.eof()){
    in.getline(reader,512);
    station_or_line_symbol = reader[0];
    strcpy(station_or_line_name, reader+2);
    if(station_or_line_symbol == symbol)
      strcpy(name, station_or_line_name);
  }
  in.close();
}

int validate_route(char **map, int height, int width, 
  const char* start_station, const char* route, char* destination){

  int number_of_line_changes;
  int current_row, current_col;
  int previous_row, previous_col;
  int current_row_dir, current_col_dir;
  int previous_row_dir, previous_col_dir;
  char current_path_sym, previous_path_sym;
  char lines_using[10]={};
  int number_of_stops;
  
  if (get_symbol_for_station_or_line(start_station) == ' ')
    return ERROR_START_STATION_INVALID;

  current_path_sym = get_symbol_for_station_or_line(start_station);
  get_symbol_position(map,height,width,current_path_sym,current_row,current_col);
  number_of_stops = number_of_stops_of_route(route);

  char *route_array[number_of_stops+1]; // addinf one space for NULL string
  split_string(route, ",", route_array);

  for(int i=0; route_array[i] != NULL; i++){
    previous_row = current_row;
    previous_col = current_col;
    previous_row_dir = (i == 0) ? 0 : current_row_dir;
    previous_col_dir = (i == 0) ? 0 : current_col_dir;
    previous_path_sym = (i == 0) ? ' ' : map[current_row][current_col];
    update_movement(route_array[i],current_row,current_col);

    current_row_dir = current_row-previous_row;
    current_col_dir = current_col-previous_col;

    if(is_valid_direction(route_array[i]) == INVALID_DIRECTION)
      return ERROR_INVALID_DIRECTION;
    if(is_out_of_bounds(height,width,current_row,current_col))
      return ERROR_OUT_OF_BOUNDS;
    
    current_path_sym = map[current_row][current_col];
    
    if(map[current_row][current_col] == ' ')
      return ERROR_OFF_TRACK;
    if(map[previous_row][previous_col] != map[current_row][current_col] && 
        is_line(current_path_sym) && is_line(previous_path_sym)){
      return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
    } 
    if(i != 0 && !is_station(previous_path_sym))
      if((current_row_dir > 0 && previous_row_dir < 0) ||
          (current_row_dir < 0 && previous_row_dir > 0) ||
          (current_col_dir > 0 && previous_col_dir < 0) ||
          (current_col_dir < 0 && previous_col_dir > 0) ){
      return ERROR_BACKTRACKING_BETWEEN_STATIONS;
    }
    
    if(is_line(current_path_sym)){
      bool exist = false;
      if(strlen(lines_using) == 0)
        lines_using[0] = current_path_sym;
      else{
        for(int i = 0; i<lines_using[i] != '\0'; i++){
          if(lines_using[i] == current_path_sym){
            exist = true;
            break;
          }
        }
        if(!exist)
          lines_using[strlen(lines_using)] = current_path_sym;
      }  
    }
  }
    
    // Cechking last stop
    if(!is_station(current_path_sym))
      return ERROR_ROUTE_ENDPOINT_IS_NOT_STATION;
    else
      get_station_or_line_from_symbol(current_path_sym, destination);

    number_of_line_changes = strlen(lines_using) - 1;

    return number_of_line_changes;
}

bool is_valid_route(const char* route){

  char direction[512];
  int j  = 0;
  for(int i = 0; i < strlen(route); i++){
    if(route[i] != ','){
      direction[j] = route[i];
      j++;
    }
    else{
      direction[j] = '\0';
      if(is_valid_direction(direction) == INVALID_DIRECTION)
        return false;
      j = 0;
    } 
  }
  return true;
}

int number_of_stops_of_route(const char* route){
  
  char direction[512];
  int j  = 0;
  int number_of_stops = 0;
  for(int i = 0; i < strlen(route); i++){
    if(route[i] != ','){
      direction[j] = route[i];
      j++;
    }
    else{
      direction[j] = '\0';
      if(is_valid_direction(direction) == INVALID_DIRECTION)
        return -1;
      else
        number_of_stops++;
      j = 0;
    }
    
  }
  number_of_stops++; // Last stop 
  return number_of_stops;
}

Direction is_valid_direction(char* direction){

  if (strcmp(direction,"N")==0)
    return N;
  else if (strcmp(direction,"S")==0)
    return S;
  else if (strcmp(direction,"W")==0)
    return W;
  else if (strcmp(direction,"E")==0)
    return E;
  else if (strcmp(direction,"NE")==0)
    return NE;
  else if (strcmp(direction,"NW")==0)
    return NW;
  else if (strcmp(direction,"SE")==0)
    return SE;
  else if (strcmp(direction,"SW")==0)
    return NW;
  else
    return INVALID_DIRECTION;
}

void update_movement(char* direction, int &row, int &col){

  if (strcmp(direction,"N")==0){
    row--;
  }
  else if (strcmp(direction,"S")==0){
    row++;
  }  
  else if (strcmp(direction,"W")==0){
    col--;
  }
  else if (strcmp(direction,"E")==0){
    col++;
  }
  else if (strcmp(direction,"NE")==0){
    row--;
    col++;
  }
  else if (strcmp(direction,"NW")==0){
    row--;
    col--;
  }
  else if (strcmp(direction,"SE")==0){
    row++;
    col++;
  }
  else if (strcmp(direction,"SW")==0){
    row++;
    col--;
  }

}

bool is_out_of_bounds(int height, int width, int current_row, int current_col){
  if(current_row < 0 || current_row < 0 || 
      current_row >= height || current_row >= width)
      return true;
  return false;
}

bool is_station(char symbol){
  
  ifstream in;
  char reader[512];
  char station_symbol;
  in.open("stations.txt");
  if (!in)
    return ' ';
  while(!in.eof()){
    in.getline(reader,512);
    station_symbol = reader[0];
    if(symbol == station_symbol)
      return true;
  }
  in.close();

  return false;
}

bool is_line(char symbol){
  
  ifstream in;
  char reader[512];
  char line_symbol;
  in.open("lines.txt");
  if (!in)
    return ' ';
  while(!in.eof()){
    in.getline(reader,512);
    line_symbol = reader[0];
    if(symbol == line_symbol)
      return true;
  }
  in.close();

  return false;
}

void split_string(const char* input_string, const char* delimiters,
  char* output_array[]){
  
  char copied_input_string[512];
  int index = 0;
  strcpy(copied_input_string,input_string); //strtok modifies input string
                                            //copy is created

  // Extract the first token
  char* token = strtok(copied_input_string, delimiters);
  // Loop through the string to extract all other tokens
  while(token != '\0') {
    output_array[index] = new char[512];
    strcpy(output_array[index],token);
    token = strtok(NULL, delimiters);
    index++;
  }
  output_array[index] = NULL;
}
