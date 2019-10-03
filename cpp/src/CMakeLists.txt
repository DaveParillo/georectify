cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(georectify VERSION 1.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
  main.cpp
  handler.h handler.cpp
  listener.h listener.cpp
  session.h session.cpp
  util.h
  )

target_link_libraries(${PROJECT_NAME} 
  boost_system
  boost_thread
  )
