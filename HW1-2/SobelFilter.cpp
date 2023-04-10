#include <cmath>

#include "SobelFilter.h"
using namespace std;
const int width = 256;
const int height = 256;

SobelFilter::SobelFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// mask
const int mask[3][3] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};

void SobelFilter::do_filter() {
  //初始化存的矩陣
  unsigned char** median_bitmap = new unsigned char*[height];
  unsigned char** mean_bitmap = new unsigned char*[height];
  for (int y = 0; y < height; y++) {
    median_bitmap[y] = new unsigned char[width];
    mean_bitmap[y] = new unsigned char[width];
    for (int x = 0; x < width; x++) {
      median_bitmap[y][x] = 0;
      mean_bitmap[y][x] = 0;
    }
  }

  cout << "median filter" << endl;
  int count = 0;
  for(int y = 0; y < height; y++) {
    for(int i = 0; i < 9; i++){
      median[i] = (i_r.read() + i_g.read() + i_b.read())/3;
      count++;
    }
    sort(median, median + 9);
    median_bitmap[y][0] = median[4];
    int median_count = 0;
    for(int x = 1; x < width; x++) {
      for(int v =0;v < 3;v++){
        median[median_count % 9] = (i_r.read() + i_g.read() + i_b.read())/3;
        median_count++;
        count++;
      }
      //cout << "y & x = " << y << "," << x << endl;
      sort(median, median + 9);
      median_bitmap[y][x] = median[4];
      //cout << median_bitmap[y][x] << "," << median[4] << " & count = " << count << endl;
    }
  }

  cout << "mean filter" << endl;
  for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
      int sum = 0;
      //cout << "-------------------y & x = " << y << "," << x << endl;
      for (int v_offset = -1; v_offset <= 1; v_offset++) {
        for (int u_offset = -1; u_offset <= 1; u_offset++) {
          int yy = y + v_offset;
          int xx = x + u_offset;
          //cout << "yy & xx = " << yy << "," << xx << endl;
          if (yy >= 0 && yy < height && xx >= 0 && xx < width) {
            sum += median_bitmap[yy][xx] * mask[v_offset+1][u_offset+1];            
          }
        }
      }
      //count++;
      mean_bitmap[y][x] = (unsigned char)(sum / 10);
      //cout << "mean_bitmap[y][x] = " << mean_bitmap[y][x] << endl;
    }
  }
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      o_result.write(median_bitmap[y][x]);
    }
  }
  cout << "count = " << count << endl;
  // 釋放 median_bitmap 的記憶體空間 
  for (int i = 0; i < height; i++) {
    delete[] median_bitmap[i];
    delete[] mean_bitmap[i];
  }
  delete[] median_bitmap;
  delete[] mean_bitmap;


  //cout << "????????????" << endl;
  wait(10);   
 
}
