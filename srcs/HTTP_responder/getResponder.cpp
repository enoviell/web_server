#include "../../include/getResponder.hpp"
#include "../../include/Location.hpp"
#include <fstream>

/*
 * Constructors
 */
getResponder::getResponder() : httpResponder() {}

getResponder::~getResponder() {}

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

static int check_for_error_page_req(std::string uri, WebServer *server) {
    std::map<std::string, std::string> tmp_err_pages = server->get_error_pages();
    for (std::map<std::string, std::string>::iterator it = tmp_err_pages.begin(); it != tmp_err_pages.end(); ++it) {
        if (it->first == uri)
            return std::atoi(uri.substr(it->first.size() - 3).c_str());
    }
    return 0;
}

/*
 * Response construction functions
 */
static std::string get_response_body(std::string *cgi_response) {
    if (cgi_response->empty() || cgi_response->size() <= 0)
        return "";
    const size_t start = cgi_response->find("\r\n\r\n");

    if (start == std::string::npos)
        return "";
    std::string res = cgi_response->substr(start + 4, std::string::npos);
    cgi_response->erase(start + 2, std::string::npos);
    return res;
}

static std::vector<std::string>response_header_parse(std::string cgi_response) {
    std::vector<std::string> headers_vec;
    int pos;
    std::string header;
    bool type = false, length = false;

    for (size_t i = 0; i < cgi_response.size();) {
        pos = cgi_response.find("\r\n");
        header = cgi_response.substr(0, pos + 2);
        cgi_response.erase(0, pos + 2);
        headers_vec.push_back(header);
    }

    for (std::vector<std::string>::iterator it = headers_vec.begin(); it != headers_vec.end(); ++it) {
        if (it->find("Content-Type: ") != std::string::npos)
            type = true;
        if (it->find("Content-Length: ") != std::string::npos)
            length = true;
    }
    if ((type && length))
        return headers_vec;
    headers_vec.clear();
    return headers_vec;
}

/*
 * Implementation parent virtual function
 */
void	getResponder::answer(HTTPParser parser, Location *location, WebServer *server, std::string server_name, std::string index) {
    std::string requestd_file = parser.get_content_by_key("PATH");
    std::ifstream test;
    std::string file_path;
    std::string ext;
    size_t pos;
    int err_code;

    if (requestd_file == "/")
        requestd_file = index;

    if (requestd_file[0] == '/')
        requestd_file.erase(0, 1);

    if ((err_code = check_for_error_page_req(requestd_file, server))) {
        std::stringstream ss;
        ss << err_code;
        this->generate_error(ss.str(), location, server);
        ss.clear();
    }

    if ((pos = requestd_file.find('=')) != std::string::npos)
        requestd_file = requestd_file.substr(pos + 1, std::string::npos);

    ext = extrapulate_extension(requestd_file);

    if (ext == "py")
        file_path = location->autoindex(location->get_cgi_pass_val(), requestd_file);
    else
        file_path = location->autoindex(location->get_path(), requestd_file);

    if (file_path.empty()) {
        this->generate_error("404", location, server);
        return;
    }

    if (!file_path.empty()) {
        if (ext == "py") {
            std::string cgi_response;
            if (location->cgi_handler(file_path, parser, &cgi_response) == 0) {
                std::string final_answer;
                std::string response_body = get_response_body(&cgi_response);
                std::vector<std::string>response_headers = response_header_parse(cgi_response);

                if (response_headers.empty() || response_body.empty()) {
                    generate_error("500", location, server);
                    return;
                }
                final_answer.append("HTTP/1.1 200 OK\r\n");
                final_answer.append("Host: " + server_name + "\r\n");
                for (std::vector<std::string>::iterator it = response_headers.begin(); it != response_headers.end(); ++it) {
                    final_answer.append(*it);
                }
                final_answer.append("\r\n\r\n");
                final_answer.append(response_body);
                setFullAnswer(final_answer);
                return;
            } else {
                this->generate_error("500", location, server);
                return;
            }
        }
        if (ext == "jpg" || ext == "png") {
            std::string dio;
            std::ifstream myfile (file_path.c_str(), std::ios::binary);
            std::stringstream  buffer;
            buffer<<myfile.rdbuf();
            dio=buffer.str();
            setBody(dio);
            std::string res = generateFullAnswer(file_path, dio, requestd_file, server_name);
            myfile.close();
            buffer.clear();
            dio.clear();
            return;
        } else {
            test.open(file_path.c_str());
            std::stringstream buff;
            buff<<test.rdbuf();
            std::string _body = buff.str();
            test.close();
            setBody(_body);
            std::string res = generateFullAnswer(file_path, _body, requestd_file, server_name);
            buff.clear();
            _body.clear();
            return;
        }
    }
}
