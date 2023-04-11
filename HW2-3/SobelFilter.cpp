#include <cmath>
#include <iomanip>

#include "SobelFilter.h"

using namespace std;
const int width = 512;
const int height = 512;

SobelFilter::SobelFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &SobelFilter::blocking_transport);
}

// mask
const int mask[3][3] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};

void SobelFilter::do_filter() {
  //int count = 0;
  for(int y = 0; y < height; y++) {
    int median_count = 0;
    for(int x = 0; x < width; x++) {
      //cout << "x,y = " << x << " , " << y << endl;
      if(x == 0){
        for(int z = 0;z < 9; z++){
          median[z] = (i_r.read() + i_g.read() + i_b.read())/3;
          //count++;
          //cout << "median = " << median[z] << " & count = " << count << endl;
        }
          sort(median, median + 9);
          int temp = median[4];
          //cout << "median = " << median[4] << " & temp = " << temp << endl;
          median_bitmap[y][x] = temp;
          //cout << median_bitmap[y][x] << "," << median[4] << " & count = " << count << endl;
      } else {
          for(int z = 0;z < 3; z++){
            median[median_count % 9] = (i_r.read() + i_g.read() + i_b.read())/3;
            //count++;
            median_count++;
            //cout << "median = " << median[z] << " & count = " << count << endl;
          }
          sort(median, median + 9);
          int temp = median[4];
          median_bitmap[y][x] = temp;
          //cout << median_bitmap[y][x] << "," << median[4] << " & count = " << count << endl;
      }
    }
  }

  cout << "mean filter" << endl;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int sum = 0;
      //cout << "-------------------y & x = " << y << "," << x << endl;
      for (int v = -1; v <= 1; v++) {
        for (int u = -1; u <= 1; u++) {
          int yy = y + v;
          int xx = x + u;
          if (yy >= 0 && yy < height && xx >= 0 && xx < width) {
            sum += median_bitmap[yy][xx] * mask[v+1][u+1];
          } 
          //cout << "yy & xx = " << yy << "," << xx << endl;      
        }
      }
      //count++;
      mean_bitmap[y][x] = (sum / 10);
      o_result.write(mean_bitmap[y][x]);
      //cout << "mean_bitmap[y][x] = " << mean_bitmap[y][x] << endl;
    }
  }

  //cout << "count = " << count << endl;

  wait(10);   
  
}

void SobelFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr -= base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case SOBEL_FILTER_RESULT_ADDR:
      buffer.uint = o_result.read();
      break;
    case SOBEL_FILTER_CHECK_ADDR:
      buffer.uint = o_result.num_available();
    break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;
  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case SOBEL_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      break;
      default:
        std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                  << std::setfill('0') << std::setw(8) << std::hex << addr
                  << std::dec << " is not valid" << std::endl;
      break;
    }
    break;
  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}