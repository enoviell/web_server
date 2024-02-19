#ifndef CONFIGPARSING_HPP
#define CONFIGPARSING_HPP
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cstdlib>
#include "include_wrapper.hpp"
#include "data_types.hpp"

#define SEPARATORS "\t\n\v\f\r "

class ConfigParsing {
private:
    std::vector<std::string> conf_lines;
    std::vector<std::string>::iterator it;
    std::vector<Tokens> tok_vec;

public:
    // setup
    void conf_parser_wrapper(std::string filename);

    void clear_vector();

    // scanner
    void line_closure(std::string line);
    void line_scanner(size_t cursor);
    void scanner();

    // checking functions
    void check_braces();
    void checkTokens();
    bool special_keywords(std::vector<Tokens>::iterator it);
    int  skip_spaces_to_endl(size_t cursor);
    bool check_prev_and_next_tok_val(std::vector<Tokens>::iterator it);
    bool check_prev_and_next_tok_key(std::vector<Tokens>::iterator it);
    bool check_prev_and_next_tok_endl(std::vector<Tokens>::iterator it);


    // scanner utils
    int skip_spaces(size_t cursor);
    int check_next(size_t cursor);
    std::string get_word(size_t cursor);
    size_t find_separator(size_t cursor) const;

    // token identification
    int identify_token(size_t cursor); //will need adjustment on return type
    static bool is_numeric(std::string word);
    static bool is_alpha_numeric(std::string word);

    // token vector management
    void add_token(std::string content, Token type);

    // getters
    std::vector<Tokens> get_tokens(void) const;
    size_t get_tokens_len() const;

    // Exceptions
    class FileOpeningException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "FileOpeningException";
        }
    };

    class EmptyConfigException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "EmptyConfigException";
        }
    };

    class BadLineClosure : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "BadLineClosure, parenthesis and semicolon closing the same line";
        }
    };

    class InvalidLineClosure : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "NoLineClosure";
        }
    };

    class InvalidConfigSyntax : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "InvalidConfigSyntax";
        }
    };

    class InvalidToken : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "InvalidToken";
        }
    };

    class UneavenNumberOfBraces : public std::exception {
    public:
        virtual const char *what() const throw() {
            return "UneavenNumberOfBraces";
        }
    };
};

#endif //CONFIGPARSING_HPP
