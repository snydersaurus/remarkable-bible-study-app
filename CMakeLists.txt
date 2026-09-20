cmake_minimum_required(VERSION 3.21)
project(word_study VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick)
qt_standard_project_setup()

qt_add_executable(word_study
    src/main.cpp
    src/BibleData.cpp
    src/BibleData.h
)

set(WORD_STUDY_UI
    ui/Board.qml
    ui/WordToken.qml
)
foreach(f ${WORD_STUDY_UI} qml/Preview.qml)
    get_filename_component(base ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${base})
endforeach()

qt_add_qml_module(word_study
    URI BibleStudy
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        qml/Preview.qml
        ${WORD_STUDY_UI}
)

target_link_libraries(word_study PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)

install(TARGETS word_study RUNTIME DESTINATION bin)

add_executable(word_study_backend
    src/backend_main.cpp
    src/AppLoadLink.cpp
    src/AppLoadLink.h
    src/BibleData.cpp
    src/BibleData.h
)
target_link_libraries(word_study_backend PRIVATE Qt6::Core)
