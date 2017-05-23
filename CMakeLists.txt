cmake_minimum_required(VERSION 3.5)
project(url_filtering)

set(CONF_DEST /etc/urlfiltering)
set(SERVICE_DEST /etc/init.d/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w")

set(SOURCE_FILES
        main.cc
        db_search.cc
        symbols.hh
        lmdbcpp.h
        uri_manager.cc)


find_path(LMDB_INCLUDE_DIR NAMES  lmdb.h PATHS "$ENV{LMDB_DIR}/include")
find_library(LMDB_LIBRARIES NAMES lmdb   PATHS "$ENV{LMDB_DIR}/lib" )

if (NOT LMDB_LIBRARIES)
    message(FATAL_ERROR "LMDB Libraires required, but not found!")
else()
    include_directories(${LMDB_INCLUDE_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS system filesystem regex)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Libraires required, but not found!")
else()
    include_directories(${Boost_INCLUDE_DIRS})
endif()


add_executable(url_filtering ${SOURCE_FILES})

target_link_libraries(url_filtering ssl crypto ${Boost_LIBRARIES} ${LMDB_LIBRARIES} ${REST})

install (TARGETS url_filtering DESTINATION bin)
install(FILES url_filtering.ini DESTINATION ${CONF_DEST})
install(FILES badwordlist.txt DESTINATION ${CONF_DEST})
install(FILES url_filtering_service DESTINATION ${SERVICE_DEST}  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)