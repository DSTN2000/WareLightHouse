cmake_minimum_required(VERSION 3.16)

project(WareWatchHouse VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

list(APPEND CMAKE_PREFIX_PATH "/usr/local")
find_path(CPR_INCLUDE_DIR
          NAMES cpr.h)

find_library(CPR_LIBRARY
             NAMES cpr
             HINTS ${CPR_LIBRARY_ROOT})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CPR REQUIRED_VARS CPR_LIBRARY CPR_INCLUDE_DIR)
# set(cpr_DIR "/usr/local/lib/cmake/cpr")
# find_package(cpr REQUIRED)
# if(TARGET cpr::cpr)
#   message(STATUS "CPR library found and target exists")
# else()
#   message(FATAL_ERROR "CPR library not found or target doesn't exist")
# endif()

if(CPR_FOUND)
    set(CPR_LIBRARIES ${CPR_LIBRARY})
    set(CPR_INCLUDE_DIRS ${CPR_INCLUDE_DIR})
endif()

# find_package(nlohmann_json REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WareWatchHouse
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        dashboard.hpp
        startscreen.hpp
        loginscreen.hpp
        registerscreen.hpp
        firebaselib.hpp
        passwordgen.hpp
        viewscreen.hpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WareWatchHouse APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WareWatchHouse SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WareWatchHouse
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(WareWatchHouse PRIVATE Qt${QT_VERSION_MAJOR}::Widgets /usr/local/lib/libcpr.so)

# target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets /usr/local/include/cpr)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WareWatchHouse)
endif()
set_target_properties(WareWatchHouse PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WareWatchHouse
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WareWatchHouse)
endif()
