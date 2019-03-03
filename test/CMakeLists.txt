cmake_minimum_required(VERSION 2.8.9)

include_directories(../thirdparty/bandit)
include_directories(../thirdparty/cpp-httplib)
include_directories(${LIBCEX_EXTERNAL_INCLUDES})

# just loop all files found in test directory, and create a test for each file found
# test name will be file's basename

file(GLOB files "*.cc")
file(GLOB datafiles "testdata*")

foreach(file ${files})
   get_filename_component(BASENAME ${file} NAME_WE)

   add_executable(${BASENAME} ${file})
   
   target_compile_features(${BASENAME} PRIVATE cxx_range_for)
   target_link_libraries(${BASENAME} cex pthread ${LIBEVHTP_LIBRARIES} ${LIBCEX_EXTERNAL_LIBS})
   
   add_test(${BASENAME} ${BASENAME} "--reporter=spec" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)
endforeach()

file(COPY testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
