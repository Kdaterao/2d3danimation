
#----- version -----
cmake_minimum_required(VERSION 3.16)
project(2d3danimation VERSION 0.1.0 LANGUAGES C CXX)

#--- Set cpp version to 20 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #make compiler use 20



#----- Specify sources(qt) and include directories -----

set(SOURCES 
    include/Widgets/CanvasWidget/CanvasWidget.cpp include/Widgets/CanvasWidget/CanvasWidget.h

    include/2D/Painter/Canvas/painterGL.cpp include/2D/Painter/Canvas/painterGL.h 
    include/2D/Painter/General/Texture/toonzTextureManager.cpp include/2D/Painter/General/Texture/toonzTextureManager.h
    include/2D/Painter/General/Shader/toonzShader.cpp include/2D/Painter/General/Shader/toonzShader.h
    include/2D/Brush/Brush.cpp include/2D/Brush/Brush.h
    include/2D/Brush/ToonzBrush/toonzRasterBrush.cpp include/2D/Brush/ToonzBrush/toonzRasterBrush.h
    include/2D/Brush/ToonzBrush/toonzRasterCircleBrush.h


    include/2D/Raster/rasterPixel.cpp include/2D/Raster/rasterPixel.h



    include/2D/Brush/MPLBrush/mplBrush.cpp include/2D/Brush/MPLBrush/mplBrush.h
    include/2D/Brush/MPLBrush/mplSurfaceAdapter.cpp include/2D/Brush/MPLBrush/mplSurfaceAdapter.h
    
    
    include/Widgets/CanvasWidget/CanvasWidget.cpp include/Widgets/CanvasWidget/CanvasWidget.h

    include/Widgets/ColorWidget/ColorTriangleWidget/ColorTriangleWidget.cpp include/Widgets/ColorWidget/ColorTriangleWidget/ColorTriangleWidget.h

    include/Widgets/ToolOptionWidget/ToolOptionWidget.cpp include/Widgets/ToolOptionWidget/ToolOptionWidget.h
    include/Widgets/ToolOptionWidget/BrushCatalog.cpp include/Widgets/ToolOptionWidget/BrushCatalog.h
    include/Widgets/ToolOptionWidget/ToolViews/BrushWidget/BrushWidget.cpp include/Widgets/ToolOptionWidget/ToolViews/BrushWidget/BrushWidget.h
    include/Widgets/ToolOptionWidget/ToolViews/BrushWidget/BrushPreviewCell.cpp include/Widgets/ToolOptionWidget/ToolViews/BrushWidget/BrushPreviewCell.h

    include/Widgets/TimelineWidget/TimelineWidget.cpp include/Widgets/TimelineWidget/TimelineWidget.h
    

    include/2D/Raster/raster.h
    include/2D/Raster/rasterTile.h

    include/Utility/Math/toonzGeometry.h
    include/Utility/Types/types.h
    include/2D/Painter/UI/UIPainter.cpp include/2D/Painter/UI/UIPainter.h 
    include/2D/Painter/UI/UIShapes.h
    include/Utility/Math/toonzCalculations.cpp include/Utility/Math/toonzCalculations.h
    

    include/Storage/storageManager.cpp include/Storage/storageManager.h

    include/Storage/tinyXML/tinyxml2.cpp include/Storage/tinyXML/tinyxml2.h

    include/Scene/canvas.cpp include/Scene/canvas.h
    include/Scene/rasterLayer.cpp include/Scene/rasterLayer.h
    include/Scene/frame.h
    include/Scene/layer.h
    
)

set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Widgets/CanvasWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Widgets/ColorWidget/ColorTriangleWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Widgets/ToolOptionWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Widgets/ToolOptionWidget/ToolViews/BrushWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Widgets/TimelineWidget

    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Brush
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Brush/MPLBrush
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Brush/ToonzBrush
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Painter/Canvas
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Painter/General/Shader
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Painter/General/Texture
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Painter/UI
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/2D/Raster

    ${CMAKE_CURRENT_SOURCE_DIR}/include/Utility/Math
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Utility/Types

    ${CMAKE_CURRENT_SOURCE_DIR}/include/Storage
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Storage/tinyXML

    ${CMAKE_CURRENT_SOURCE_DIR}/include/Scene

    
    
)


#=============================
#   PkgConfig (for dependencies without make files )
#=============================

find_package(PkgConfig REQUIRED)


# ============================
#       QT Setup 
# ============================

#----- Get Package ------

# with make files 
list(APPEND CMAKE_PREFIX_PATH "/Users/krish/codingStuff/qt6-build")#share location of qt build(just in case)
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets) 

# without make files (pkg)

set(ENV{PKG_CONFIG_PATH}
    "/Users/krish/codingStuff/libmypaint"
)

pkg_check_modules(MYPAINT REQUIRED IMPORTED_TARGET libmypaint-2.0)


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

#qt libraries 
target_link_libraries(${PROJECT_NAME} PUBLIC  Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

# link mypaint
target_link_libraries(${PROJECT_NAME}  PUBLIC PkgConfig::MYPAINT)

#----- OS build options -----

set_target_properties(${PROJECT_NAME} PROPERTIES
    #WIN32_EXECUTABLE ON (we are testing on macos for now)
    MACOSX_BUNDLE ON
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer
)

target_link_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
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

    PkgConfig::MYPAINT

    
    
)

gtest_discover_tests(${PROJECT_NAME}_tests)