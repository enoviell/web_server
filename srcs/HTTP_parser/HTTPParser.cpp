#include "../../include/HTTPParser.hpp"

/*
 * Constructors
 */
HTTPParser::HTTPParser() {}

HTTPParser::~HTTPParser() {}

/*
 * Verification functions
 */
bool HTTPParser::verify_line_syntax() {
    if (req_lines.size() <= 0)
        return false;

    for (std::vector<std::string>::iterator it = req_lines.begin() + 1 ; it <  req_lines.end() -1; ++it) {
        if (!strncmp(it->c_str(), "GET", 3) || !strncmp(it->c_str(), "POST", 4) || !strncmp(it->c_str(), "DELETE", 6)) {
            ++it;
            continue;
        }
        if (it->find("\r\n") != it->size() - 2)
            return false;
        if (it->find(':') < 1 || it->at(it->find(':') + 1) != ' ')
            return false;
    }
    return true;
}

bool HTTPParser::search_fundamentals(std::string fundamental) {
    for (std::map<std::string, std::string>::iterator it = contents.begin(); it != contents.end(); ++it) {
        if (it->first == fundamental)
            return true;
    }
    return false;
}

bool HTTPParser::verify_fundamentals() {
    const char *fund_init[] = {"METHOD", "PATH", "VERSION"};
    std::vector<std::string> fundamentals(fund_init, fund_init + sizeof(fund_init)/sizeof(fund_init[0]));

    for (std::vector<std::string>::iterator i = fundamentals.begin(); i < fundamentals.end(); ++i) {
        if (!search_fundamentals(*i)) {
            std::cout << "fundamental not found: " << *i << '\n';
            return false;
        }
    }
    return true;
}

/*
 * Tokenization functions
 **/
void HTTPParser::clear_parser() {
    this->contents.clear();
    this->req_lines.clear();
    this->req_body.clear();
}

void HTTPParser::extract_fundamentals(std::string line) {
    long unsigned int pos;
    std::string tmp;

    pos = line.find(' ');
    tmp = line.substr(0, pos);
    contents.insert(std::pair<std::string, std::string>("METHOD", tmp));
    line.erase(0, pos + 1);
    switch (tmp[0])
    {
        case 'G':
            requestId = 0;
            break;
        case 'P':
            requestId = 1;
            break;
        case 'D':
            requestId = 2;
            break;
        default:
            std::cerr << "method not avaiable" << std::endl;
            break;
    }
    pos = line.find(' ');
    tmp = line.substr(0, pos);
    contents.insert(std::pair<std::string, std::string>("PATH", tmp));
    line.erase(0, pos + 1);

    pos = line.find("\r\n");
    tmp = line.substr(0, pos);
    contents.insert(std::pair<std::string, std::string>("VERSION", tmp));
    line.erase(0, std::string::npos);
}

void HTTPParser::tokenizer(std::vector<std::string> request_lines) {
    long unsigned int pos;
    std::string tmp;
    std::string tmp1;

    if (!contents.empty())
        contents.clear();

    extract_fundamentals(request_lines.at(0));

    for (std::vector<std::string>::iterator it = request_lines.begin() + 1; it < request_lines.end(); ++it) {
        pos = it->find(':');
        tmp = it->substr(0, pos);
        it->erase(0, pos + 2);

        pos = it->find("\r\n");
        tmp1 = it->substr(0, pos);
        it->erase(0, std::string::npos);

        if (contents.find(tmp) != contents.end()) {
            for (std::map<std::string, std::string>::iterator it1 = contents.begin(); it1 != contents.end(); ++it1) {
                if (it1->first == tmp) {
                    it1->second.append(", " + tmp1);
                    break;
                }
            }
        } else {
            contents.insert(std::pair<std::string, std::string>(tmp, tmp1));
        }
    }
}

void HTTPParser::split_req_lines(std::string raw_req) {
    std::string temp;
    long unsigned int new_pos;

    while (raw_req.size() >= 2) {
        new_pos = raw_req.find("\r\n");
        temp = raw_req.substr(0, new_pos + 2);
        raw_req.erase(0, new_pos + 2);
        req_lines.push_back(temp);
    }
    temp.clear();
}

void HTTPParser::extract_body(std::string& raw_req) {
    req_body.clear();
    if (raw_req.empty() || raw_req.size() <= 0)
        return;
    int start = raw_req.find("\r\n\r\n");

    req_body = raw_req.substr(start + 4, std::string::npos);
    raw_req.erase(start + 2, std::string::npos);
}

void HTTPParser::tokenizer_wrapper(std::string raw_req) {
    try {
        extract_body(raw_req);
        split_req_lines(raw_req);

        if (!verify_line_syntax())
            throw IncorrectRequestLineSyntaxException();

        tokenizer(req_lines);

        req_lines.clear();
        if (!verify_fundamentals()) {
            std::cout << "\033[96m ↑ \033[0m throwing HTTPPARSER exception\n";
            throw RequestFundamentalsException();
        }
    } catch (const std::exception &exception) {
        std::cerr << "\033[96m ↑ \033[0m Caught an exception while reading the HTTP request: " << exception.what() << '\n';
        throw HTTPParserError400();
    }
}

/*
 * Getters
 */
std::map<std::string, std::string> HTTPParser::get_header() const {
    return this->contents;
}

std::string HTTPParser::get_req_body() const {
    return this->req_body;
}

int HTTPParser::get_max_body_size() const {
    return this->max_body_size;
}

std::string HTTPParser::get_content_by_key(const std::string& key) {
    if (contents.find(key) == contents.end())
        return "";
    return contents[key];
}

/*
 * Setters
 */
void HTTPParser::set_max_body_size(int max_body_size) {
    this->max_body_size = max_body_size;
}

void HTTPParser::clear_map() {
    this->contents.clear();
}
