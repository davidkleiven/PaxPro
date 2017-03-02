cmake_minimum_required (VERSION 2.6)

project( ParaxialSimulationLibrary )
set( CMAKE_CXX_STANDARD 11 )
#add_definitions( -DARMA_NO_DEBUG )
add_definitions( -O2 )
set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fopenmp")

# Add libraries
include_directories( include )
include_directories(/usr/include/jsoncpp)
include_directories( /usr/local/include )
include_directories( /usr/lib/x86_64-linux-gnu/hdf5/serial/include )

set( INSTALL_INC_DIR /usr/local/include )
set( INSTALL_LIB_DIR /usr/local/lib )

link_directories( /usr/local/lib )
link_directories( /usr/lib/x86_64-linux-gnu )
set( CMAKE_LIBRARY_PATH /usr/lib/x86_64-linux-gnu )

# Find required packages
find_package( LAPACK REQUIRED )
include_directories(${LAPACK_INCLUDE_DIR})
set(LIB ${LAPACK_LIBRARIES})

find_package( BLAS REQUIRED )
include_directories(${BLAS_INCLUDE_DIR})
set(LIB ${LIB} ${BLAS_LIBRARIES})

find_package( GSL REQUIRED )
include_directories(${GSL_INCLUDE_DIR})
set(LIB ${LIB} ${GSL_LIBRARIES})

set(CMAKE_INCLUDE_PATH "/usr/include/jsoncpp")
find_package( jsoncpp REQUIRED )
include_directories(${JSONCPP_INCLUDE_DIR})
set(LIB ${LIB} ${JSONCPP_LIBRARIES})

# TODO: Seem to be something wrong witht the package detection
find_library( JSONCPP_LIB jsoncpp )
set( LIB ${LIB} jsoncpp.so)
if ( NOT JSONCPP_LIB )
  MESSAGE( FATAL_ERROR "Did not find the jsoncpp library!")
endif( NOT JSONCPP_LIB)
find_path( JSON_HEADER json)
if ( NOT JSON_HEADER )
  MESSAGE( FATAL_ERROR "Did not find the json headers!")
endif( NOT JSON_HEADER )

find_package( ZLIB REQUIRED )
include_directories(${ZLIB_INCLUDE_DIR})
set(LIB ${LIB} ${ZLIB_LIBRARIES})

find_package( HDF5 REQUIRED COMPONENTS C CXX HL)
include_directories( ${HDF5_INCLUDE_DIRS} )

MESSAGE( STATUS ${HDF5_INCLUDE_DIRS} )
set(LIB ${LIB} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_CXX_LIBRARIES})

#set(CMAKE_INCLUDE_PATH "/usr/include")
find_package( Armadillo REQUIRED )
include_directories(${ARMADILLO_INCLUDE_DIRS})
set(LIB ${LIB} ${ARMADILLO_LIBRARIES})

find_package( SFML REQUIRED )
include_directories(${SFML_INCLUDE_DIR})
set(LIB ${LIB} libsfml-window.so libsfml-graphics.so libsfml-system.so)

# Check for the VISA library
find_library( VISA_LIB visa HINTS /usr/local/lib )
if ( NOT VISA_LIB )
  MESSAGE (FATAL_ERROR "Did not find the VISA library")
endif ( NOT VISA_LIB )
set( LIB ${LIB} ${VISA_LIB} )

find_path( VISA_HEADER visa/ )
if ( NOT VISA_HEADER )
  MESSAGE(FATAL_ERROR "Did not find the VISA header files!")
endif( NOT VISA_HEADER )

# Find pthreads library
find_package( OpenMP )
if ( OpenMP_found )
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
endif()

find_package( OpenGL REQUIRED )

include_directories( FLTK_INCLUDE_DIR )
set( LIB ${LIB} ${FLTK_BASE_LIBRARY} )
set( LIB ${LIB} ${FLTK_GL_LIBRARY} )
set( LIB ${LIB} ${FLTK_FORMS_LIBRARY} )

# Find the FFTW3 library with threads
find_library( FFTW_OMP_LIB fftw3_omp )
if ( NOT FFTW_OMP_LIB )
  MESSAGE( FATAL_ERROR "Could not find the FFTW3 openMP library!")
endif()
set( LIB ${LIB} ${FFTW_OMP_LIB})

# Find the FFTW3 library
find_library( FFTW_LIB fftw3 )
if ( NOT FFTW_LIB )
  MESSAGE( FATAL_ERROR "Could not find the FFTW library")
endif()
find_path( FFTW_INC fftw3.h)
if ( NOT FFTW_INC )
  MESSAGE( FATAL_ERROR "Could not find the FFTW3 headers!")
endif()
set( LIB ${LIB} ${FFTW_LIB} )
set( LIB ${LIB} "m") # Add C-math library
include_directories( FFTW_INC )

add_subdirectory( src )
FILE(GLOB HEADERFILES "include/*.hpp" )
install( FILES ${HEADERFILES} DESTINATION ${INSTALL_INC_DIR}/PaxPro )