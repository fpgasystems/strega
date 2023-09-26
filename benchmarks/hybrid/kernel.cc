/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include <ap_int.h>

void hybrid_static_pages (
  ap_uint<512>* storage_mem,
  ap_uint<512>* buffer_mem,
  unsigned int mode,
  unsigned int req_bytes,
  unsigned int req_pointer
) {
  if (mode == 1) {
    // load from buffer to storage
    for (int i=0; i < req_bytes/(512/8); i++) {
      storage_mem[req_pointer + i] = buffer_mem[i];
    }
  } else if (mode == 2) {
    // serve from storage to buffer
    for (int i=0; i < req_bytes/(512/8); i++) {
      buffer_mem[i] = storage_mem[req_pointer + i];
    }
  }
}