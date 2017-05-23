#define URL_MATCHED_IP 4
#define URL_ERROR -1
#define URL_MATCHED_WORD 3
#define URL_MATCHED_DOMAIN 2
#define URL_MATCHED_PATH 1
#define URL_NOT_MATCHED 0
#define CATEGORY_SEPARATOR ";"
#define CATEGORY_IP_ADDRESS "ip_address"

#include "db_search.h"

lmdb::env env = lmdb::env::create();    // LMDB environment
lmdb::dbi dbi;                          // LMDB database
lmdb::txn rtxn = lmdb::txn(nullptr);    // LMDB database

//initialize lmdb database
 db_search::db_search(const char* database_folder, const char* database_name,  const char* badword_filename, const std::string& badword_category)
{
    env.set_mapsize(1UL * 1024UL * 1024UL * 1024UL);
    env.set_max_dbs(2);
    env.open(database_folder);

    auto wtxn = lmdb::txn::begin(env);
    dbi = lmdb::dbi::open(wtxn, database_name, MDB_DUPSORT);
    wtxn.commit();

    initialize_bad_wordlist(badword_filename);              // Init badword lists
    badword_cat = badword_category;
    rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);      // Start a new readonly transaction
}

//load in a vector all the words found in the file
void db_search::initialize_bad_wordlist (const char* badword_filename) {

    std::ifstream ifs(badword_filename);
    std::string s;
    while (std::getline(ifs, s)) {
        if (s.size() > 0) {
            bad_word_list_vector.push_back(s);
        }
    }
    std::sort (bad_word_list_vector.begin(), bad_word_list_vector.end()); // Using default sort comparison:
}

//search trough lmdb to matche the domain or the querystring
void db_search::match_url_category(const std::string& searchDomain, std::string& searchPath) {

#ifdef DEBUG
    std::cout << "Search Domain: " << searchDomain << " - Search Path: " << searchPath << std::endl;
#endif
    try {

        // Check if in the domain is an IP address
        if (is_ipaddress(searchDomain)) {
#ifdef DEBUG
            std::cout << "IP address found: " << searchDomain << std::endl;
#endif
            result_code = URL_MATCHED_IP;
            result_value = CATEGORY_IP_ADDRESS;
            return;

        } else {

            lmdb::cursor c = lmdb::cursor::open(rtxn, dbi);                             // Open the cursor
            std::string value;
            int domain_search_result = check_domain(c, searchDomain, value);            // Search by domain
            switch (domain_search_result) {
                case 0:
                    if (is_bad_word(searchPath)) {                                      // No matches found so check if in the url  there is a banned word
#ifdef DEBUG
                        std::cout << "Bad word found in URL: " << badword_cat << std::endl;
#endif
                        result_code = URL_MATCHED_WORD;
                        result_value = badword_cat;
                        return;
                    }else {
                        result_code = URL_NOT_MATCHED;                                  // Key not present in database
                        result_value = value;

                    }
                    break;
                case 1:
                    result_code = URL_MATCHED_DOMAIN;
                    result_value = value;                                               // Key present in database - domain matched
                    break;
                case 2:
                    if (check_path(c, searchDomain, searchPath)) {
                        result_code = URL_MATCHED_DOMAIN;
                        result_value = value;                                           // Key present in database - domain matched
                    }else{
                        result_code = URL_MATCHED_PATH;                                 // Key present in database - domain matched
                        result_value = value;                                           // but also need a match check of the url
                    }
                    break;
            }
            c.close();
            return;
        }
    } catch (std::exception &e) {
        result_code = URL_ERROR;
        result_value = e.what();
        return;
    }
}

//  Search for a domain and send a response after a domain->key search
bool db_search::check_path(lmdb::cursor& cursor, const std::string& key, const std::string &value) {

    lmdb::val k { key.c_str()};
    lmdb::val v { value.c_str()};

    bool search = cursor.get(k, v, MDB_GET_BOTH_RANGE);         // Move the cursor to the nearest key/value
#ifdef DEBUG
    std::cout << "K-V:" << k.data() << " : " << v.data() << " - MDB_GET_BOTH_RANGE search result: " << search << std::endl;
#endif
    if (search) {
        std::string kk,vv;
        cursor.get(kk, vv, MDB_GET_CURRENT);                    // Retrieve the current row from the cursor

        std::string url = vv.substr(0, vv.find_last_of(":"));
#ifdef DEBUG
        std::cout << "search found -> url: " << url  <<std::endl;
#endif
        if (value.find(url) != std::string::npos) {             // If the value of the row match then found
            return true;
        }
    }
    return false;                                               // Match not found
}

// Search for a domain and send a response after a domain->key search
// return 0 -> no match found for key send response no found
// return 1 -> match found for key and key has also values (paths) -> start a search also for paths
// return 2 -> match found for key and key has also values (paths) -> start a search also for paths
int db_search::check_domain(lmdb::cursor& cursor, const std::string &key, std::string &value) {

    lmdb::val k{key.c_str()};
    lmdb::val v;
    int retval = 0;
    bool domain_found = cursor.get(k, v, MDB_SET_KEY);          // Search key by domain only
    if (domain_found) {
        retval = 1;                                             // Domain found - no url
        std::string domain_val(v.data());
        value = domain_val.substr(0, v.size()).substr(2);       // Clean string from unwanted chars
#ifdef DEBUG
        std::cout << "First value found:" << value << std::endl;
#endif
        while (cursor.get(k, v, MDB_NEXT_DUP)) {                // Loop trough all domain keys ("*:[category]")
            if (strlen(v.data()) > 0 && v.data()[0] != '*') {   // If result is not a domain exit
                retval = 2;                                     // Domain found - check also for path in url
                break;
            }
            std::string domain_secondary_val(v.data());
            domain_secondary_val = domain_secondary_val.substr(0, v.size()).substr(2);

            value = value.append(CATEGORY_SEPARATOR).append(domain_secondary_val); // Append value to existing categories
#ifdef DEBUG
            std::cout << "Next value found:" << value << std::endl;
#endif
        }
        return retval;
    } else {
        return retval;
    }
}


bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool db_search::is_ipaddress(const std::string &domain) {

    std::vector<std::string> strs;
    boost::split(strs, domain, boost::is_any_of("."));
    return strs.size() == 4
                && is_number(strs.at(0))
                && is_number(strs.at(1))
                && is_number(strs.at(2))
                && is_number(strs.at(3));
}

//check in the vector of a word is matched in the url
bool db_search::is_bad_word(const std::string &url){ // Check if the url contains a bad word

    std::string found_target;  // Set to the target we found, if we found any
    for(std::vector<std::string>::const_iterator it = bad_word_list_vector.begin(); found_target.empty() && (it != bad_word_list_vector.end()); ++it )
    {
        if( url.find(*it) != std::string::npos )
            found_target = *it;
    }
    return  !found_target.empty();
}

void db_search::db_close()
{
    rtxn.abort();
    env.close();
}

db_search::~db_search(void) {
    db_search::db_close();
}
