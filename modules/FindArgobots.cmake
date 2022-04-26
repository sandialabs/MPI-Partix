
find_library(abt_lib_found abt PATHS ${Argobots_ROOT} SUFFIXES lib lib64 NO_DEFAULT_PATHS)
find_path(abt_headers_found abt.h PATHS ${Argobots_ROOT}/include NO_DEFAULT_PATHS)

if (abt_lib_found AND abt_headers_found)
message(STATUS "Argobots found: ${abt_lib_found}")
set_target_properties(Argobots PROPERTIES
    INTERFACE_LINK_LIBRARIES ${abt_lib_found}
    INTERFACE_INCLUDE_DIRECTORIES ${abt_headers_found}
  )
else()
  message(WARNING "Argobots not found")
endif()
