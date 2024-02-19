#pragma once
#include "httpResponder.hpp"
#include "include_wrapper.hpp"

class	deleteResponder : public httpResponder
{
public:
    deleteResponder();
    ~deleteResponder();
    void	answer(HTTPParser pars, Location *, WebServer *server, std::string server_name, std::string index);
    int search_delete( std::string to_find);


class UnableToOpenDir : public std::exception {
public:
    virtual const char *what() const throw() {
        return "UnableToOpenDir";
        }
    };
};
