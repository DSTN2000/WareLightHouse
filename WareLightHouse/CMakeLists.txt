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


# set(CPR_ENABLE_SSL OFF)

# FetchContent for CPR and json
include(FetchContent)

# FetchContent_Declare(openssl GIT_REPOSITORY https://github.com/openssl/openssl.git)
# FetchContent_MakeAvailable(openssl)

# set(OPENSSL_BUILD_DIR "${CMAKE_BINARY_DIR}/openssl-build")
# mkdir(${OPENSSL_BUILD_DIR})
# add_subdirectory(${openssl_SOURCE_DIR} ${OPENSSL_BUILD_DIR})

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr.git
    CMAKE_ARGS
    # -DENABLE_STRICT_TRY_COMPILE=ON
    # -DCURL_CA_FALLBACK=ON
    # -DCPR_ENABLE_SSL=OFF
    #GIT_TAG v1.10.4 # Use a specific tag for stability
)
#FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cpr)

# Get the cpr build directory
get_target_property(cpr_build_dir cpr::cpr IMPORTED_LOCATION_RELEASE)
get_filename_component(cpr_build_dir ${cpr_build_dir} DIRECTORY)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
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

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")



if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WareLightHouse
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}
    )

# Define target properties for Android with Qt 6 as:
   set_property(TARGET WareLightHouse APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WareLightHouse SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
    #set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
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

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

# qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ru)

# qt6_add_translations(${PROJECT_NAME} TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/WareLightHouse_en.ts ${CMAKE_CURRENT_SOURCE_DIR}/translations/WareLightHouse_ru.ts)

# Make the main target depend on updating translations
# This means lupdate will run before compiling translations
#add_dependencies(${PROJECT_NAME} update_translations)


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

if(WIN32)
    install(CODE "
        message(STATUS \"Copying runtime dependencies for WareLightHouse using TARGET_RUNTIME_DLLS\")
        file(COPY $<TARGET_RUNTIME_DLLS:WareLightHouse>
             DESTINATION \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
        )
    ")
endif()




