cmake_minimum_required(VERSION 3.8)

project(SERAPHIM VERSION 1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
    ./vendor/obj/obj.cpp
    ./vendor/obj/obj.h

    ./source/core/types/LWO.cpp
    ./source/core/types/LWO.h
    ./source/core/types/OBJ.cpp
    ./source/core/types/OBJ.h
    ./source/core/types/ResourceFile.cpp
    ./source/core/types/ResourceFile.h

    ./source/core/ModelConverter.cpp
    ./source/core/ModelConverter.h
    ./source/core/Oodle.cpp
    ./source/core/Oodle.h
    ./source/core/ResourceFileReader.cpp
    ./source/core/ResourceFileReader.h
    ./source/core/Utilities.cpp
    ./source/core/Utilities.h

    ./source/qt/resourceform.ui
    ./source/qt/resourceform.cpp
    ./source/qt/resourceform.h
    ./source/qt/mainwindow.ui
    ./source/qt/mainwindow.cpp
    ./source/qt/mainwindow.h
    ./source/qt/main.cpp
)

set(APP_ICON_RESOURCE_WINDOWS "./resources/icon.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SERAPHIM
        WIN32
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )
else()
    if(ANDROID)
        add_library(SERAPHIM SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SERAPHIM
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Doom Eternal Model Importer v1.2")

target_link_libraries(SERAPHIM PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${CMAKE_DL_LIBS})

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /Oi /Ot /EHsc")
else()
    set(CMAKE_CXX_FLAGS "-Ofast -Wno-unused-result -pthread")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SERAPHIM)
endif()
