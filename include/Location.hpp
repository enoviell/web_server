#ifndef LOCATION_HPP
#define LOCATION_HPP
#include "include_wrapper.hpp"
#include "HTTPParser.hpp"

class Location {
private:
    std::string path;
    bool auto_index;
    std::map<std::string, std::string> cgi_pass;
    char **env;
    std::string pwd;

public:
    // constructors
    Location();
    Location(std::string path, bool auto_index, std::map<std::string, std::string> cgi_pass);
    ~Location();

    // loaction functions
    std::string autoindex(std::string path, std::string to_find);
    int cgi_handler(std::string file_name, HTTPParser parser, std::string *cgi_response);
    void env_format(HTTPParser request); //, Socket socket
    void clear_env();

    // getters
    std::string get_path();

    // setters
    void set_path(std::string path);
    void set_autoindex(std::string status);
    void set_cgi_pass(std::string cgi);
    void set_pwd(std::string pwd);


    // getters
    std::string get_cgi_pass_val();
    std::string get_path() const;
    bool get_autoindex() const;
    std::string get_pwd()const;
    std::map<std::string, std::string> get_cgi_pass() const;

    class UnableToOpenDir : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "UnableToOpenDir";
        };
    };
};
#endif
