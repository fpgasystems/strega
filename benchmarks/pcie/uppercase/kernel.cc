#include <stdint.h>
#include <hls_stream.h>

#define TRIPCOUNT_SIZE 4096
const int c_tripcount = TRIPCOUNT_SIZE;

static void load_input(char* in, hls::stream<char>& inStream, int size) {
mem_rd:
    for (int i = 0; i < size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = c_tripcount max = c_tripcount
        inStream << in[i];
    }
}

static void compute_add(hls::stream<char>& in1_stream,
                        hls::stream<char>& out_stream,
                        int size) {
// The kernel is operating with vector of NUM_WORDS integers. The + operator performs
// an element-wise add, resulting in NUM_WORDS parallel additions.
execute:
    for (int i = 0; i < size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = c_tripcount max = c_tripcount
        char aux = in1_stream.read();
        out_stream << (('a' <= aux && aux <= 'z') ? (aux - ('a'-'A')) : aux);
    }
}

static void store_result(char* out, hls::stream<char>& out_stream, int size) {
mem_wr:
    for (int i = 0; i < size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = c_tripcount max = c_tripcount
        out[i] = out_stream.read();
    }
}

extern "C" {

void uppercase(char* input, char* output, int size) {
#pragma HLS INTERFACE m_axi port = input bundle = gmem0
#pragma HLS INTERFACE m_axi port = output bundle = gmem0

    static hls::stream<char> in1_stream("input_stream_1");
    static hls::stream<char> out_stream("output_stream");

#pragma HLS dataflow
    load_input(input, in1_stream, size);
    compute_add(in1_stream, out_stream, size);
    store_result(output, out_stream, size);
}
}
