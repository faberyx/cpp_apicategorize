//
// Created by fabry on 8/6/16.
//

#include "uri_manager.h"


// Convert an URL to a domain / path
// Returns true if the url is valid
 uri_manager::uri_manager(const std::string &url) {


    bool start_domain = 0, end_domain = 0, start_path = 0, end_path = 0;

    for(unsigned int i = 0; i<url.length(); i++) {                          // Cycle through all chars

        if(i>4 && url[i] == '/' && url[i-1] == '/' && url[i-2] == ':'){     // Found the start of domain
            if(url.length() > 10 && url[i+1] == 'w' && url[i+2] == 'w' && url[i+3] == 'w' && url[i+4]){
                i= i+4;                                                     // discard wwww from domain
            }
            start_domain = 1;
        }else{
            if(start_domain && url[i] == '/'){                              // Found end of Domain and beginning of path
                end_domain = 1;
                start_path = 1;
            }
            if(start_domain && (url[i] == '?' || url[i] == '#')){           // Querystrings are discarded
                break;
            }
            if(start_domain && !end_domain){
                domain += url[i];                                           // Print Domain
            }
            if(start_path && !end_path){
                path += url[i];                                             // Print Path
            }
        }
    }

    is_valid_url = domain != "";
}