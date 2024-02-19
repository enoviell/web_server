#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP
#include <string>

enum sections_keyword {
    //sections
    server,
    location,
};

typedef enum {
    //general settings
    max_clients,

    //directives
    _index,
    host,
    _listen,
    server_name,
    include,
    root,
    max_body_size,
    autoindex,
    fastcgi,
    methods,

    //error pages
    error_page_400,
    error_page_403,
    error_page_404,
    error_page_406,
    error_page_413,
    error_page_500,
    error_page_501,
} directives_keyword;

enum Token {
    // Single char
    TOKEN_OPEN_BRACE, TOKEN_CLOSE_BRACE,
    TOKEN_SEMICOLON,

    // Literals
    TOKEN_STRING, TOKEN_NUMBER,

    TOKEN_ERROR
};

struct Tokens {
    Token type;
    std::string content;
    size_t position;
};

#endif
