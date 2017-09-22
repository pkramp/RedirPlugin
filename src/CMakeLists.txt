if(NOT DEFINED ENV{XROOTD_PATH})
message(FATAL_ERROR "XROOTD_PATH environment variable should point to your XRootD install dir")
else()
SET(XROOTD_PATH $ENV{XROOTD_PATH})
endif()

include_directories( ${XROOTD_PATH}/include/xrootd )
add_library( RedirPlugin SHARED
             RedirPlugin.cc RedirPlugin.hh )


add_definitions( -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 )
link_directories( ${XROOTD_PATH}/lib )
find_library(XLIBS XrdCl ${XROOTD_PATH}/lib)
message(STATUS "XrdCl is ${XLIBS}" )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    target_link_libraries( RedirPlugin ${XLIBS})
    if( LINUX )

    set_target_properties( RedirPlugin PROPERTIES
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        SOVERSION ${VERSION_MAJOR}
        CLEAN_DIRECT_OUTPUT 1
    )
  endif( LINUX )

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -Wall -g -O2 -Werror ")
    SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -Wall -g -O2 -Werror ")
    INSTALL( TARGETS RedirPlugin LIBRARY DESTINATION lib/${PROJECT_NAME} )
