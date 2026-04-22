#include "catch_amalgamated.hpp"
#include "nukex/learning/rating_db.hpp"

#include <sqlite3.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using namespace nukex::learning;
namespace fs = std::filesystem;

namespace {
fs::path tmp_db_path(const std::string& suffix) {
    fs::path p = fs::temp_directory_path() / ("nukex_test_rating_" + suffix + ".sqlite");
    fs::remove(p);
    return p;
}
} // namespace

TEST_CASE("open_rating_db: creates fresh DB with schema", "[learning][rating_db]") {
    const auto path = tmp_db_path("fresh");

    sqlite3* db = open_rating_db(path.string());
    REQUIRE(db != nullptr);

    // runs table must exist
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='runs';";
    REQUIRE(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    REQUIRE(sqlite3_column_int(stmt, 0) == 1);
    sqlite3_finalize(stmt);

    // user_version == 1
    stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    REQUIRE(sqlite3_column_int(stmt, 0) == 1);
    sqlite3_finalize(stmt);

    close_rating_db(db);
    fs::remove(path);
}

TEST_CASE("open_rating_db: reopens existing DB without clobbering data", "[learning][rating_db]") {
    const auto path = tmp_db_path("reopen");

    sqlite3* db = open_rating_db(path.string());
    REQUIRE(db != nullptr);
    char* err = nullptr;
    REQUIRE(sqlite3_exec(db,
        "INSERT INTO runs(run_id, created_at, stretch_name, target_class, filter_class,"
        " params_json, rating_brightness, rating_saturation, rating_star_bloat, rating_overall)"
        " VALUES(randomblob(16), 1700000000, 'VeraLux', 0, 0, '{}', 0, 0, 0, 3);",
        nullptr, nullptr, &err) == SQLITE_OK);
    close_rating_db(db);

    // Reopen --> row is still present
    db = open_rating_db(path.string());
    REQUIRE(db != nullptr);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "SELECT count(*) FROM runs;", -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    REQUIRE(sqlite3_column_int(stmt, 0) == 1);
    sqlite3_finalize(stmt);
    close_rating_db(db);
    fs::remove(path);
}

TEST_CASE("open_rating_db: corrupt DB is renamed and fresh DB created", "[learning][rating_db]") {
    const auto path = tmp_db_path("corrupt");

    // Write garbage to the DB path
    {
        std::ofstream f(path);
        f << "this is not a SQLite file";
    }

    sqlite3* db = open_rating_db(path.string());
    REQUIRE(db != nullptr);

    // New DB at original path has the runs table
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db,
        "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='runs';",
        -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    REQUIRE(sqlite3_column_int(stmt, 0) == 1);
    sqlite3_finalize(stmt);

    // A .corrupt.<ts> sibling exists
    bool found_corrupt_sibling = false;
    for (const auto& entry : fs::directory_iterator(path.parent_path())) {
        const auto n = entry.path().filename().string();
        if (n.find(path.filename().string() + ".corrupt.") == 0) {
            found_corrupt_sibling = true;
            fs::remove(entry.path());
            break;
        }
    }
    REQUIRE(found_corrupt_sibling);

    close_rating_db(db);
    fs::remove(path);
}

TEST_CASE("open_rating_db: unwritable path returns nullptr", "[learning][rating_db]") {
    // A path under /proc is not a writable file location.
    sqlite3* db = open_rating_db("/proc/this_cannot_be_a_db");
    REQUIRE(db == nullptr);
}
