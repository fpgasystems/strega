open_project fpga_http_server
open_solution "solution1" -flow_target vitis

set_part {xcu55c-fsvh2892-2L-e}
create_clock -period 10 -name default
set_top http_top

set command [lindex $argv 2]
set module [lindex $argv 3]

# source files
# add_files [glob hw/*.cc hw/*.h]
add_files "hw/hls/headline_parser.cc \
  hw/hls/http.cc \
  hw/hls/method_parser.cc \
  hw/hls/req_payload_parser.cc \
  hw/hls/request_processor.cc \
  hw/hls/response_aligner.cc \
  hw/hls/response_processor.cc \
  hw/hls/listen_port.cc \
  hw/hls/status_code_parser.cc" \
  -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"

# testbench file
add_files -tb hw/hls/$module.spec.cc -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"

switch $command {
  csim {
    csim_design -clean -O
  }
  synthesis {
    csynth_design
  }
  simulation {
    cosim_design -trace_level all -O
  }
  hw {
    export_design \
        -format xo \
        -display_name "HTTP Server" \
        -description "Replies to ARP queries and resolves IP addresses." \
        -vendor "ethz.systems.fpga" \
        -library "http-server" \
        -ipname "http" \
        -version "1.1" \
        -output "ips/http.xo"
  }
  default {
    puts "Invalid command specified. Use vivado_hls -f make.tcl <csim|synthesis|simulation|hw>."
  }
}
exit
