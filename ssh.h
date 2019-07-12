#ifndef _SSH_H_
#define _SSH_H_

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <string>

class Ssh
{
public:
    enum class Status {
        disconnected,
        connected,
        unknownserv,
        verified,
        authenticated
    };

    enum class R {
        ok,
        status,
        connection,
        security,
        authentication,
        other
    };

    Ssh();
    ~Ssh();
    R execCommand(std::string command);
    R sendFileToFile(std::string src, std::string dest);
    R sendMemToFile(const void* mem, std::size_t size, std::string dest);
    R auth();
    R auth(std::string password);
    R verify();
    R accept();
    R addKnownHost();
    R connect(long timeOut = 10L);
    R setupSftp();
    R disconnect();
    //R echo();
    std::string getSshOut();
    std::string getSshErr();
    R setHost(std::string host);
    R setUser(std::string user);
    Status getStatus();
    std::string getHash();

    //setHost, setUser --> verify (server) ----------------------------------> auth -> auth(pass)
    //                       `-> (not known) -> getHash -> (ask user) -> accept -^

private:

    ssh_session ssh; //pointer!
    sftp_session sftp; //pointer!
    Status status;
    std::string sshOut;
    std::string sshErr;
    std::string host;
    std::string user;
    std::string hash;

};


#endif //_SSH_H_
