#include "../../include/Location.hpp"

/*
 * Constructors
 */
Location::Location() {}

Location::Location(std::string path, bool auto_index, std::map<std::string, std::string> cgi_pass) :
path(path), auto_index(auto_index), cgi_pass(cgi_pass)
{}

Location::~Location() {}

/*
 * Autoindex
 */
std::string Location::autoindex(std::string path, std::string to_find) {
    DIR *dir;
    std::string path_cpy = path;
    struct dirent *ent;
    std::string ret = "";

    if (to_find.find('.') == std::string::npos)
        return "";

    if (to_find[0] == '/')
        to_find.erase(0, 1);

    if (path.at(path.size() - 1) != '/')
        path.append("/");

    // if autoindex is off will use this block to search in the path received
    // by the request instead of recursively searching in subdirs
    if (!this->auto_index) {
        if ((dir = opendir(path.c_str())) == NULL)
            return "";
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            if (ent->d_name == to_find) {
                ret = path + ent->d_name;
                closedir(dir);
                return ret;
            }
        }
        closedir(dir);
        return "";
    } else {
        if ((dir = opendir(path.c_str())) == NULL)
            return "";
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            if (ent->d_type == DT_DIR) {
                ret = autoindex(path + ent->d_name + '/', to_find);
                if (!ret.empty()) {
                    closedir(dir);
                    return ret;
                }
            }
            if (ent->d_name == to_find) {
                ret = path + ent->d_name;
                closedir(dir);
                return ret;
            }
        }
        closedir(dir);
        return "";
    }
}

/*
 * CGI Functions
 */
static std::string port_to_string(uint16_t port) {
    std::ostringstream conv;

    conv << port;
    return conv.str();
}

void Location::clear_env() {
    int i = 0;
    while (this->env[i]) {
        delete[] this->env[i];
        i++;
    }
    delete[] this->env;
}

void Location::env_format(HTTPParser request) {
    std::map<std::string, std::string> tmp_env;
    uint32_t port_bo = htons(6969);
    std::string host = request.get_content_by_key("Host");
    host = host.substr(0, host.find(':'));
    std::string port = port_to_string(ntohs(port_bo));
    std::string to_convert;

    tmp_env["CONTENT_LENGTH"] = request.get_content_by_key("Content-Length");
    tmp_env["CONTENT_TYPE"] = request.get_content_by_key("Content-Type");
    tmp_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    tmp_env["HTTP_COOKIE"] = "";
    tmp_env["PATH_INFO"] = get_cgi_pass_val() + request.get_content_by_key("PATH").substr(1);
    tmp_env["PATH_TRANSLATED"] = get_cgi_pass_val() + request.get_content_by_key("PATH").substr(1);
    tmp_env["QUERY_STRING"] = "";
    tmp_env["REDIRECT_STATUS"] = "200";
    tmp_env["REMOTE_IDENT"] = "";
    tmp_env["REMOTE_USER"] = "";
    tmp_env["REMOTEaddr"] = host;
    tmp_env["REQUEST_METHOD"] = request.get_content_by_key("METHOD");
    tmp_env["REQUEST_URI"] =  this->get_cgi_pass_val() + request.get_content_by_key("PATH").substr(1);
    tmp_env["SCRIPT_FILENAME"] = this->get_cgi_pass_val() + request.get_content_by_key("PATH").substr(1);
    tmp_env["SCRIPT_NAME"] = this->get_cgi_pass_val() + request.get_content_by_key("PATH").substr(1);
    tmp_env["SERVER_NAME"] = host;
    tmp_env["SERVER_PORT"] = port;
    tmp_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    tmp_env["SERVER_SOFTWARE"] = "Webserv/1.0";
    tmp_env["UPLOAD_PATH"] = this->pwd + "/uploads/";

    env = new char*[tmp_env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = tmp_env.begin(); it != tmp_env.end(); ++it) {
        std::string to_convert = it->first + '=' + it->second;
        env[i] = new char[to_convert.size() + 1];
        std::strcpy(env[i], to_convert.c_str());
        to_convert.clear();
        ++i;
    }
    env[i] = NULL;
    tmp_env.clear();
    to_convert.clear();
    port.clear();
    host.clear();
}

static char *command_selector(std::string file_ext) {
    if (file_ext == "py")
        return (char *)"python3";
    if (file_ext == "php")
        return (char *)"php";
    return (char *)"";
}

static char *get_cmd_path(std::string file_path) {
    std::string file_ext = file_path.substr(file_path.find_last_of('.') + 1, std::string::npos);
    std::string cmd = command_selector(file_ext);
    char *ret;
    std::string path_to_cmd = "/usr/local/bin/" + cmd;

    if (access(path_to_cmd.c_str(), F_OK))
        return NULL;

    ret = new char[path_to_cmd.size() + 1];
    std::strcpy(ret, path_to_cmd.c_str());
    ret[path_to_cmd.size()] = 0;

    file_ext.clear();
    cmd.clear();
    path_to_cmd.clear();

    return ret;
}

static char **args_format(std::string file_path) {
    std::string file_ext = file_path.substr(file_path.find_last_of('.') + 1, std::string::npos);
    std::string cmd = command_selector(file_ext);
    std::string test1 = "/usr/local/bin/python3";
    char **ret;

    ret = new char *[3];

    ret[0] = new char[test1.size() + 1];
    std::strcpy(ret[0], test1.c_str());
    ret[0][test1.size()] = 0;

    ret[1] = new char[file_path.size() + 1];
    std::strcpy(ret[1], file_path.c_str());
    ret[1][file_path.size() + 1] = 0;

    ret[2] = NULL;

    file_ext.clear();
    cmd.clear();
    test1.clear();

    return ret;
}


int Location::cgi_handler(std::string file_path, HTTPParser request, std::string *cgi_response) {
    pid_t pid;
    env_format(request);

    std::string newBody;
    int ret = 1;
    int saveStdin = dup(STDIN_FILENO);
    int saveStdout = dup(STDOUT_FILENO);
    FILE* fileIn = tmpfile();
    FILE* fileOut = tmpfile();
    int fdIn = fileno(fileIn);
    int fdOut = fileno(fileOut);

    write(fdIn, request.get_req_body().c_str(), request.get_req_body().size());
    lseek(fdIn, 0, SEEK_SET);

    pid = fork();

    if (pid < -1) {
        std::cout << "FORK FAILED\n";
        return -1;
    }

    char **args1 = args_format(file_path);
    char *path_to_cmd = get_cmd_path(file_path);

    if (pid == 0) {
        dup2(fdIn, STDIN_FILENO);
        dup2(fdOut, STDOUT_FILENO);

        if (execve(path_to_cmd, args1, this->env) != 0) {
            perror("execve failed");
            clear_env();
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);

        clear_env();

        char	buffer[65536] = {0};
        lseek(fdOut, 0, SEEK_SET);
        while (ret > 0) {
            memset(buffer, 0, 65536);
            ret = read(fdOut, buffer, 65536 - 1);
            newBody += buffer;
        }

        dup2(saveStdin, STDIN_FILENO);
        dup2(saveStdout, STDOUT_FILENO);
        fclose(fileIn);
        fclose(fileOut);
        close(fdIn);
        close(fdOut);
        close(saveStdin);
        close(saveStdout);

        cgi_response->append(newBody);
        newBody.clear();

        int i=-1;
        while(args1[++i])
            delete[] args1[i];
        delete[] args1;
        delete[] path_to_cmd;
        if (WIFEXITED(status)) {
            std::cout << "\033[96m" << "Location: \033[0m" << "Child process exited with status: " << WEXITSTATUS(status) << '\n';
            return 0;
        } else {
            clear_env();
            int i=-1;
            while(args1[++i])
                delete[] args1[i];
            delete[] args1;
            delete[] path_to_cmd;
            std::cout << "\033[31m" << "Child process did not exit normally" << "\033[0m " << '\n';
            return -1;
        }
    }
    clear_env();
    int j=-1;
    while(args1[++j])
        delete[] args1[j];
    delete[] args1;
    delete[] path_to_cmd;
    return -1;
}

/*
 * Setters
 */
void Location::set_path(std::string path) {
    this->path = path;
}

void Location::set_autoindex(std::string status) {
    if (status[0] == 'o' && status[1] == 'n')
        this->auto_index = true;
    else
        this->auto_index = false;
}

void Location::set_cgi_pass(std::string cgi_string) {
    std::string ext = cgi_string.substr(0, cgi_string.find('|'));
    std::string cgi_path = cgi_string.substr(cgi_string.find('|') + 1, std::string::npos);

    this->cgi_pass.insert(std::pair<std::string, std::string>(ext, cgi_path));
}

void Location::set_pwd(std::string pwd) {
    this->pwd = pwd;
}

/*
 * Getters
 */
std::string Location::get_path() {
    return this->path;
}

bool Location::get_autoindex() const {
    return this->auto_index;
}

std::string Location::get_cgi_pass_val() {
    std::map<std::string, std::string>::iterator it = this->cgi_pass.begin();
    return it->second;
}

std::map<std::string, std::string> Location::get_cgi_pass() const {
    return this->cgi_pass;
}

std::string Location::get_pwd() const {
    return this->pwd;
}
