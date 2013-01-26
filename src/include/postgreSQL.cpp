#include "postgreSQL.h"

void getPGColumnTypes(const char* rowFormat,
                             std::vector <uint32_t> &columnTypes) {
  uint32_t columnType = 0;
  while (*rowFormat) {
    switch (*rowFormat++) {
      case '%':
        if (columnType) {
          columnTypes.push_back(columnType);
          columnType = 0;
        }
        break;
      case 'n':
        columnType = PG_NULL;
        break;
      case 'u':
        columnType |= (columnType & PG_FIRST_DIMENSION_UNSIGNED) == 0 ?
                       PG_FIRST_DIMENSION_UNSIGNED :
                       PG_SECOND_DIMENSION_UNSIGNED;
        break;
      case 'h':
        columnType |= (columnType & PG_FIRST_DIMENSION_INT16) == 0 ?
                       PG_FIRST_DIMENSION_INT16 :
                       PG_SECOND_DIMENSION_INT16;
        break;
      case 'd':
        columnType |= (columnType & PG_FIRST_DIMENSION_INT32) == 0 ?
                       PG_FIRST_DIMENSION_INT32 :
                       PG_SECOND_DIMENSION_INT32;
        break;
      case 'l':
        columnType |= (columnType & PG_FIRST_DIMENSION_INT64) == 0 ?
                       PG_FIRST_DIMENSION_INT64 :
                       PG_SECOND_DIMENSION_INT64;
        break;
      case 'f':
        columnType |= (columnType & PG_FIRST_DIMENSION_DOUBLE) == 0 ?
                       PG_FIRST_DIMENSION_DOUBLE :
                       PG_SECOND_DIMENSION_DOUBLE;
        break;
      case 's':
        columnType |= (columnType & PG_FIRST_DIMENSION_STRING) == 0 ?
                       PG_FIRST_DIMENSION_STRING :
                       PG_SECOND_DIMENSION_STRING;
        break;
      case 'V':
        columnType |= (columnType & PG_FIRST_DIMENSION_VECTOR) == 0 ?
                       PG_FIRST_DIMENSION_VECTOR :
                       PG_SECOND_DIMENSION_VECTOR;
        break;
      case 'S':
        columnType |= (columnType & PG_FIRST_DIMENSION_UNORDERED_SET) == 0 ?
                       PG_FIRST_DIMENSION_UNORDERED_SET :
                       PG_SECOND_DIMENSION_UNORDERED_SET;
        break;
      case 'M':
        columnType |= PG_FIRST_DIMENSION_UNORDERED_MAP;
        break;
    }
  }
  columnTypes.push_back(columnType);
}

PGBulkInserter::PGBulkInserter(PGconn* _postgreSQL,
                                      const char* _schemaName,
                                      const char* _tableName,
                                      size_t &_flushSize,
                                      const char* rowFormat) {
  postgreSQL = _postgreSQL;
  schemaName = _schemaName;
  tableName = _tableName;
  flushSize = _flushSize * 1024;
  getPGColumnTypes(rowFormat, columnTypes);
}

void PGBulkInserter::setTableName(std::string _tableName) {
  tableName = _tableName;
}

template <class T>
void make1DArray(std::ostringstream &query, T &container,
                        bool firstDimension = true) {
  if (firstDimension) {
    query << "'{";
  }
  else {
    query << "{";
  }
  typename T::iterator containerItr = container.begin();
  for (size_t elementNumber = 0; containerItr != container.end();
       ++containerItr, ++elementNumber) {
    query << *containerItr;
    if (elementNumber != container.size() - 1) {
      query << ", ";
    }
  }
  if (firstDimension) {
    query << "}'";
  }
  else {
    query << "}";
  }
}

template <class T>
void make2DArray(std::ostringstream &query, T &container) {
  query << "'{";
  typename T::iterator containerItr = container.begin();
  for (size_t elementNumber = 0; containerItr != container.end();
       ++containerItr, ++elementNumber) {
    make1DArray(query, *containerItr, false);
    if (elementNumber != container.size() - 1) {
      query << ", ";
    }
  }
  query << "}'";
}

void addPGRow(PGconn *postgreSQL, va_list &columns,
                     std::vector <uint32_t> &columnTypes,
                     std::ostringstream &query) {
  static char *unescapedString, *escapedString;
  static size_t unescapedStringLength;
  query << '(';
  for (size_t index = 0; index < columnTypes.size(); ++index) {
    switch (columnTypes[index]) {
      case (PG_NULL):
        query << "NULL";
        break;
      case (PG_FIRST_DIMENSION_INT32):
        query << va_arg(columns, int32_t);
        break;
      case (PG_FIRST_DIMENSION_UNSIGNED | PG_FIRST_DIMENSION_INT32):
        query << '\'' << va_arg(columns, uint32_t) << '\'';
        break;
      case (PG_FIRST_DIMENSION_INT64):
        query << va_arg(columns, int64_t);
        break;
      case (PG_FIRST_DIMENSION_UNSIGNED | PG_FIRST_DIMENSION_INT64):
        query << '\'' << va_arg(columns, uint64_t) << '\'';
        break;
      case (PG_FIRST_DIMENSION_DOUBLE):
        query << va_arg(columns, double);
        break;
      case (PG_FIRST_DIMENSION_STRING):
        unescapedString = va_arg(columns, char*);
        unescapedStringLength = strlen(unescapedString);
        /*
         * Allocates memory for the worst case--every character requiring
         * escaping--plus one byte for the terminating NULL byte.
         */
        escapedString = (char*)malloc((unescapedStringLength * 2) + 1);
        PQescapeStringConn(postgreSQL, escapedString, unescapedString,
                           unescapedStringLength, NULL);
        query << '\'' << escapedString << '\'';
        free(escapedString);
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_INT16):
        make1DArray(query, *(std::vector <int16_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT16):
        make1DArray(query, *(std::vector <uint16_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_INT32):
        make1DArray(query, *(std::vector <int32_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT32):
        make1DArray(query, *(std::vector <uint32_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_INT64):
        make1DArray(query, *(std::vector <int64_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT64):
        make1DArray(query, *(std::vector <uint64_t>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_DOUBLE):
        make1DArray(query, *(std::vector <double>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_FIRST_DIMENSION_STRING):
        make1DArray(query, *(std::vector <std::string>*)va_arg(columns, void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_INT16):
        make1DArray(query, *(std::tr1::unordered_set <int16_t>*)va_arg(columns,
                                                                       void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT16):
        make1DArray(query, *(std::tr1::unordered_set <uint16_t>*)va_arg(columns,
                                                                        void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_INT32):
        make1DArray(query, *(std::tr1::unordered_set <int32_t>*)va_arg(columns,
                                                                       void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT32):
        make1DArray(query, *(std::tr1::unordered_set <uint32_t>*)va_arg(columns,
                                                                        void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_INT64):
        make1DArray(query, *(std::tr1::unordered_set <int64_t>*)va_arg(columns,
                                                                       void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_UNSIGNED |
            PG_FIRST_DIMENSION_INT64):
        make1DArray(query, *(std::tr1::unordered_set <uint64_t>*)va_arg(columns,
                                                                        void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_DOUBLE):
        make1DArray(query, *(std::tr1::unordered_set <double>*)va_arg(columns,
                                                                      void*));
        break;
      case (PG_FIRST_DIMENSION_UNORDERED_SET | PG_FIRST_DIMENSION_STRING):
        make1DArray(query,
                    *(std::tr1::unordered_set <std::string>*)va_arg(columns,
                                                                    void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_SECOND_DIMENSION_VECTOR |
            PG_FIRST_DIMENSION_UNSIGNED | PG_FIRST_DIMENSION_INT32):
        make2DArray(query,
                    *(std::vector <std::vector <uint32_t> >*)va_arg(columns,
                                                                    void*));
        break;
      case (PG_FIRST_DIMENSION_VECTOR | PG_SECOND_DIMENSION_VECTOR |
            PG_FIRST_DIMENSION_UNSIGNED | PG_FIRST_DIMENSION_INT64):
        make2DArray(query,
                    *(std::vector <std::vector <uint64_t> >*)va_arg(columns,
                                                                    void*));
        break;
    }
    if (index < columnTypes.size() - 1) {
      query << ", ";
    }
  }
  query << ')';
}

bool PGBulkInserter::insert(void* last, ...) {
  va_list columns;
  if (!query.str().length()) {
    query << "INSERT INTO \"" + schemaName + "\".\"" + tableName +
             "\" VALUES ";
  }
  else {
    query << ", ";
  }
  va_start(columns, last);
  addPGRow(postgreSQL, columns, columnTypes, query);
  va_end(columns);
  if (query.str().length() >= flushSize) {
    return flush();
  }
  return true;
}

size_t PGBulkInserter::size() {
  return query.str().length();
}

bool PGBulkInserter::flush() {
  bool status;
  PGresult* result = PQexec(postgreSQL, query.str().c_str());
  status = (PQresultStatus(result) == PGRES_COMMAND_OK);
  query.str("");
  PQclear(result);
  return status;
}

bool insertPGRow(PGconn* postgreSQL, std::string schemaName,
                 std::string tableName, const char* rowFormat, ...) {
  std::vector <uint32_t> columnTypes;
  std::ostringstream query;
  va_list columns;
  getPGColumnTypes(rowFormat, columnTypes);
  query << "INSERT INTO \"" + schemaName + "\".\"" + tableName +
           "\" VALUES ";
  va_start(columns, rowFormat);
  addPGRow(postgreSQL, columns, columnTypes, query);
  va_end(columns);
  return (PQresultStatus(PQexec(postgreSQL,
                                query.str().c_str())) == PGRES_COMMAND_OK);
}

int existsPGTable(PGconn* postgreSQL, std::string schemaName,
                         std::string tableName) {
  PGresult* result;
  std::string query = "SELECT \"table_name\" FROM " \
                      "\"information_schema\".\"tables\" WHERE " \
                      "\"table_schema\" = '" + schemaName +
                      "' AND \"table_name\" = '" + tableName + "';";
  result = PQexec(postgreSQL, query.c_str());
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    PQclear(result);
    return -1;
  }
  int ret = PQntuples(result);
  PQclear(result);
  return ret;
}

bool createPGTable(PGconn* postgreSQL, std::string schemaName,
                          std::string tableName, std::string tableSchema) {
  std::string query = "CREATE TABLE \"" + schemaName + "\".\"" + tableName +
                        "\" (" + tableSchema + ");";
  return (PQresultStatus(PQexec(postgreSQL,
                                query.c_str())) == PGRES_COMMAND_OK);
}

bool deletePGRows(PGconn* postgreSQL, std::string schemaName,
                         std::string tableName, std::string columnName,
                         std::string value) {
  std::string query = "DELETE FROM \"" + schemaName + "\".\"" + tableName +
                      "\" WHERE \"" + columnName + "\" = '" + value + "';";
  return (PQresultStatus(PQexec(postgreSQL,
                                query.c_str())) == PGRES_COMMAND_OK);
}

bool dropPGTable(PGconn* postgreSQL, std::string schemaName,
                        std::string tableName) {
  std::string query = "DROP TABLE \"" + schemaName + "\".\"" + tableName +
                      "\";";
  return (PQresultStatus(PQexec(postgreSQL,
                                query.c_str())) == PGRES_COMMAND_OK);
}

bool preparePGTable(PGconn* postgreSQL, std::string schemaName,
                           std::string tableName, std::string tableSchema,
                           bool drop) {
  if (existsPGTable(postgreSQL, schemaName, tableName) == 1) {
    if (drop) {
      if (!dropPGTable(postgreSQL, schemaName, tableName)) {
        return false;
      }
    }
    else {
      return true;
    }
  }
  return createPGTable(postgreSQL, schemaName, tableName, tableSchema);
}

bool preparePGTable(PGconn* postgreSQL, std::string schemaName,
                           std::string tableName, std::string tableSchema,
                           std::string columnName, std::string value) {
  return (existsPGTable(postgreSQL, schemaName, tableName) == 1 &&
          deletePGRows(postgreSQL, schemaName, tableName, columnName, value)) ||
          createPGTable(postgreSQL, schemaName, tableName, tableSchema);
}

bool createPGIndex(PGconn *postgreSQL, std::string schemaName,
				   std::string tableName, std::string columnName)
{
	std::string query = "CREATE INDEX \"" + tableName + "_" + columnName + "_index\" ON \"" + schemaName + "\".\"" + tableName +
                        "\" (\"" + columnName + "\");";
	return (PQresultStatus(PQexec(postgreSQL,
                                query.c_str())) == PGRES_COMMAND_OK);
}

std::pair <bool, std::string> getPreviousTable(PGconn *postgreSQL,
                                               const std::string schema,
                                               const std::string table) {
  std::ostringstream query;
  PGresult* result;
  query << "SELECT \"table_name\" FROM \"information_schema\".\"tables\" WHERE "
        << "\"table_schema\" = '" << schema << "' AND \"table_name\" < '"
        << table << "' ORDER BY \"table_name\" DESC LIMIT 1";
  result = PQexec(postgreSQL, query.str().c_str());
  if (PQresultStatus(result) != PGRES_TUPLES_OK) {
    PQclear(result);  
    return std::make_pair(false, "");
  }
  else {
    if (PQntuples(result) == 0) {
      return std::make_pair(true, "");
    }
    else {
      return std::make_pair(true, (char*)PQgetvalue(result, 0, 0));
    }
  }
}
