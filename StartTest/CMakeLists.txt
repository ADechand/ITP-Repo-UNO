cmake_minimum_required(VERSION 3.16)

project(StartTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appStartTest
    main.cpp
    resources.qrc
    linkservice.cpp
)

qt_add_qml_module(appStartTest
    URI StartTest
    VERSION 1.0
    QML_FILES
        Main.qml
        StartMenu.qml
        StartPage.qml
        JoinPage.qml
        GamePage.qml
        SOURCES linkservice.h linkservice.cpp
)

qt_add_resources(appStartTest "app_resources"
    PREFIX "/"
    FILES
        assets/images/UNOHintergrund.jpg
)



# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStartTest PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStartTest
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_sources(appStartTest PRIVATE resources.qrc)

target_link_libraries(appStartTest
    PRIVATE Qt6::Quick Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS appStartTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
