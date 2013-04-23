/*
   Copyright (C) 2013  Nick Ogden <nick@nickogden.org>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "database.h"
#include "error.h"

#include <sqlite3.h>

#include <ostream>
#include <sstream>
#include <cassert>

namespace sqlite {

database::database(const std::string &path, const access_mode &permissions) {
    auto status(sqlite3_open_v2(path.c_str(), &db, permissions, nullptr));
    if (status != SQLITE_OK) {
        sqlite3_close_v2(db);
        throw error(status);
    }
}

database::~database() {
    close();
}

result database::execute(const std::string &sql) {
    return create_statement(sql);
}

result database::execute(const statement &statement) {
    return make_result(statement);
}

void database::close() {
    auto status(sqlite3_close(db));
    db = nullptr;
    // Can't throw, called from destructor
    assert(
        status == SQLITE_OK &&
        "database closed with active statements or unfinished backups"
    );
}

std::shared_ptr<sqlite3_stmt> database::create_statement(
        const std::string &sql
) const {
    sqlite3_stmt *stmt(nullptr);
    auto status(sqlite3_prepare_v2(
        db, sql.c_str(), sql.size(), &stmt, nullptr
    ));
    if (status != SQLITE_OK)
        throw bad_statement(status, sql);
    return std::shared_ptr<sqlite3_stmt>(stmt, &sqlite3_finalize);
}

statement database::prepare_statement(
        const std::string &sql
) const {
    return create_statement(sql);
}

std::ostream& operator<<(std::ostream &os, const database &db) {
    os << "database:\n"
          "  open: " << ((db.db == nullptr) ? "false" : "true") << "\n";
    return os;
}

} // namespace sqlite
