#include "../../include/deleteResponder.hpp"

/*
 * Constructors
 */
deleteResponder::deleteResponder():httpResponder() {}

deleteResponder::~deleteResponder() {}

/*
 * Implementation parent virtual function
 */
void deleteResponder::answer(HTTPParser pars, Location *location, WebServer *server, std::string server_name, std::string index) {
    std::string to_find = pars.get_content_by_key("PATH");
    std::string file_path = location->autoindex("uploads/", to_find);
    int res;

    if (file_path.empty()) {
        this->generate_error("404", location, server);
        return;
    }
    if ((res = search_delete(file_path)))
        this->generate_error("500", location, server);
}

/*
 * Response generation functions
 */
int deleteResponder::search_delete(std::string to_delete) {
    std::string ret;

    char *matrix[] = {(char *)"/bin/rm", (char*)"-f", (char *)to_delete.c_str(), NULL};

    int pid = fork();

    if (pid == -1)
        return -1;

    if(pid == 0) {
        execve(matrix[0],matrix , NULL);
        std::cerr << "evecve failed\n";
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            ret.append("HTTP/1.1 200 OK\r\n");
            setFullAnswer(ret);
            return 0;
        } else {
            std::cerr << "delete execve failed: " <<  WEXITSTATUS(status) << '\n';
            return WEXITSTATUS(status);
        }
    }
    return 1;
}
