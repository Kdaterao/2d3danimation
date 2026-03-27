
#----- version -----
cmake_minimum_required(VERSION 3.16)
project(2d3danimation VERSION 0.1.0 LANGUAGES C CXX)

#--- Set cpp version to 20 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #make compiler use 20



#----- Specify sources(qt) and include directories -----

set(SOURCES 
    include/CanvasWidget/CanvasWidget.cpp include/CanvasWidget/CanvasWidget.h

    include/2D/toonzPainterGL.cpp include/2D/toonzPainterGL.h 
    include/2D/toonzTextureManager.cpp include/2D/toonzTextureManager.h
    include/2D/toonzShader.cpp include/2D/toonzShader.h

    include/utility/toonzGeometry.h
    include/utility/types.h
    include/utility/toonzGL.cpp include/utility/toonzGL.h


)

set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CanvasWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utility

)

# ============================
#       QT Setup 
# ============================

#----- Get Package ------

list(APPEND CMAKE_PREFIX_PATH "/Users/krish/codingStuff/qt6-build")#share location of qt build(just in case)
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets) 


#----- Set up qt project -----
qt_standard_project_setup()


qt_add_executable(${PROJECT_NAME}
    main.cpp

    ${SOURCES}
)

#----- linking libraries and headers (do afte qt project setup!)-----

#Set up header file location(must be done after qt project set up)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)



#link libraries (must be done after qt project set up)
target_link_libraries(${PROJECT_NAME} PUBLIC  Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

#----- OS build options -----

set_target_properties(${PROJECT_NAME} PROPERTIES
    #WIN32_EXECUTABLE ON (we are testing on macos for now)
    MACOSX_BUNDLE ON
)



# ============================
#       Google Test Setup 
# ============================



# Enable testing
enable_testing()
include(GoogleTest)

#----- Get package ----- 
find_package(GTest REQUIRED)


# Add test executable
add_executable(${PROJECT_NAME}_tests
    tests/test_main.cpp
    ${SOURCES}   
)

# Include directories for tests
target_include_directories(${PROJECT_NAME}_tests PUBLIC
    ${PROJECT_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)


# Link GTest libraries
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread  # required on macOS


    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

gtest_discover_tests(${PROJECT_NAME}_tests)