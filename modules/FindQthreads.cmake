
find_library(qthreads_lib_found qthread PATHS ${Qthreads_ROOT} SUFFIXES lib lib64 NO_DEFAULT_PATHS)
find_path(qthreads_headers_found qthread.h PATHS ${Qthreads_ROOT}/include NO_DEFAULT_PATHS)

if (qthreads_lib_found AND qthreads_headers_found)
message(STATUS "Qthreads found: ${qthreads_lib_found}")
set_target_properties(Qthreads PROPERTIES
    INTERFACE_LINK_LIBRARIES ${qthreads_lib_found}
    INTERFACE_INCLUDE_DIRECTORIES ${qthreads_headers_found}
  )
else()
  message(WARNING "Qthreads not found")
endif()
