cmake_minimum_required(VERSION 3.16)

project(WareLightHouse VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# list(APPEND CMAKE_PREFIX_PATH "/usr/local")
# find_path(CPR_INCLUDE_DIR
#           NAMES cpr.h)

# find_library(CPR_LIBRARY
#              NAMES cpr
#              HINTS ${CPR_LIBRARY_ROOT})

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(CPR REQUIRED_VARS CPR_LIBRARY CPR_INCLUDE_DIR)

# if(CPR_FOUND)
#     set(CPR_LIBRARIES ${CPR_LIBRARY})
#     set(CPR_INCLUDE_DIRS ${CPR_INCLUDE_DIR})
# endif()

# FetchContent for CPR
include(FetchContent)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr.git
    #GIT_TAG v1.10.4 # Use a specific tag for stability
)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cpr)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WareLightHouse
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        dashboard.hpp
        startscreen.hpp
        loginscreen.hpp
        registerscreen.hpp
        firebaselib.hpp
        passwordgen.hpp
        viewscreen.hpp
        adduserscreen.hpp
        edituserscreen.hpp
        messageboard.hpp
        resources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WareLightHouse APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WareLightHouse SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WareLightHouse
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(WareLightHouse PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui
    #/usr/local/lib/libcpr.so
    cpr::cpr
    nlohmann_json::nlohmann_json
    )

# target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets /usr/local/include/cpr)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WareLightHouse)
endif()
set_target_properties(WareLightHouse PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WareLightHouse
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WareLightHouse)
endif()
