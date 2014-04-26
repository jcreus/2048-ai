#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;
typedef vector<vector<int> > tauler;

const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;

const int PASSOS = 7;

// la funcion principal es move(b, dir), el resto son funciones auxiliares

int fastlog(int i) {
if ( i==0 ) return 0;
if ( i==1 ) return 0;
if ( i==2 ) return 1;
if ( i==4 ) return 2;
if ( i==8 ) return 3;
if ( i==16 ) return 4;
if ( i==32 ) return 5;
if ( i==64 ) return 6;
if ( i==128 ) return 7;
if ( i==256 ) return 8;
if ( i==512 ) return 9;
if ( i==1024 ) return 10;
if ( i==2048 ) return 11;
if ( i==4096 ) return 12;
if ( i==8192 ) return 13;
if ( i==16384 ) return 14;
if ( i==32768 ) return 15;
if ( i==65536 ) return 16;
if ( i==131072 ) return 17;
if ( i==262144 ) return 18;
if ( i==524288 ) return 19;
if ( i==1048576 ) return 20;
if ( i==2097152 ) return 21;
if ( i==4194304 ) return 22;
if ( i==8388608 ) return 23;
if ( i==16777216 ) return 24;
if ( i==33554432 ) return 25;
if ( i==67108864 ) return 26;
if ( i==134217728 ) return 27;
if ( i==268435456 ) return 28;
if ( i==536870912 ) return 29;
if ( i==1073741824 ) return 30;
throw "dumbass";
}

void move_row_left(vector<int>& r) {
  for (int i = 0, j = 0; i < 4; ++i) {
    if (r[i] > 0) {
      if (j < i) {
        r[j] = r[i];
        r[i] = 0;
      }
      ++j;
    }
  }
  if (r[0] == r[1]) {
    r[0] *= 2;
    r[1] = r[2];
    r[2] = r[3];
    r[3] = 0;
  }
  if (r[1] == r[2]) {
    r[1] *= 2;
    r[2] = r[3];
    r[3] = 0;
  }
  if (r[2] == r[3]) {
    r[2] *= 2;
    r[3] = 0;
  }
}

void move_left(tauler& b) {
  for (int i = 0; i < 4; ++i) {
    move_row_left(b[i]);
  }
}

void rotate_clockwise(tauler& b) {
  tauler b2(4, vector<int>(4));
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      b2[i][j] = b[3-j][i];
    }
  }
  swap(b, b2);
}

void rotate_counterclockwise(tauler& b) {
  tauler b2(4, vector<int>(4));
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      b2[3-j][i] = b[i][j];
    }
  }
  swap(b, b2);
}

void horizontal_symmetry(tauler& b) {
  for (int i = 0; i < 4; ++i) {
    swap(b[i][0], b[i][3]);
    swap(b[i][1], b[i][2]);
  }
}

// mueve el tablero en la direccion dir, que tiene valor entero en [0, 3]
void move(tauler& b, int dir) {
  if (dir == UP) {
    rotate_counterclockwise(b);
    move_left(b);
    rotate_clockwise(b);
  }
  else if (dir == RIGHT) {
    horizontal_symmetry(b);
    move_left(b);
    horizontal_symmetry(b);
  }
  else if (dir == DOWN) {
    rotate_clockwise(b);
    move_left(b);
    rotate_counterclockwise(b);
  }
  else if (dir == LEFT) {
    move_left(b);
  }
}

const double SCORE_WEIGHT = 3.0;
const double EMPTY_WEIGHT = 15.0;
const double DENSITY_WEIGHT = 30.0;

double get_density(tauler& b) {
  int nx = 4, ny = 4;
  vector<int> files;
  vector<int> columnes;
  columnes.push_back(0);
  columnes.push_back(0);
  columnes.push_back(0);
  columnes.push_back(0);
  for (int i=0; i<4; i++) {
    int fila = 0;
    for (int j=0; j<4; j++) {
      columnes[j] += b[i][j];
      fila += b[i][j];
    }
    files.push_back(fila);
  }
  double meanX = 0;
  for (int i=0; i<4; i++) {
    meanX += i*files[i];
  }
  meanX /= (files[0]+files[1]+files[2]+files[3]);
  double meanY = 0;
  for (int i=0; i<4; i++) {
    meanY += i*columnes[i];
  }
  meanY /= (columnes[0]+columnes[1]+columnes[2]+columnes[3]);
  double d00 = meanX*meanX + meanY*meanY*2;
  return d00;
}

double get_score(tauler& b, int noves) {
  double score = 2*noves;
  int nempty = -noves;
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      if (b[i][j] == 0) nempty++;
      else score += b[i][j]*(fastlog(b[i][j])-1);
    }
  }
  if (nempty <= 0) {
     nempty = -10;
  }
  return SCORE_WEIGHT*score+EMPTY_WEIGHT*nempty+DENSITY_WEIGHT*1.0/get_density(b);
}

bool es_diferent(tauler& b1, tauler& b2) {
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      if (b1[i][j] != b2[i][j]) return true;
    }
  }
  return false;
}

pair<int, double> predict(tauler& b, int passos) {
  int directions[] = {UP, RIGHT, DOWN, LEFT};

  if (passos == 0) return pair<int, double>(0,get_score(b, 1));

  vector<double> scores;
  for (int i=0; i<4; i++) {
    tauler b1 = b;
    move(b1, directions[i]);
    //cout << es_diferent(b, b1) << endl;
    //cout << get_score(b1, 1) << endl;
    if (es_diferent(b, b1)) {
      scores.push_back(get_score(b1, PASSOS-passos+1)+predict(b1, passos-1).second/1.5);
    } else {
      scores.push_back(-10000000.0);
    }
    //cout << get_score(b1, 1) << endl;
  }

   int maxpos = distance(scores.begin(), max_element(scores.begin(), scores.end()));
   return pair<int, double>(maxpos, scores[maxpos]);


}

int main() {
  srand(time(NULL));
  
  tauler b(4, vector<int>(4));
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      cin >> b[i][j];
    }
  }
  /*int directions[] = {UP, RIGHT, DOWN, LEFT};
  vector<string> names;
  names.push_back("up");
  names.push_back("right");
  names.push_back("down");
  names.push_back("left");

  vector<double> scores;
  for (int i=0; i<4; i++) {
    tauler b1 = b;
    move(b1, directions[i]);
    //cout << es_diferent(b, b1) << endl;
    //cout << get_score(b1, 1) << endl;
    if (es_diferent(b, b1)) {
      scores.push_back(get_score(b1, 1));
    } else {
      scores.push_back(-1.0);
    }
    //cout << get_score(b1, 1) << endl;
  }

   int maxpos = distance(scores.begin(), max_element(scores.begin(), scores.end()));*/
  vector<string> names;
  names.push_back("up");
  names.push_back("right");
  names.push_back("down");
  names.push_back("left");
  int maxpos = predict(b, PASSOS).first;

  cout << names[maxpos] << endl;
  
  //sleep(1); // esto limita la velocidad del jugador a un movimiento por segundo
}
