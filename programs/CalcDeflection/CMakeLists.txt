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

# Добавляем dll файлы в папку build
#if(${WIN32})
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
#  COMMAND_EXPAND_LISTS
#)
#endif()

set_target_properties(CalcDeflection PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(CalcDeflection PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

#include(GNUInstallDirs)
install(TARGETS CalcDeflection
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    #RESOURCE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CalcDeflection)
endif()
