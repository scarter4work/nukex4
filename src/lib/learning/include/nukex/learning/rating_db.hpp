#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace nukex::learning {

// Per-row record that is inserted / queried from the runs table.
// Channel-specific stats carry NaN when the filter class doesn't populate
// that channel (e.g. mono has R channel; G/B carry NaN).
struct RunRecord {
    // Identifiers
    std::array<std::uint8_t, 16> run_id{};
    std::int64_t                 created_at_unix = 0;
    std::string                  stretch_name;
    int                          target_class    = 0;
    int                          filter_class    = 0;

    // 24 per-channel stats in RGB order (median, MAD, p50, p95, p99, p999, skew, sat_frac)
    std::array<double, 24>       per_channel_stats{};

    // 5 global stats: bright_concentration, color_rg, color_bg, fwhm_median, star_count
    double bright_concentration = 0.0;
    double color_rg             = 0.0;
    double color_bg             = 0.0;
    double fwhm_median          = 0.0;
    int    star_count           = 0;

    // Applied params as JSON text
    std::string params_json;

    // Ratings (-2..2 on signed axes; 1..5 on overall)
    int                  rating_brightness = 0;
    int                  rating_saturation = 0;
    std::optional<int>   rating_color;          // NULL for mono/narrowband
    int                  rating_star_bloat = 0;
    int                  rating_overall    = 0;
};

// Opens (or creates) a SQLite DB at `path`. Applies schema v1 if the DB is
// empty. Enables WAL mode. Runs PRAGMA integrity_check; on failure renames
// the DB to `<path>.corrupt.<timestamp>` and returns a fresh DB.
//
// Returns nullptr on unrecoverable failure (e.g. dir not writable).
sqlite3* open_rating_db(const std::string& path);

// Optional: attach the read-only bootstrap DB under alias "bootstrap".
// No-op when bootstrap_path is empty or the file does not exist.
// Used by train_model to union user + bootstrap rows at fit time.
bool attach_bootstrap(sqlite3* db, const std::string& bootstrap_path);

// Insert a single row (runs table). Wrap in a transaction; return false on
// any error. Row is either fully written or not written at all.
bool insert_run(sqlite3* db, const RunRecord& rec);

// Select all rows for a given stretch_name from user DB + attached bootstrap.
std::vector<RunRecord> select_runs_for_stretch(sqlite3* db, const std::string& stretch_name);

void close_rating_db(sqlite3* db);

} // namespace nukex::learning
