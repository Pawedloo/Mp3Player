# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Mp3PlayerQT_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Mp3PlayerQT_autogen.dir\\ParseCache.txt"
  "Mp3PlayerQT_autogen"
  )
endif()
