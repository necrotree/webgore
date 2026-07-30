cmake_minimum_required(VERSION 3.16)

project(Webgore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets WebKitWidgets)

add_executable(Webgore WIN32
    src/main.cpp
)

target_link_libraries(Webgore PRIVATE
    Qt5::Widgets
    Qt5::WebKitWidgets
)

get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(MINGW_RUNTIME_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(QT_WINDEPLOYQT "${QT_BIN_DIR}/windeployqt.exe")
if(NOT EXISTS "${QT_WINDEPLOYQT}")
    set(QT_WINDEPLOYQT "${QT_BIN_DIR}/windeployqt-qt5.exe")
endif()

add_custom_command(TARGET Webgore POST_BUILD
    COMMAND "${QT_WINDEPLOYQT}" --no-translations "$<TARGET_FILE:Webgore>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${MINGW_RUNTIME_DIR}/libgcc_s_seh-1.dll"
        "${MINGW_RUNTIME_DIR}/libstdc++-6.dll"
        "${MINGW_RUNTIME_DIR}/libwinpthread-1.dll"
        "${MINGW_RUNTIME_DIR}/libdouble-conversion.dll"
        "${MINGW_RUNTIME_DIR}/libbrotlicommon.dll"
        "${MINGW_RUNTIME_DIR}/libbrotlidec.dll"
        "${MINGW_RUNTIME_DIR}/libbz2-1.dll"
        "${MINGW_RUNTIME_DIR}/libicudt78.dll"
        "${MINGW_RUNTIME_DIR}/libicuin78.dll"
        "${MINGW_RUNTIME_DIR}/libicuuc78.dll"
        "${MINGW_RUNTIME_DIR}/libiconv-2.dll"
        "${MINGW_RUNTIME_DIR}/libintl-8.dll"
        "${MINGW_RUNTIME_DIR}/libfreetype-6.dll"
        "${MINGW_RUNTIME_DIR}/libglib-2.0-0.dll"
        "${MINGW_RUNTIME_DIR}/libgraphite2.dll"
        "${MINGW_RUNTIME_DIR}/libharfbuzz-0.dll"
        "${MINGW_RUNTIME_DIR}/libjpeg-8.dll"
        "${MINGW_RUNTIME_DIR}/libmd4c.dll"
        "${MINGW_RUNTIME_DIR}/libpcre2-16-0.dll"
        "${MINGW_RUNTIME_DIR}/libpcre2-8-0.dll"
        "${MINGW_RUNTIME_DIR}/libpng16-16.dll"
        "${MINGW_RUNTIME_DIR}/libsharpyuv-0.dll"
        "${MINGW_RUNTIME_DIR}/libsqlite3-0.dll"
        "${MINGW_RUNTIME_DIR}/libwebp-7.dll"
        "${MINGW_RUNTIME_DIR}/libwoff2common.dll"
        "${MINGW_RUNTIME_DIR}/libwoff2dec.dll"
        "${MINGW_RUNTIME_DIR}/libxml2-16.dll"
        "${MINGW_RUNTIME_DIR}/libxslt-1.dll"
        "${MINGW_RUNTIME_DIR}/libzstd.dll"
        "${MINGW_RUNTIME_DIR}/zlib1.dll"
        "$<TARGET_FILE_DIR:Webgore>"
    COMMENT "Deploying Qt runtime dependencies"
)