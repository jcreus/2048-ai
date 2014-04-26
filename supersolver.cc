#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <map>
#include <vector>

#define debug(x) cerr << #x << " = " << x << endl

using namespace std;

typedef unsigned long long ull;
typedef unsigned short us;
typedef vector<int> VI;
typedef vector<VI> V2I;
typedef vector<ull> VL;
typedef vector<us> VS;

const int MAX_DEPTH = 4;
const int MAX_CACHE_DEPTH = 4;
const double CPROB_THRESHOLD = 1e-5;
const ull ROW_MASK = 0x000000000000ffffULL;
const ull COL_MASK = 0x000f000f000f000fULL;

map<ull, double> mem;
VL col_up, row_right, col_down, row_left;
VI line_heur;

ull encode(const V2I& d) {
  ull e = 0;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (d[i][j] >= 16) {
        cerr << "Tile too large: " << d[i][j] << endl;
      }
      e |= ((ull(d[i][j])&0xf)<<((i<<4)+4*j));
    }
  }
  return e;
}

V2I decode(ull e) {
  V2I d(4, VI(4));
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      d[i][j] = (e&0xf);
      e >>= 4;
    }
  }
  return d;
}

inline int tile_value(int k) {
  if (k == 0) return 0;
  return (1<<k);
}

int tile_kind(int v) {
  if (v < 2) return 0;
  int k = 1;
  for (; v > 2; v >>= 1) ++k;
  return k;
}


// expectimax

inline ull expand_col(us c) {
  ull lc = c;
  return (lc|(lc<<12)|(lc<<24)|(lc<<36))&COL_MASK;
}

inline us compress_col(ull c) {
  return us(c|(c>>12)|(c>>24)|(c>>36));
}

inline ull move_up(ull e) {
  ull e2 = 0;
  for (int i = 0; i < 4; ++i) {
    us r = compress_col((e>>(i<<2))&COL_MASK);
    e2 |= (col_up[r]<<(i<<2));
  }
  return e2;
}

inline ull move_right(ull e) {
  ull e2 = 0;
  for (int i = 0; i < 4; ++i) {
    us r = us(e>>(i<<4));
    e2 |= (row_right[r]<<(i<<4));
  }
  return e2;
}

inline ull move_down(ull e) {
  ull e2 = 0;
  for (int i = 0; i < 4; ++i) {
    us r = compress_col((e>>(i<<2))&COL_MASK);
    e2 |= (col_down[r]<<(i<<2));
  }
  return e2;
}

inline ull move_left(ull e) {
  ull e2 = 0;
  for (int i = 0; i < 4; ++i) {
    us r = us(e>>(i<<4));
    e2 |= (row_left[r]<<(i<<4));
  }
  return e2;
}

inline ull move(ull e, int dir) {
  if (dir == 0) {
    return move_up(e);
  }
  if (dir == 1) {
    return move_right(e);
  }
  if (dir == 2) {
    return move_down(e);
  }
  if (dir == 3) {
    return move_left(e);
  }
  cerr << "Invalid move" << endl;
  return e;
}

double heuristic_value(ull e) {
  int v = 100;
  for (int i = 0; i < 4; ++i) {
    v += line_heur[(e>>(i<<4))&ROW_MASK];
    v += line_heur[compress_col((e>>(i<<2))&COL_MASK)];
  }
  return double(v);
}

pair<double, int> expectimax_player(ull e, int depth = 0, double cprob = 1.0);

double expectimax_game(ull e, int depth, double cprob) {
  int free = 0;
  for (int k = 0; k < 16; ++k) {
    if (((e>>(k<<2))&0xf) == 0) ++free;
  }
  cprob /= double(free);
  
  double sum = 0.0;
  for (int k = 0; k < 16; ++k) {
    if (((e>>(k<<2))&0xf) == 0) {
      sum += 0.9*expectimax_player(e|(1ULL<<(k<<2)), depth+1, cprob*0.9).first;
      sum += 0.1*expectimax_player(e|(2ULL<<(k<<2)), depth+1, cprob*0.9).first;
    }
  }
  return sum/double(free);
}

pair<double, int> expectimax_player(ull e, int depth, double cprob) {
  if (depth >= MAX_DEPTH || cprob < CPROB_THRESHOLD) {
    return pair<double, int>(heuristic_value(e), -1);
  }
  
  if (depth < MAX_CACHE_DEPTH) {
    map<ull, double>::iterator it = mem.find(e);
    if (it != mem.end()) {
      return pair<double, int>(it->second, -1);
    }
  }
  
  pair<double, int> ret(0.0, -1);
  for (int dir = 0; dir < 4; ++dir) {
    ull e2 = move(e, dir);
    if (e2 == e) continue;
    double value = expectimax_game(e2, depth, cprob);
    if (ret.second == -1 || value > ret.first) {
      ret.first = value;
      ret.second = dir;
    }
  }
  
  if (depth < MAX_CACHE_DEPTH) {
    mem[e] = ret.first;
  }
  
  return ret;
}


// precomputation

V2I move_decoded(V2I b, int dir) {
  if (dir < 0 || dir > 3) {
    cerr << "Invalid decoded move" << endl;
    return b;
  }
  
  const int N = 4;
  const int dy[] = {-1, 0, 1, 0};
  const int dx[] = {0, 1, 0, -1};
  
  for (int i = 0; i < N; ++i) {
    pair<int, int> free = pair<int, int>(-1, -1);
    for (int j = 0; j < N; ++j) {
      int y, x;
      if (dir == 0) {
        y = j;
        x = i;
      }
      else if (dir == 1) {
        y = i;
        x = N-1-j;
      }
      else if (dir == 2) {
        y = N-1-j;
        x = N-1-i;
      }
      else {
        y = N-1-i;
        x = j;
      }
      
      int yn = y-dy[dir], xn = x-dx[dir];
      // look for next tile
      for (; yn >= 0 && yn < N && xn >= 0 && xn < N && b[yn][xn] == 0;) {
        yn -= dy[dir];
        xn -= dx[dir];
      }
      if (yn >= 0 && yn < N && xn >= 0 && xn < N) {
        if (b[y][x] == 0) {
          b[y][x] = b[yn][xn];
          b[yn][xn] = 0;
          
          // look for a second tile
          yn -= dy[dir];
          xn -= dx[dir];
          for (; yn >= 0 && yn < N && xn >= 0 && xn < N && b[yn][xn] == 0;) {
            yn -= dy[dir];
            xn -= dx[dir];
          }
          if (yn >= 0 && yn < N && xn >= 0 && xn < N) {
            if (b[yn][xn] == b[y][x]) {
              ++b[y][x];
              b[yn][xn] = 0;
            }
          }
        }
        else if (b[yn][xn] == b[y][x]) {
          ++b[y][x];
          b[yn][xn] = 0;
        }
      }
    }
  }
  
  return b;
}

void precompute(string file_name = "") {
  col_up = VL(1<<16);
  row_right = VL(1<<16);
  col_down = VL(1<<16);
  row_left = VL(1<<16);
  line_heur = VI(1<<16);
  
  // read from file if provided
  if (file_name != "") {
    ifstream fin(file_name.c_str());
    for (int i = 0; i < (1<<16); ++i) {
      fin >> col_up[i] >> row_right[i] >> col_down[i] >> row_left[i] >> line_heur[i];
    }
    return;
  }
  
  // actually precompute otherwise
  for (ull e = 0; e < (1<<16); ++e) {
    V2I dh = decode(e);
    V2I dv = decode(expand_col(us(e)));
    
    // move up
    V2I du = move_decoded(dv, 0);
    bool du_ok = true;
    for (int i = 0; i < 4; ++i) {
      if (du[i][0] >= 16) du_ok = false;
    }
    if (du_ok) col_up[e] = encode(du);
    
    // move right
    V2I dr = move_decoded(dh, 1);
    bool dr_ok = true;
    for (int i = 0; i < 4; ++i) {
      if (dr[0][i] >= 16) dr_ok = false;
    }
    if (dr_ok) row_right[e] = encode(dr);
    
    // move down
    V2I dd = move_decoded(dv, 2);
    bool dd_ok = true;
    for (int i = 0; i < 4; ++i) {
      if (dd[i][0] >= 16) dd_ok = false;
    }
    if (dd_ok) col_down[e] = encode(dd);
    
    // move left
    V2I dl = move_decoded(dh, 3);
    bool dl_ok = true;
    for (int i = 0; i < 4; ++i) {
      if (dr[0][i] >= 16) dl_ok = false;
    }
    if (dl_ok) row_left[e] = encode(dl);
    
    // heuristic value
    V2I d = decode(e);
    line_heur[e] = 0;
    
    bool nonincr = true, nondecr = true;
    for (int i = 0; i < 3; ++i) {
      if (d[0][i] < d[0][i+1]) nonincr = false;
      if (d[0][i] > d[0][i+1]) nondecr = false;
    }
    if (nonincr || nondecr) line_heur[e] += 10;
    
    for (int i = 0; i < 4; ++i) {
      if (d[0][i] == 0) {
        line_heur[e] += 10;
      }
    }
    
    for (int i = 0; i < 3; ++i) {
      if (abs(d[0][i]-d[0][i+1]) == 1) {
        ++line_heur[e];
      }
    }
    
    int mxind = 0;
    for (int i = 0; i < 4; ++i) {
      if (d[0][i] > d[0][mxind]) {
        mxind = i;
      }
    }
    if (mxind == 0 || mxind == 3) {
      line_heur[e] += 20;
    }
  }
}

int main() {
  ios_base::sync_with_stdio(false);
  precompute();
  
  V2I d(4, VI(4));
  for (int v; cin >> v;) {
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        if (i > 0 || j > 0) cin >> v;
        d[i][j] = tile_kind(v);
      }
    }
    mem.clear();
    int dir = expectimax_player(encode(d)).second;
    if (dir == 0) cout << "up" << endl;
    else if (dir == 1) cout << "right" << endl;
    else if (dir == 2) cout << "down" << endl;
    else if (dir == 3) cout << "left" << endl;
    else cout << "error" << endl;
    //sleep(1);
  }
}

