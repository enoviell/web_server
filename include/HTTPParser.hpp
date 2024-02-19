#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "include_wrapper.hpp"

class HTTPParser {
private:
    std::map<std::string, std::string> contents;
    std::vector<std::string> req_lines;
    std::string req_body;
    int max_body_size;

public:
    HTTPParser();
    ~HTTPParser();
    int requestId;
    // tokenization functions
    void    extract_body(std::string& raw_req);
    void    split_req_lines(std::string raw_req);
    void    tokenizer_wrapper(std::string raw_req);

    // tokenization functions to rework
    void    tokenizer(std::vector<std::string> request_lines);
    void    extract_fundamentals(std::string line);

    // utils functions
    void    print_map();
    std::string get_content_by_key(const std::string& key);

    // verification functions
    bool    search_fundamentals(std::string);
    bool    verify_line_syntax();
    bool    verify_fundamentals();
    void    clear_map();
    void    clear_parser();

    // getters
    std::map<std::string, std::string> get_header() const;
    std::string get_req_body() const;
    int get_max_body_size() const;

    //setters
    void set_max_body_size(int max_body_size);

    // exceptions
    class IncorrectRequestLineSyntaxException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "IncorrectRequestLineSyntax";
        }
    };

    class RequestFundamentalsException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "RequestFundamentalsException \nNot all fundamentals values has been correctly passed/read in the HTTP request";
        }
    };

    class HTTPParserError400 : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "HTTPParserError400";
        }
    };
};

#endif //HTTP_PARSER_HPP
