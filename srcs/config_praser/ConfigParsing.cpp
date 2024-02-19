#include "../../include/ConfigParsing.hpp"

void ConfigParsing::clear_vector() {
     this->tok_vec.clear();
     this->conf_lines.clear();
}

/*
 * Token management function
 */
void ConfigParsing::add_token(std::string content, Token type) {
    Tokens newElement;

    newElement.content = content;
    newElement.type = type;
    newElement.position = tok_vec.size();
    tok_vec.push_back(newElement);
}

/*
 * Scanner utils functions
 */
int ConfigParsing::check_next(size_t cursor) {
    return it->at(cursor + 1);
}

int ConfigParsing::skip_spaces(size_t cursor) {
    int tmp;

    for (;;) {
        tmp = it->at(cursor);
        switch (tmp) {
            case ' ':
            case '\r':
            case '\t':
                cursor++;
                break;
            case '\n':
                return it->size() - 1;
            case '#':
                if (check_next(cursor) == '#') {
                    return it->size() - 1;
                }
                else
                    return cursor;
            default: return cursor;
        }
    }
}

size_t ConfigParsing::find_separator(size_t cursor) const {
    std::string sep = SEPARATORS;
    const size_t cursor_beg = cursor;
    size_t find_pos;

    for (; cursor < it->size(); cursor++) {
        for (size_t i = 0; i < sep.length() - 1; ++i) {
            find_pos = sep.find(it->at(cursor));
            if (find_pos != std::string::npos)
                return cursor - cursor_beg;
        }
    }
    return std::string::npos;
}

std::string ConfigParsing::get_word(size_t cursor)  {
    const size_t sep = find_separator(cursor);

    if (sep == std::string::npos)
        return NULL;
    std::string res;
    if (it->find(';') == it->size() - 1)
        res = it->substr(cursor, sep - 1);
    else
        res = it->substr(cursor, sep);
    return res;
}

/*
 * Token identification
 */
bool ConfigParsing::is_numeric(std::string word) {
    for (size_t i = 0; i < word.size(); ++i) {
        if (word.at(i) < 48 || word.at(i) > 57)
            return false;
    }
    return true;
}

bool ConfigParsing::is_alpha_numeric(std::string word) {
    for (size_t i = 0; i < word.size(); ++i) {
        if ((word.at(i) >= 0 && word.at(i) <= 32) || word.at(i) == 59 || word.at(i) == 127
            || word.at(i) == 125 || word.at(i) == 123)
            return false;
    }
    return true;
}

int ConfigParsing::identify_token(size_t cursor) {
    std::string word = "";
    word = get_word(cursor);

    if (word.empty()) {
        return cursor += 1;
    }

    if (word.find(';') == word.size() - 1)
        word = word.substr(0, word.length() - 1);

    if (it->at(cursor) == ';') {
        add_token(";", TOKEN_SEMICOLON);
        return cursor + 2;
    }

    if (is_numeric(word)) {
        add_token(word, TOKEN_NUMBER);
        return cursor + word.length();
    }

    if (is_alpha_numeric(word)) {
        add_token(word, TOKEN_STRING);
        return cursor + word.length();
    }

    if (word == "{") {
        add_token(word, TOKEN_OPEN_BRACE);
        return cursor + word.length() + 1;
    }

    if (word == "}") {
        add_token(word, TOKEN_CLOSE_BRACE);
        return cursor + word.length() + 1;
    }

    add_token(word, TOKEN_ERROR);
    return cursor + word.length() + 1;
}

/*
 * Scanner
 */
void ConfigParsing::line_scanner(size_t cursor) {
    for (cursor = 0; cursor < it->size() - 1;) {
        cursor = skip_spaces(cursor);
        cursor = identify_token(cursor);
    }
}

int ConfigParsing::skip_spaces_to_endl(size_t cursor) {
    int tmp;

    for (;;) {
        tmp = it->at(cursor);
        switch (tmp) {
            case ' ':
            case '\r':
            case '\t':
                cursor++;
            break;
            case '\n':
                return it->length();
            case '#':
                if (check_next(cursor) == '#') {
                    return it->size() - 1;
                }
                else
                    return cursor;
            default: return cursor + 1;
        }
    }
}



void ConfigParsing::line_closure(std::string line) {
    if ((line.find('{') != std::string::npos || line.find('}') != std::string::npos) && line.find(';') != std::string::npos) {
        throw BadLineClosure();
    }
    if (skip_spaces(0) == line.size() - 1)
        return;

    size_t pos;

    if (line.find_last_of('{') == std::string::npos && line.find_last_of('}') == std::string::npos && line.find_last_of(';') == std::string::npos)
        throw InvalidLineClosure();

    if ((pos = line.find_last_of('{')) != std::string::npos && !line.empty()) {
        if (skip_spaces_to_endl(pos + 1) != line.size())
            throw InvalidLineClosure();
    }

    if ((pos = line.find_last_of('}')) != std::string::npos && !line.empty()) {
        if (skip_spaces_to_endl(pos + 1) != line.size())
            throw InvalidLineClosure();
    }

    if ((pos = line.find_last_of(';')) != std::string::npos && !line.empty()) {
        if (skip_spaces_to_endl(pos + 1) != line.size())
            throw InvalidLineClosure();
    }
}

void ConfigParsing::scanner() {
    it = conf_lines.begin();
    size_t cursor = 0;

    try {
        for (it = conf_lines.begin(); it < conf_lines.end(); ++it) {
            line_closure(*it);
            line_scanner(cursor);
        }
    } catch (std::exception &exception) {
        std::cerr << "\033[96m ↑ \033[0m Caught an exception while scanning line " << std::distance(conf_lines.begin(), it) + 1 <<": " << exception.what() << '\n';
        throw InvalidConfigSyntax();
    }
}

/*
 * Check Tokens
 */
void ConfigParsing::check_braces() {
    int open = 0, close = 0;

    for (std::vector<Tokens>::iterator it = tok_vec.begin(); it != tok_vec.end(); ++it) {
        if ((*it).type == TOKEN_OPEN_BRACE)
            open++;
        if ((*it).type == TOKEN_CLOSE_BRACE)
            close++;
    }

    if (close != open)
        throw UneavenNumberOfBraces();
}

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

static bool is_valid_keyword(std::string str) {
    std::string enum_val;

    for (int i = 0; i < 18; ++i) {
        enum_val = dir_enum_val_to_string((directives_keyword)i);
        if (enum_val == str)
            return true;
    }
    // check for existence in section_keyword
    return false;
}

// function to check next and prev token for a token that contain a value
bool ConfigParsing::check_prev_and_next_tok_val(std::vector<Tokens>::iterator it) {
    --it;

    // prev token should not be a semicolon, open brace or closed brance
    if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
        && (*it).type != TOKEN_CLOSE_BRACE) {
        return false;
    }
    it += 2;
    // next token should be a semicolon, open brace or closed brace
    if ((*it).type == TOKEN_SEMICOLON && (*it).type == TOKEN_OPEN_BRACE
        && (*it).type == TOKEN_CLOSE_BRACE) {
        return false;
    }
    return true;
}

// function to check next and prev token for a token that contain a key
bool ConfigParsing::check_prev_and_next_tok_key(std::vector<Tokens>::iterator it) {
    --it;
    // prev token should be a semicolon, open brace or closed brace
    if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
        && (*it).type != TOKEN_CLOSE_BRACE) {
        return false;
    }
    it += 2;
    // next token should not be a semicolon, open brace or closed brace
    if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
        && (*it).type != TOKEN_CLOSE_BRACE) {
        return false;
    }
    return true;
}

// function to check next and prev token for a token that contain a semicolon, open brace or closed brace
bool ConfigParsing::check_prev_and_next_tok_endl(std::vector<Tokens>::iterator it) {
    std::vector<Tokens>::iterator curr = it;

    --it;
    // prev token should not be a semicolon, open brace or closed brace
    if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
        && (*it).type != TOKEN_CLOSE_BRACE) {
        return false;
    }
    it += 2;
    // next token should not be a semicolon, open brace but could be a "double" closed brace ('}}')
    if ((*it).type == TOKEN_SEMICOLON && (*it).type == TOKEN_OPEN_BRACE
        && (*it).type == TOKEN_CLOSE_BRACE) {
        if (curr->type == TOKEN_CLOSE_BRACE && (*it).type == TOKEN_CLOSE_BRACE)
            return true;
        return false;
    }
    return true;
}

// special case for server and location keywords
bool ConfigParsing::special_keywords(std::vector<Tokens>::iterator it) {
    if ((*it).content == "server") {
        --it;
        // prev token should be a semicolon, open brace or closed brace
        if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
            && (*it).type != TOKEN_CLOSE_BRACE) {
            throw InvalidToken();
        }
        it += 2;
        // next token should be an open brace or closed brace
        if ((*it).type != TOKEN_OPEN_BRACE && (*it).type != TOKEN_CLOSE_BRACE) {
            throw InvalidToken();
        }
        return true;
    }

    if ((*it).content == "location") {
        --it;
        // prev token should be a semicolon, open brace or closed brace
        if ((*it).type != TOKEN_SEMICOLON && (*it).type != TOKEN_OPEN_BRACE
            && (*it).type != TOKEN_CLOSE_BRACE) {
                throw InvalidToken();
        }
        it += 2;
        // next token should not be an open brace or closed brace
        if ((*it).type == TOKEN_SEMICOLON && (*it).type == TOKEN_OPEN_BRACE
            && (*it).type == TOKEN_CLOSE_BRACE) {
                throw InvalidToken();
        }
        return true;
    }
    return false;
}

// check validity of tokens
void ConfigParsing::checkTokens() {
    check_braces();
    for (std::vector<Tokens>::iterator it = tok_vec.begin(); it != tok_vec.end(); ++it) {
        // check validity of delimiter
        if (((*it).type == TOKEN_SEMICOLON || (*it).type == TOKEN_CLOSE_BRACE || (*it).type == TOKEN_OPEN_BRACE) &&
                check_prev_and_next_tok_endl(it)) {
            continue;
        }
        // handle keywords: server and location
        if (special_keywords(it)) {
            continue;
        }
        // first token case
        if (it == tok_vec.begin()) {
            if (!is_valid_keyword((*it).content))
                throw InvalidToken();
        }
        // last token case
        if (it == tok_vec.end()) {
            if ((*it).type == TOKEN_STRING || (*it).type != TOKEN_NUMBER)
                throw InvalidToken();
        }
        // case for tokens that contains a value
        if (!is_valid_keyword((*it).content) && check_prev_and_next_tok_val(it)) {
                throw InvalidToken();
        }
        // case for tokens that contains a keyword
        if (is_valid_keyword((*it).content) && check_prev_and_next_tok_key(it)) {
            throw InvalidToken();
        }
    }
}

/*
 * Setup
 */
void ConfigParsing::conf_parser_wrapper(std::string filename) {
    std::ifstream file;
    std::string line;

    file.open(filename.c_str());
    if (!file.is_open())
        throw FileOpeningException();

    while (getline(file, line)) {
        line.append("\n");
        conf_lines.push_back(line);
    }
    if (this->conf_lines.empty())
        throw EmptyConfigException();
    file.close();
    scanner();
    checkTokens();
}

/*
 * Getters
 */
std::vector<Tokens> ConfigParsing::get_tokens() const {
    return this->tok_vec;
}

size_t ConfigParsing::get_tokens_len() const {
    return this->tok_vec.size();
}
