///------------------------------------------------------------------------------------------------
/// clang++ -std=c++14 -I /var/lib/include *.cc -llmdb -lboost_system -lmicrohttpd -o webserver
///------------------------------------------------------------------------------------------------

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

//Sylicon
#include <silicon/api.hh>
#include <silicon/backends/mhd.hh>
#include <silicon/api_description.hh>
#include "symbols.hh"

//Internal
#include "uri_manager.h"
#include "db_search.h"

using namespace sl; // Silicon namespace
using namespace s; // Symbols namespace

// Database class
db_search data_lmdb;


//   ------------------- REST SERVICE -----------------------
// Define the URL Filter API:
auto filter_api = http_api(

  // Filter Rest class url as a querystring parameter
  GET / _filter * get_parameters(_url) = [] (auto p) {

      uri_manager uri (p.url);  // Process the querystring and divide it by domain/path

      if(!uri.is_valid_url){    // Check if the URL in the parameter is valid
          throw error::bad_request("invalid url");
      }else {
          //Process the URL with LMDB Database
          data_lmdb.match_url_category(uri.domain, uri.path);
          //Return search results
          return  D(_code = data_lmdb.result_code, _result = data_lmdb.result_value);
      }
  }
);

//   ------------------- MAIN -----------------------
int main(int argc, char *argv[]) {

    boost::property_tree::ptree pt;         // Load settings from INI
    boost::property_tree::ini_parser::read_ini("url_filtering.ini", pt);
                                                /// /etc/urlfiltering/url_filtering.ini", pt);
    int port = pt.get<int>("server.port");
    // Check if there is a host from command line
    if(argc == 2){
        port = atoi(argv[1]);
    }
    std::string badword_list_file = pt.get<std::string>("server.badwords_list_file");
    std::string badword_category = pt.get<std::string>("server.badwords_category");
    std::string dbname = pt.get<std::string>("lmdb.db_name");
    std::string dbfolder = pt.get<std::string>("lmdb.db_folder");

    //init lmdb data
    db_search lm_data (dbfolder.c_str(), dbname.c_str(), badword_list_file.c_str(), badword_category);

    data_lmdb = lm_data;

    // Serve api via microhttpd using the json format
    std::cout << api_description(filter_api) << std::endl;
    auto srv_mhd = sl::mhd_json_serve(filter_api, port, _non_blocking);

    std::string line;
    std::cout << "Hit Enter to stop the listener.";
    std::getline(std::cin, line);

    lm_data.db_close();
}
