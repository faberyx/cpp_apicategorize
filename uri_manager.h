//
// Created by fabry on 8/6/16.
//

#ifndef MYWS_URI_MANAGER_H
#define MYWS_URI_MANAGER_H

#include <stdio.h>
#include <string>

class uri_manager {

public:
    uri_manager(const std::string &uri);

    bool is_valid_url;
    std::string domain;
    std::string path;

private:

};


#endif //MYWS_URI_MANAGER_H
