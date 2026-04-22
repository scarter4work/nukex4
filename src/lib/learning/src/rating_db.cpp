#include "nukex/learning/rating_db.hpp"

namespace nukex::learning {

sqlite3* open_rating_db(const std::string& /*path*/) {
    return nullptr;
}

bool attach_bootstrap(sqlite3* /*db*/, const std::string& /*bootstrap_path*/) {
    return false;
}

bool insert_run(sqlite3* /*db*/, const RunRecord& /*rec*/) {
    return false;
}

std::vector<RunRecord> select_runs_for_stretch(sqlite3* /*db*/, const std::string& /*stretch_name*/) {
    return {};
}

void close_rating_db(sqlite3* /*db*/) {}

} // namespace nukex::learning
