/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#include "response_aligner.h"

namespace http {

ap_uint<64> lenToKeep(ap_uint<6> length) {
  #pragma HLS INLINE

  switch (length) {
    case 1:
      return 0x1;
    case 2:
      return 0x3;
    case 3:
      return 0x7;
    case 4:
      return 0xF;

    case 5:
      return 0x1F;
    case 6:
      return 0x3F;
    case 7:
      return 0x7F;
    case 8:
      return 0xFF;

    case 9:
      return 0x1FF;
    case 10:
      return 0x3FF;
    case 11:
      return 0x7FF;
    case 12:
      return 0xFFF;

    case 13:
      return 0x1FFF;
    case 14:
      return 0x3FFF;
    case 15:
      return 0x7FFF;
    case 16:
      return 0xFFFF;

    case 17:
      return 0x1FFFF;
    case 18:
      return 0x3FFFF;
    case 19:
      return 0x7FFFF;
    case 20:
      return 0xFFFFF;

    case 21:
      return 0x1FFFFF;
    case 22:
      return 0x3FFFFF;
    case 23:
      return 0x7FFFFF;
    case 24:
      return 0xFFFFFF;

    case 25:
      return 0x1FFFFFF;
    case 26:
      return 0x3FFFFFF;
    case 27:
      return 0x7FFFFFF;
    case 28:
      return 0xFFFFFFF;

    case 29:
      return 0x1FFFFFFF;
    case 30:
      return 0x3FFFFFFF;
    case 31:
      return 0x7FFFFFFF;
    case 32:
      return 0xFFFFFFFF;

    case 33:
      return 0x1FFFFFFFF;
    case 34:
      return 0x3FFFFFFFF;
    case 35:
      return 0x7FFFFFFFF;
    case 36:
      return 0xFFFFFFFFF;

    case 37:
      return 0x1FFFFFFFFF;
    case 38:
      return 0x3FFFFFFFFF;
    case 39:
      return 0x7FFFFFFFFF;
    case 40:
      return 0xFFFFFFFFFF;

    case 41:
      return 0x1FFFFFFFFFF;
    case 42:
      return 0x3FFFFFFFFFF;
    case 43:
      return 0x7FFFFFFFFFF;
    case 44:
      return 0xFFFFFFFFFFF;

    case 45:
      return 0x1FFFFFFFFFFF;
    case 46:
      return 0x3FFFFFFFFFFF;
    case 47:
      return 0x7FFFFFFFFFFF;
    case 48:
      return 0xFFFFFFFFFFFF;

    case 49:
      return 0x1FFFFFFFFFFFF;
    case 50:
      return 0x3FFFFFFFFFFFF;
    case 51:
      return 0x7FFFFFFFFFFFF;
    case 52:
      return 0xFFFFFFFFFFFFF;

    case 53:
      return 0x1FFFFFFFFFFFFF;
    case 54:
      return 0x3FFFFFFFFFFFFF;
    case 55:
      return 0x7FFFFFFFFFFFFF;
    case 56:
      return 0xFFFFFFFFFFFFFF;

    case 57:
      return 0x1FFFFFFFFFFFFFF;
    case 58:
      return 0x3FFFFFFFFFFFFFF;
    case 59:
      return 0x7FFFFFFFFFFFFFF;
    case 60:
      return 0xFFFFFFFFFFFFFFF;

    case 61:
      return 0x1FFFFFFFFFFFFFFF;
    case 62:
      return 0x3FFFFFFFFFFFFFFF;
    case 63:
      return 0x7FFFFFFFFFFFFFFF;
    default:
      return 0xFFFFFFFFFFFFFFFF;

	}
}

ap_uint<8> keepToLen(ap_uint<64> keepValue) {
  #pragma HLS INLINE

  switch (keepValue) {
    case 0x1:
      return 0x1;
    case 0x3:
      return 0x2;
    case 0x7:
      return 0x3;
    case 0xF:
      return 0x4;
      
    case 0x1F:
      return 0x5;
    case 0x3F:
      return 0x6;
    case 0x7F:
      return 0x7;
    case 0xFF:
      return 0x8;
      
    case 0x1FF:
      return 0x9;
    case 0x3FF:
      return 0xA;
    case 0x7FF:
      return 0xB;
    case 0xFFF:
      return 0xC;
      
    case 0x1FFF:
      return 0xD;
    case 0x3FFF:
      return 0xE;
    case 0x7FFF:
      return 0xF;

    case 0xFFFF:
      return 0x10;
      
    case 0x1FFFF:
      return 0x11;
    case 0x3FFFF:
      return 0x12;
    case 0x7FFFF:
      return 0x13;
    case 0xFFFFF:
      return 0x14;
      
    case 0x1FFFFF:
      return 0x15;
    case 0x3FFFFF:
      return 0x16;
    case 0x7FFFFF:
      return 0x17;
    case 0xFFFFFF:
      return 0x18;
      
    case 0x1FFFFFF:
      return 0x19;
    case 0x3FFFFFF:
      return 0x1A;
    case 0x7FFFFFF:
      return 0x1B;
    case 0xFFFFFFF:
      return 0x1C;
      
    case 0x1FFFFFFF:
      return 0x1D;
    case 0x3FFFFFFF:
      return 0x1E;
    case 0x7FFFFFFF:
      return 0x1F;
    case 0xFFFFFFFF:
      return 0x20;
      
    case 0x1FFFFFFFF:
      return 0x21;
    case 0x3FFFFFFFF:
      return 0x22;
    case 0x7FFFFFFFF:
      return 0x23;
    case 0xFFFFFFFFF:
      return 0x24;
      
    case 0x1FFFFFFFFF:
      return 0x25;
    case 0x3FFFFFFFFF:
      return 0x26;
    case 0x7FFFFFFFFF:
      return 0x27;
    case 0xFFFFFFFFFF:
      return 0x28;
      
    case 0x1FFFFFFFFFF:
      return 0x29;
    case 0x3FFFFFFFFFF:
      return 0x2A;
    case 0x7FFFFFFFFFF:
      return 0x2B;
    case 0xFFFFFFFFFFF:
      return 0x2C;
      
    case 0x1FFFFFFFFFFF:
      return 0x2D;
    case 0x3FFFFFFFFFFF:
      return 0x2E;
    case 0x7FFFFFFFFFFF:
      return 0x2F;
    case 0xFFFFFFFFFFFF:
      return 0x30;
      
    case 0x1FFFFFFFFFFFF:
      return 0x31;
    case 0x3FFFFFFFFFFFF:
      return 0x32;
    case 0x7FFFFFFFFFFFF:
      return 0x33;
    case 0xFFFFFFFFFFFFF:
      return 0x34;
      
    case 0x1FFFFFFFFFFFFF:
      return 0x35;
    case 0x3FFFFFFFFFFFFF:
      return 0x36;
    case 0x7FFFFFFFFFFFFF:
      return 0x37;
    case 0xFFFFFFFFFFFFFF:
      return 0x38;
      
    case 0x1FFFFFFFFFFFFFF:
      return 0x39;
    case 0x3FFFFFFFFFFFFFF:
      return 0x3A;
    case 0x7FFFFFFFFFFFFFF:
      return 0x3B;
    case 0xFFFFFFFFFFFFFFF:
      return 0x3C;
      
    case 0x1FFFFFFFFFFFFFFF:
      return 0x3D;
    case 0x3FFFFFFFFFFFFFFF:
      return 0x3E;
    case 0x7FFFFFFFFFFFFFFF:
      return 0x3F;
    case 0xFFFFFFFFFFFFFFFF:
      return 0x40;
      
    default:
      return 0;
  }
}

enum class fsm_state {
  IDLE,
  PULL_FIRST,
  PULL_N,
  RESIDUE
};

axi_stream_ispt alignWords(ap_uint<6> prevLength, axi_stream_ispt prevWord, axi_stream_ispt currWord)
{
  const unsigned int WIDTH = 512;
  axi_stream_ispt alignedWord;

  alignedWord.data(WIDTH - 1, prevLength*8) = currWord.data(WIDTH - prevLength*8 - 1, 0);
  alignedWord.keep(WIDTH/8 - 1, prevLength) = currWord.keep(WIDTH/8 - prevLength - 1, 0);
  alignedWord.data(prevLength*8 - 1, 0) = prevWord.data(prevLength*8 - 1, 0);
  alignedWord.keep(prevLength - 1, 0)  = prevWord.keep(prevLength - 1, 0);

  return alignedWord;
}

axi_stream_ispt shiftDown(ap_uint<6> offset, axi_stream_ispt currWord)
{
  const unsigned int WIDTH = 512;
  axi_stream_ispt alignedWord;

  alignedWord.data(WIDTH - (offset*8) - 1, 0) = currWord.data(WIDTH-1, offset*8);
  alignedWord.keep(WIDTH/8 - offset - 1, 0)  = currWord.keep(WIDTH/8-1, offset);
  alignedWord.keep(WIDTH/8-1, WIDTH/8 - offset) = 0;

  return alignedWord;
}

void response_aligner (
  hls::stream<axi_stream_ispt>& tx_aligner,
  hls::stream<pkt512>& tcp_tx_data
) {
  #pragma HLS PIPELINE II=1
	#pragma HLS INLINE off

  const unsigned int BUFFER = 512/8;
  static fsm_state state = fsm_state::IDLE;
  static ap_uint<8> prevLength;
	static axi_stream_ispt prevWord;

  #pragma HLS reset variable=state
  #pragma HLS reset variable=prevLength off
  #pragma HLS reset variable=prevWord off

  switch (state) {
    case fsm_state::IDLE: {
      state = (tx_aligner.empty()) ? fsm_state::IDLE : fsm_state::PULL_FIRST;
      break;
    }

    case fsm_state::PULL_FIRST: {
      tx_aligner.read(prevWord);

      prevLength = keepToLen(prevWord.keep);

      bool isLast = prevWord.last;
      bool isFull = prevLength == BUFFER;

      if (isLast || isFull) {
        state = fsm_state::RESIDUE;
      } else {
        state = fsm_state::PULL_N;
      }
      
      break;
    }

    case fsm_state::PULL_N: {
      axi_stream_ispt currWord;
      tx_aligner.read(currWord);

      auto sendWord = alignWords(prevLength, prevWord, currWord);

      auto currLength = keepToLen(currWord.keep);
      auto sendLength = keepToLen(sendWord.keep);

      bool isLast = currWord.last;
      bool isFull = sendLength == BUFFER;
      bool hasResidue = (currLength + prevLength) > BUFFER;

      // last decision
      sendWord.last = (isLast && !hasResidue);
      prevWord.last = (isLast && hasResidue);

      // push decision
      if (isLast || isFull || hasResidue) {
        tcp_tx_data.write(sendWord.serialise());
      }

      // buffer decision
      if (isFull) {
        prevWord = shiftDown(prevLength, currWord);
        prevLength = prevLength + currLength - BUFFER;
      } else {
        prevWord = sendWord;
        prevLength = sendLength;
      }

      // state decision
      if (isLast) {
        state = (hasResidue) ? fsm_state::RESIDUE : fsm_state::IDLE;
      } else {
        state = fsm_state::PULL_N;
      }

      break;
    }

    case fsm_state::RESIDUE: {
      tcp_tx_data.write(prevWord.serialise());
      state = fsm_state::IDLE;
      break;
    }
  }
}

} // namespace http