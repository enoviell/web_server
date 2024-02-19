#include "../../include/AstGeneration.hpp"

/*
 * Constructors
 */
Directives::Directives(std::string val) : directive_val(val) {
}

Section::Section() {}

/*
 * Utils functions
 */
static std::string dir_enum_val_to_string(directives_keyword val) {
    switch (val) {
        case max_clients:
            return "max_clients";
        case _listen :
            return "listen";
        case host :
            return "host";
        case server_name :
            return "server_name";
        case include:
            return "include";
        case _index :
            return "index";
        case root :
            return "root";
        case max_body_size :
            return "max_body_size";
        case autoindex :
            return "autoindex";
        case fastcgi :
            return "fastcgi";
        case methods:
            return "methods";
        case error_page_400 :
            return "error_page_400";
        case error_page_403 :
            return "error_page_403";
        case error_page_404 :
            return "error_page_404";
        case error_page_406 :
            return "error_page_406";
        case error_page_413 :
            return "error_page_413";
        case error_page_500 :
            return "error_page_500";
        case error_page_501 :
            return "error_page_501";
        default: return NULL;
    }
}

static bool is_directive(Tokens token, std::vector<Tokens> tokens, size_t cursor) {
    std::string enum_val;

    // if (token.type == TOKEN_STRING )
    //     check_key_validity(tokens, cursor);

    for (int i = 0; i < 18; ++i) {
        enum_val = dir_enum_val_to_string((directives_keyword)i);
        if (enum_val.empty())
            throw DirectiveKeywordException();
        if (enum_val == token.content)
            return true;
    }
    return false;
}

static std::string sec_enum_val_to_string(sections_keyword val) {
    switch (val) {
    case server:
            return "server";
        case location:
            return "location";
        default:
            return NULL;
    }
}

static std::string get_section_name(std::vector<Tokens> tokens, size_t cursor) {
    std::string enum_val;

    for (int i = 0; i < 2; ++i) {
        enum_val = sec_enum_val_to_string((sections_keyword)i);
        if (tokens[cursor].content == enum_val)
            return enum_val;
    }

    enum_val = sec_enum_val_to_string(location);
    if (tokens[cursor - 1].content == enum_val) {
        return enum_val;
    }
    return "";
}

static void add_directive(std::vector<Tokens> tokens, size_t cursor, ASection *section) {
    // if (tokens[cursor + 1].content == ";")
    //     throw MissingValueForDirective(tokens[cursor].content);

    Directives *new_directive = new Directives(tokens[cursor + 1].content);
    if (Section *s = dynamic_cast<Section *>(section))
        s->block.insert(std::pair<std::string, ASection * >(tokens[cursor].content, new_directive));
}

static void add_location(std::vector<Tokens> tokens, size_t cursor, ASection *section) {
    Directives *new_directive = new Directives(tokens[cursor].content);

    if (Section *s = dynamic_cast<Section *>(section))
        s->block.insert(std::pair<std::string, ASection * >("PATH", new_directive));
}

static void add_section(std::vector<Tokens> tokens, size_t cursor, ASection *section, Section *new_section) { //std::vector<Tokens> tokens, size_t cursor,
    std::string name = get_section_name(tokens, cursor);

    if (name.empty())
        name = "ROOT";

    if (Section *s = dynamic_cast<Section *>(section))
        s->block.insert(std::pair<std::string, ASection * >(name, new_section));

    if (name == "location")
        add_location(tokens, cursor, new_section);
}

/**
 * @param tokens List of tokens
 * @param begin Number of the token where to start
 * @param end Number of the token where to end
 * @param ast Pointer to the Section you are working in (on first call should be the root of the file)
 *
 * @brief transforms the tokens vector into an AST
 */
void tokens_to_ast(std::vector<Tokens> tokens, size_t begin, size_t end, ASection *ast) {
    size_t cursor = begin, curr_section_begin = 0;
    int depth = 0;
    Tokens token;

    while (begin <= end) {
        token = tokens[cursor];
        if (is_directive(token, tokens, cursor) && depth == 0) {
            add_directive(tokens, cursor, ast);
            cursor += 2;
            begin += 2;
            continue;
        }
        if (token.type == TOKEN_OPEN_BRACE) {
            if (depth == 0)
                curr_section_begin = cursor + 1;
            depth++;
        } else if (token.type == TOKEN_CLOSE_BRACE) {
            depth--;
            if (depth == 0) {
                Section *new_section = new Section();
                add_section(tokens, curr_section_begin - 2, ast, new_section);
                tokens_to_ast(tokens, curr_section_begin, cursor - 1, new_section);
            }
        }
        begin++;
        cursor++;
    }
}


static std::string check_prev_section(ASection *prev) {
    if (prev == NULL)
        return "ROOT";
    if (Section *s = dynamic_cast<Section *>(prev)) {
        for (std::multimap<std::string, ASection*>::iterator it = s->block.begin(); it != s->block.end(); ++it) {
            if (it->first == "server")
                return "server";
        }
    }
    return "";
}

static bool is_error_page(std::string key) {
    std::string tmp;

    tmp = key.substr(0, key.find_last_of('_') + 1);

    if (tmp == "error_page_")
        return true;
    return false;
}

static void setup_location(Section *curr, Socket *curr_sock) {
    Location *new_location = new Location;

    for (std::multimap<std::string, ASection *>::iterator it = curr->block.begin(); it != curr->block.end(); ++it) {
        if (Directives *d = dynamic_cast<Directives *>(it->second)) {
            if (it->first == "PATH")
                new_location->set_path(d->directive_val);
            if (it->first == "autoindex")
                new_location->set_autoindex(d->directive_val);
            if (it->first ==  "fastcgi")
                new_location->set_cgi_pass(d->directive_val);
        }
        curr_sock->set_location(new_location);
    }
}

static void setup_server(ASection *curr, WebServer *server) {
    if (Section *s = dynamic_cast<Section *>(curr)) {
        Socket *new_sock = new Socket;
        new_sock->set_max_body_size(8000);
        for (std::multimap<std::string, ASection *>::iterator it = s->block.begin(); it != s->block.end(); ++it) {
            if (Directives *d = dynamic_cast<Directives *>(it->second)) {
                if (it->first == "listen")
                    new_sock->set_tmp_port(std::atoi(d->directive_val.c_str()));
                if (it->first == "host")
                    new_sock->set_tmp_host(d->directive_val);
                if (it->first == "server_name")
                    new_sock->set_server_name(d->directive_val);
                if (it->first == "root")
                    new_sock->set_root(d->directive_val);
                if (it->first == "max_body_size")
                    new_sock->set_max_body_size(std::atoi(d->directive_val.c_str()));
                if (it->first == "methods")
                    new_sock->set_methods(d->directive_val);
                if (it->first == "index")
                    new_sock->set_index(d->directive_val);
            } else if (it->first == "location") {
                if (Section *l = dynamic_cast<Section *>(it->second))
                    setup_location(l, new_sock);
            } else {
                delete new_sock;
                return ;
            }
        }
        server->add_socket(new_sock);
    }
}

void ast_to_settings(ASection *ast, WebServer *server, ASection *prev_section) {
    if (Section *s = dynamic_cast<Section *>(ast)) {
        for (std::multimap<std::string, ASection*>::iterator it = s->block.begin(); it != s->block.end(); ++it) {
            if (it->first == "server") {
                if (Section *s = dynamic_cast<Section *>(it->second))
                    setup_server(it->second, server);
            } else {
                std::string res = check_prev_section(prev_section);
                if (Directives *d = dynamic_cast<Directives *>(it->second)) {
                    if (it->first == "max_clients") {
                        server->set_max_clients(std::atoi(d->directive_val.c_str()));
                        server->get_max_clients();
                    }
                    else if (is_error_page(it->first))
                        server->add_error_page(it->first, d->directive_val);
                }
            }
        }
    }
}
