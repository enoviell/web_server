#include "../../include/httpResponder.hpp"
#include "../../include/getResponder.hpp"
#include "../../include/postResponder.hpp"
#include "../../include/deleteResponder.hpp"

/*
 * Constructors
 */
httpResponder::httpResponder() {}

httpResponder::~httpResponder() {}

/*
 * Response generation functions
 */
static std::string extrapulate_extension(std::string file_name) {
    size_t it = 0;
    std::string extension = "";

    it = file_name.find('.');
    extension = file_name.substr(it + 1);
    return extension;
}

static std::string generate_content_type(std::string file_name) {
    std::string ext = extrapulate_extension(file_name);

    if (ext == "jpg" || ext == "png")
        return "image/" + ext;
    if (ext == "html")
        return "text/html";
    return "";
}

std::string httpResponder::generateFullAnswer(std::string file_path, std::string _body, std::string to_find, std::string server_name) {
    std::string res;
    std::string content_type_val = generate_content_type(to_find);

    res.append("HTTP/1.1 200 OK\r\n");
    res.append("Host: " + server_name + "\r\n");
    res.append("Content-Type: ");
    res.append(content_type_val);
    res.append("\r\n");
    res.append("Content-Length: ");

    std::ifstream in_file(file_path.c_str(), std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();
    std::stringstream ciao;
    ciao << file_size;
    in_file.close();

    res.append(ciao.str());
    res.append("\r\n\r\n");
    res.append(_body);
    this->full_answer = res;

    return res;
}

std::string httpResponder::getHeaderString() {
    std::string res;
    for (std::map<std::string,std::string>::iterator it = header.begin(); it != header.end(); ++it) {
        if (it->first == "METHOD" || it->first == "PATH" || it->first == "VERSION")
            continue;

        if (it->first == "Accept") {
            res.append("Content-Type");
            res.append(": ");
            res.append("image/jpeg");
            res.append("\r\n");
            continue;
        }

        res.append(it->first);
        res.append(": ");
        res.append(it->second);
        res.append("\r\n");
    }
    return res;
}

static std::string get_error_page_path(std::string error_code, WebServer *server) {
    std::map<std::string, std::string> error_pages_map = server->get_error_pages();

    for (std::map<std::string, std::string>::iterator it = error_pages_map.begin(); it != error_pages_map.end(); it++) {
        if (it->first.find(error_code) != std::string::npos)
            return it->second;
    }
    return "";
}

static std::string generate_error_msg(std::string error_code) {
    if (error_code == "400")
        return "Bad Request";
    if (error_code == "403")
        return "Forbidden";
    if (error_code == "404")
        return "Not Found";
    if (error_code == "405")
        return "Not Acceptable";
    if (error_code == "406")
        return "Method Not Allowed";
    if (error_code == "413")
        return "Payload Too Large";
    if (error_code == "501")
        return "Method Not Implemented";
    return "Internal Server Error";
}

void httpResponder::generate_error(std::string error_code, Location *location, WebServer *server) {
    std::string error_page_path = get_error_page_path(error_code, server);
    std::string error_message = generate_error_msg(error_code);

    if (error_page_path.find(".html") == std::string::npos)
        error_page_path.append(".html");

    this->full_answer.append("HTTP/1.1 " + error_code + " " + error_message + "\r\n");

    this->full_answer.append("Content-Type: text/html\r\n");

    std::ifstream in_file(error_page_path.c_str(), std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();
    std::stringstream ss;
    ss << file_size;
    this->full_answer.append("Content-Length: " + ss.str() + "\r\n");

    this->full_answer.append("\r\n");

    std::ifstream error_page;

    std::stringstream ss1;
    error_page.open(error_page_path.c_str());
    ss1 << error_page.rdbuf();
    this->full_answer.append(ss1.str());
    error_page.close();
}

void httpResponder::clear_full_answer() {
    this->full_answer.clear();
}

/*
 * Setters
 */
void httpResponder::setStatus(int x) {
    this->status = x;
}

void httpResponder::setBody(std::string x) {
    this->body = x;
}

void httpResponder::setHeader(std::string key, std::string value) {
    this->header[key] = value;
}

void	httpResponder::setFullAnswer(std::string _ans) {
    this->full_answer = _ans;
}

/*
 * Getters
 */
int httpResponder::getStatus() {
    return this->status;
}

int httpResponder::getSocket() {
    return socket;
}

std::map<std::string, std::string> httpResponder::getHeader() {
    return this->header;
}

std::string httpResponder::getBody() {
    return this->body;
}

std::string  httpResponder::getFullAnswer() const {
    return this->full_answer;
}
