# SQLite is linked into the PixInsight module.  Same reasoning as cfitsio in
# src/lib/io/CMakeLists.txt: system libsqlite3 may pull transitive deps
# that conflict with PI's bundled libraries, and dev machines that have
# sqlite3-dev vs. sqlite3 in different ABI variants will cause module-load
# failures on end-user systems.
#
# Default path: FetchContent pulls the SQLite amalgamation and builds it as
# a static object, linked privately into nukex4_learning.

option(NUKEX_USE_SYSTEM_SQLITE
       "Use system SQLite (dev only -- module may not load in PixInsight)" OFF)

if(NUKEX_USE_SYSTEM_SQLITE)
    if(NUKEX_RELEASE_BUILD)
        message(FATAL_ERROR
            "NUKEX_USE_SYSTEM_SQLITE=ON is incompatible with NUKEX_RELEASE_BUILD=ON. "
            "Distribution builds require the vendored SQLite amalgamation.")
    endif()
    find_package(SQLite3 REQUIRED)
    set(_nukex_sqlite_libs SQLite::SQLite3)
    set(_nukex_sqlite_inc  "")
else()
    enable_language(C)
    include(FetchContent)

    FetchContent_Declare(
        sqlite_amalgamation
        URL      https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
        URL_HASH SHA3_256=af6aae8d3eccc608857c63cf56efbadc70da48b5c719446b353ed88dded1e288
    )
    FetchContent_MakeAvailable(sqlite_amalgamation)

    add_library(sqlite3_vendored STATIC
        ${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c
    )
    target_include_directories(sqlite3_vendored PUBLIC
        ${sqlite_amalgamation_SOURCE_DIR}
    )
    # WAL_SYNCHRONOUS sets the synchronous level *when* WAL is active;
    # the connection must still request WAL at runtime via
    # PRAGMA journal_mode=WAL; (see open_rating_db in Task 6).
    target_compile_definitions(sqlite3_vendored PRIVATE
        SQLITE_ENABLE_FTS5=0
        SQLITE_ENABLE_RTREE=0
        SQLITE_OMIT_LOAD_EXTENSION=1
        SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
        SQLITE_THREADSAFE=1
    )
    set_property(TARGET sqlite3_vendored PROPERTY POSITION_INDEPENDENT_CODE ON)

    set(_nukex_sqlite_libs sqlite3_vendored)
    set(_nukex_sqlite_inc  ${sqlite_amalgamation_SOURCE_DIR})
endif()

# Learning library sources added in Task 4.
add_library(nukex4_learning INTERFACE)
target_include_directories(nukex4_learning INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(nukex4_learning INTERFACE ${_nukex_sqlite_libs})
target_compile_features(nukex4_learning INTERFACE cxx_std_17)
