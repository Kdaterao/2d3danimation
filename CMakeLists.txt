
#----- version -----
cmake_minimum_required(VERSION 3.16)
project(2d3danimation VERSION 0.1.0 LANGUAGES C CXX)

#--- Set cpp version to 20 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #make compiler use 20


#----- Get Package info (do before qt project setup!) ------

#share location of qt build(just in case)
set(CMAKE_PREFIX_PATH "/Users/krish/codingStuff/qt6-build")
message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}") #debug

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets) 

#----- Specify .h and .cpp for qt_add_executable -----

set(SOURCES 
    include/CanvasWidget/CanvasWidget.cpp include/CanvasWidget/CanvasWidget.h

)

#----- Set up qt project -----
qt_standard_project_setup()


qt_add_executable(${PROJECT_NAME}
    main.cpp
    ${SOURCES}
)

#----- linking libraries and headers (do afte qt project setup!)-----

#Set up header file location(must be done after qt project set up)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#link libraries (must be done after qt project set up)
target_link_libraries(${PROJECT_NAME} PUBLIC  Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

#----- OS build options -----

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)





