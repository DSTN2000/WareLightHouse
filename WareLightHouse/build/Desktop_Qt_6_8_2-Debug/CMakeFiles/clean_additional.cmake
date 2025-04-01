# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/WareWatchHouse_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/WareWatchHouse_autogen.dir/ParseCache.txt"
  "WareWatchHouse_autogen"
  )
endif()
