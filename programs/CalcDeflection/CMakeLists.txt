cmake_minimum_required(VERSION 3.5)

project(CalcDeflection VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets QCore)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    ui/mainwindow.ui
)

set(RESOURCE_FILES
    res/application.qrc
    res/logo.rc
)

qt_add_executable(CalcDeflection MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${RESOURCE_FILES}
)

target_link_libraries(CalcDeflection PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CalcDeflection)
endif()

set_target_properties(CalcDeflection PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    RESOURCE "${RESOURCE_FILES}"
)

include(GNUInstallDirs)
install(TARGETS CalcDeflection
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    RESOURCE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CalcDeflection)
endif()
