cmake_minimum_required(VERSION 3.5)

project(CalcDeflection VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets QCore)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
message(STATUS "Found Qt: " Qt${QT_VERSION_MAJOR})

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    #src/infoDialog.cpp
    #src/infoDialog.h
)
set(PROJECT_UI
    ui/mainwindow.ui
    #ui/infoWindow.ui
)
set(RESOURCE_FILES
    res/application.qrc
    res/logo.rc
)

# Assembly differeces for Qt5 and Qt6
if(Qt6_FOUND)
    qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES} ${PROJECT_UI} ${RESOURCE_FILES}
        ui/infoWindow.ui)
endif()
if(Qt5_FOUND)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES} ${PROJECT_UI} ${RESOURCE_FILES})
endif()

# Make building directory
file(MAKE_DIRECTORY build/)

# Setting up dependencies
if(${WIN32})
    if(MINGW)
        string(REPLACE "/c++.exe" "" LIB_PATH ${CMAKE_CXX_COMPILER})
        message(STATUS "MinGW bin directory: " ${LIB_PATH})
        file(GLOB LIBS
            "${LIB_PATH}/libstdc++*.dll"
            "${LIB_PATH}/libgcc*.dll"
            "${LIB_PATH}/libwinpthread*.dll"
        )
        foreach(DLL ${LIBS})
        message(STATUS "Dependency found: " ${DLL})
        endforeach()
	
        # Copying dll files to the building folder
        file(MAKE_DIRECTORY build/platforms/)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> "${LIBS}"
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/ $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::QWindowsIntegrationPlugin>
            COMMAND_EXPAND_LISTS
        )
    endif()
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=broadwell -O3") # Intel Core i7-6900K
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=bdver1 -O3") # AMD FX-8150
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
