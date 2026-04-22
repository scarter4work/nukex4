#include "nukex/learning/rating_db.hpp"

#include <sqlite3.h>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>

namespace nukex::learning {

namespace {

constexpr const char* kSchemaV1 = R"SQL(
CREATE TABLE IF NOT EXISTS runs (
    run_id           BLOB PRIMARY KEY,
    created_at       INTEGER NOT NULL,
    stretch_name     TEXT NOT NULL,
    target_class     INTEGER NOT NULL,
    filter_class     INTEGER NOT NULL,

    stat_median_r REAL, stat_median_g REAL, stat_median_b REAL,
    stat_mad_r    REAL, stat_mad_g    REAL, stat_mad_b    REAL,
    stat_p50_r    REAL, stat_p50_g    REAL, stat_p50_b    REAL,
    stat_p95_r    REAL, stat_p95_g    REAL, stat_p95_b    REAL,
    stat_p99_r    REAL, stat_p99_g    REAL, stat_p99_b    REAL,
    stat_p999_r   REAL, stat_p999_g   REAL, stat_p999_b   REAL,
    stat_skew_r   REAL, stat_skew_g   REAL, stat_skew_b   REAL,
    stat_sat_frac_r REAL, stat_sat_frac_g REAL, stat_sat_frac_b REAL,

    stat_bright_concentration REAL,
    stat_color_rg  REAL, stat_color_bg REAL,
    stat_fwhm_median REAL,
    stat_star_count  INTEGER,

    params_json TEXT NOT NULL,

    rating_brightness INTEGER NOT NULL CHECK (rating_brightness BETWEEN -2 AND 2),
    rating_saturation INTEGER NOT NULL CHECK (rating_saturation BETWEEN -2 AND 2),
    rating_color      INTEGER          CHECK (rating_color      BETWEEN -2 AND 2),
    rating_star_bloat INTEGER NOT NULL CHECK (rating_star_bloat BETWEEN -2 AND 2),
    rating_overall    INTEGER NOT NULL CHECK (rating_overall    BETWEEN 1 AND 5)
);
CREATE INDEX IF NOT EXISTS idx_runs_stretch ON runs(stretch_name);
CREATE INDEX IF NOT EXISTS idx_runs_target  ON runs(target_class);
PRAGMA user_version = 1;
)SQL";

bool apply_pragmas_and_schema(sqlite3* db) {
    char* err = nullptr;
    if (sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &err) != SQLITE_OK) {
        sqlite3_free(err);
        return false;
    }
    if (sqlite3_exec(db, kSchemaV1, nullptr, nullptr, &err) != SQLITE_OK) {
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool integrity_ok(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA integrity_check;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        ok = (txt != nullptr) && (std::string(reinterpret_cast<const char*>(txt)) == "ok");
    }
    sqlite3_finalize(stmt);
    return ok;
}

std::string rename_corrupt(const std::string& path) {
    const auto ts = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    const std::string renamed = path + ".corrupt." + std::to_string(ts);
    std::error_code ec;
    std::filesystem::rename(path, renamed, ec);
    return ec ? std::string{} : renamed;
}

} // namespace

sqlite3* open_rating_db(const std::string& path) {
    for (int attempt = 0; attempt < 2; ++attempt) {
        sqlite3* db = nullptr;
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            if (db) sqlite3_close(db);
            return nullptr;
        }
        // A garbage-bytes file opens without error but fails on the first
        // real statement (SQLITE_NOTADB). Treat schema-apply failure as
        // corruption on attempt 0 so we rename-and-retry the same way as a
        // post-integrity-check failure.
        const bool schema_ok   = apply_pragmas_and_schema(db);
        const bool integrity   = schema_ok && integrity_ok(db);
        if (schema_ok && integrity) {
            return db;
        }
        sqlite3_close(db);
        if (attempt == 0) {
            if (rename_corrupt(path).empty()) {
                return nullptr;
            }
            continue;
        }
        return nullptr;
    }
    return nullptr;
}

void close_rating_db(sqlite3* db) {
    if (db) sqlite3_close(db);
}

// attach_bootstrap, insert_run, select_runs_for_stretch implemented in Task 7.
bool attach_bootstrap(sqlite3*, const std::string&) { return false; }
bool insert_run(sqlite3*, const RunRecord&) { return false; }
std::vector<RunRecord> select_runs_for_stretch(sqlite3*, const std::string&) { return {}; }

} // namespace nukex::learning
