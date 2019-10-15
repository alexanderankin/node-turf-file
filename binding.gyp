{
  "targets": [
    { 
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "include_dirs" : [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!(node -e \"require('nan')\")"
      ],
      "target_name": "turf_file",
      "sources": [ "turf_file.cc" ]
    }
  ]
}