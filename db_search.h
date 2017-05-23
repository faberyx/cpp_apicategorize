
#ifndef URL_FILTERING_SERVER_LMDATABASE_H
#define URL_FILTERING_SERVER_LMDATABASE_H

// LMDB
#include "lmdbcpp.h"
#include <vector>
#include <algorithm>    // std::sort
#include <fstream>
#include <boost/algorithm/string.hpp>


class db_search {

public:
    db_search(){};
    db_search(const char* database_folder, const char* database_name, const char* badword_filename, const std::string& badword_category);
    void match_url_category(const std::string& searchDomain, std::string& searchPath);
    int result_code;
    std::string result_value;

    void db_close();
    ~db_search(void);
private:
    void initialize_bad_wordlist (const char* badword_filename);
    bool is_bad_word(const std::string &url);
    bool is_ipaddress(const std::string &domain);
    int check_domain(lmdb::cursor& cursor, const std::string &key, std::string &value);
    bool check_path(lmdb::cursor& cursor, const std::string &key, const std::string &value);
    std::vector<std::string> bad_word_list_vector;       // Vector containing list of bad words
    std::string badword_cat;
};
#endif //URL_FILTERING_SERVER_LMDATABASE_H
