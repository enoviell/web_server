#ifndef CONFIG_PARSING_SECTION_HPP
#define CONFIG_PARSING_SECTION_HPP

#include "include_wrapper.hpp"
#include "data_types.hpp"
#include "WebServer.hpp"

class ASection {
public:
    virtual void f() const = 0;
    virtual ~ASection(){};
};

class Section : public ASection{
public:
   ~Section() {}
    std::multimap<std::string, ASection *> block;
    Section();
    virtual void f() const {};
};

class Directives : public ASection {
public:
    ~Directives(){};
    std::string directive_val;
    Directives(std::string val);
    virtual void f() const {};
};

class DirectiveKeywordException : public std::exception {
public:
    virtual const char *what() const throw() {
        return "DirectiveKeywordException";
    }
};

class DirectiveValueException : public std::exception {
public:
    virtual const char *what() const throw() {
        return "DirectiveValueException";
    }
};

class InvalidDirective : public std::exception {
public:
    virtual const char *what() const throw() {
        return "DirectiveValueException";
    }
};


void tokens_to_ast(std::vector<Tokens> tokens, size_t begin, size_t end, ASection *ast);
void ast_to_settings(ASection *ast, WebServer *server, ASection *prev_section);

#endif
