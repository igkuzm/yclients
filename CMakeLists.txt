cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.
project("yclients")

#To include yclients to your project use:
#set(YCLIENTS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/path/to/yclientslib)
#add_subdirectory(${YCLIENTS_SOURCE_DIR})

if (NOT DEFINED YCLIENTS_SOURCE_DIR)
  set(YCLIENTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif ()

add_executable(yclients_test ${YCLIENTS_SOURCE_DIR}/test.c )

add_library(yclients STATIC ${YCLIENTS_SOURCE_DIR}/yclients.c ${YCLIENTS_SOURCE_DIR}/cJSON.c)

# Determine the location of the libraries to use based on the platform.
if(ANDROID)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION ${YCLIENTS_SOURCE_DIR}/resources/curl/android/${ANDROID_ABI}/libcurl.a)
elseif(APPLE)
  if(IOS)
	ADD_LIBRARY(curl STATIC IMPORTED)
	SET_TARGET_PROPERTIES(curl PROPERTIES IMPORTED_LOCATION ${YCLIENTS_SOURCE_DIR}/resources/curl/iOS/libcurl.a)
  endif()
endif()

target_link_libraries(yclients curl z)
target_link_libraries(yclients_test yclients)
