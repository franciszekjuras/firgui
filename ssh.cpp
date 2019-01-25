#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <iostream>
#include <fstream>
#include <string>
#include "ssh.h"

#define O_WRONLY 1
#define O_CREAT 64
#define O_TRUNC 512

#define BUFSIZE 4096
#define SFTP_CHUNK 65536

Ssh::Ssh(){
    status = Status::disconnected;
    ssh = NULL;
    sftp = NULL;
}

Ssh::~Ssh(){
    if(sftp != NULL)
        sftp_free(sftp);
    if(status != Status::disconnected){
        ssh_disconnect(ssh);
        ssh_free(ssh);
    }
}

/*----------------------Operations-------------------------*/

Ssh::R Ssh::execCommand(std::string command){
    sshOut.clear();
    sshErr.clear();
    if(status != Status::authenticated)
        return R::status;

    ssh_channel channel;
    int nbytes;
    int rc;
    char buffer[BUFSIZE];


    channel = ssh_channel_new(ssh);
    if (channel == NULL) {
        std::cerr << "Couldn't open channel.\n"; return R::connection;
    }

    rc = ssh_channel_open_session(channel);
    if (rc < 0) {
        std::cerr << "ssh_channel_open_session returned error\n";
        ssh_channel_close(channel); ssh_channel_free(channel); return R::connection;
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc < 0) {
        std::cerr << "ssh_channel_request_exec returned error\n";
        ssh_channel_close(channel); ssh_channel_free(channel); return R::connection;
    }

    nbytes = ssh_channel_read(channel, buffer, BUFSIZE, 0);
    // ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 0, timeout);
    while (nbytes > 0) {
        sshOut.append(buffer, nbytes);
        nbytes = ssh_channel_read(channel, buffer, BUFSIZE, 0);
    }

    nbytes = ssh_channel_read(channel, buffer, BUFSIZE, 1);
    // ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 1, timeout);
    while (nbytes > 0) {
        sshErr.append(buffer, nbytes);
        nbytes = ssh_channel_read(channel, buffer, BUFSIZE, 1);
    }
    rc = ssh_channel_is_eof(channel);

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    if(!rc) return R::connection;

    return R::ok;
}

Ssh::R Ssh::sendFileToFile(std::string src, std::string dest){
    std::ifstream srcFile (src, std::ifstream::binary);
    if(!srcFile){std::cerr << "Couldn't open local file for reading: "<< src << std::endl;return R::other;}

    // O_WRONLY = 1  O_CREAT = 64  O_TRUNC = 512
    sftp_file destRFile = sftp_open(sftp, dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(destRFile==NULL){std::cerr << "Couldn't open remote file for writing: " << dest << std::endl;return R::other;}

    /*-------------------------*/
    char buffer[SFTP_CHUNK];
    int nwritten;
    while (srcFile){
        srcFile.read(buffer, SFTP_CHUNK);
        nwritten = sftp_write(destRFile, buffer, srcFile.gcount());
        if (nwritten != srcFile.gcount()) {
            std::cerr << "Error writing to remote file: " << dest << ssh_get_error(ssh) << std::endl;
            sftp_close(destRFile); return R::connection;
        }
    }
    if (srcFile.bad()){
        std::cerr << "Error reading local file: " << src << std::endl;
        sftp_close(destRFile); return R::other;
    }
    sftp_close(destRFile);
    return R::ok;
}

Ssh::R Ssh::sendMemToFile(const void* mem, std::size_t size, std::string dest){

    // O_WRONLY = 1  O_CREAT = 64  O_TRUNC = 512
    sftp_file destRFile = sftp_open(sftp, dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(destRFile==NULL){std::cerr << "Couldn't open remote file for writing: " << dest << std::endl;return R::other;}

    /*-------------------------*/
    //char buffer[SFTP_CHUNK];
    const void* buffer = mem; std::size_t sizeLeft = size; std::size_t toWrite;
    int nwritten;
    while (sizeLeft != 0){
        //srcFile.read(buffer, SFTP_CHUNK);
        toWrite = (sizeLeft < SFTP_CHUNK) ? sizeLeft : SFTP_CHUNK;
        nwritten = sftp_write(destRFile, buffer, toWrite);
        if (nwritten != toWrite) {
            std::cerr << "Error writing to remote file: " << dest << ssh_get_error(ssh) << std::endl;
            sftp_close(destRFile); return R::connection;
        }
        sizeLeft -= toWrite;
    }
    sftp_close(destRFile);
    return R::ok;
}

/*----------------------Connection------------------------*/


Ssh::R Ssh::connect(){
    // connect reconnects if status is not 'disconnected'
    if(status != Status::disconnected)
        return R::status;
    if(user.empty() || host.empty())
        return R::other;

    ssh=ssh_new();
    ssh_options_set(ssh, SSH_OPTIONS_USER, user.c_str());
    ssh_options_set(ssh, SSH_OPTIONS_HOST, host.c_str());
    //ssh_options_set(ssh, SSH_OPTIONS_LOG_VERBOSITY, verbosity);
    if(ssh_connect(ssh) != 0){
        std::cerr << "Connection failed: " << ssh_get_error(ssh) << "\n";
        ssh_disconnect(ssh); ssh_free(ssh); ssh = NULL; return R::connection;
    }
    status = Status::connected;
    return R::ok;
}

Ssh::R Ssh::verify(){
    if(status != Status::connected)
        return R::status;

    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    size_t hlen;
    ssh_key srv_pubkey;

    ssh_get_server_publickey(ssh, &srv_pubkey);
    ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);

    char* hexa = ssh_get_hexa(hash, hlen);
    ssh_clean_pubkey_hash(&hash);

    this->hash.clear();
    this->hash.append(hexa);
    ssh_string_free_char(hexa);

    state = ssh_session_is_known_server(ssh);
    switch(state){
        case SSH_KNOWN_HOSTS_OK:
            status = Status::verified;
            ssh_userauth_none(ssh, NULL);
            break; /* ok */
        case SSH_KNOWN_HOSTS_CHANGED:
            return R::security;
        case SSH_KNOWN_HOSTS_OTHER:
            return R::security;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
        case SSH_SERVER_NOT_KNOWN:
            status = Status::unknownserv;
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            return R::other;
    }
    return R::ok;
}

Ssh::R Ssh::addKnownHost(){
    if(status != Status::unknownserv)
        return R::status;
    if(ssh_write_knownhost(ssh)<0)
        return R::other;
    return R::ok;
}

Ssh::R Ssh::accept(){
    if(status != Status::unknownserv)
        return R::status;
    status = Status::verified;
    ssh_userauth_none(ssh, NULL);
    return R::ok;
}

Ssh::R Ssh::auth(){
    if(status != Status::verified)
        return R::status;
    int method = ssh_userauth_list(ssh, NULL);
    if(method & SSH_AUTH_METHOD_PUBLICKEY){
        int rc = ssh_userauth_publickey_auto(ssh, NULL, NULL);
        if (rc == SSH_AUTH_ERROR) {
            std::cerr << "Authentication failed: " << ssh_get_error(ssh) << "\n";
            return R::authentication;
        }
        else if (rc == SSH_AUTH_SUCCESS){
            status = Status::authenticated;
            return R::ok;
        }
    }
    return R::other;
}

Ssh::R Ssh::auth(std::string password){
    if(status != Status::verified)
        return R::status;
    int method = ssh_userauth_list(ssh, NULL);
    if(method & SSH_AUTH_METHOD_PASSWORD){
        int rc = ssh_userauth_password(ssh, NULL, password.c_str());
        if (rc == SSH_AUTH_ERROR) {
            std::cerr << "Authentication failed: " << ssh_get_error(ssh) << "\n";
            return R::authentication;
        }
        else if (rc == SSH_AUTH_SUCCESS){
            status = Status::authenticated;
            return R::ok;
        }
    }
    return R::other;
}

Ssh::R Ssh::setupSftp(){
    if(status != Status::authenticated)
        return R::status;

    sftp = sftp_new(ssh);
    if(sftp_init(sftp) != SSH_OK){
        std::cerr << "SFTP Connection not established\n";
        sftp_free(sftp); sftp = NULL;
        return R::other;
    }
    return R::ok;
}

Ssh::R Ssh::disconnect(){
    sftp_free(sftp); sftp = NULL;
    if(status != Status::disconnected)
        ssh_disconnect(ssh);
    ssh_free(ssh); ssh = NULL;
    status = Status::disconnected;
    hash.clear();
    return R::ok;
}

/*-----------------Setters and getters-------------------*/

std::string Ssh::getSshOut(){return sshOut;}

std::string Ssh::getSshErr(){return sshErr;}

Ssh::R Ssh::setHost(std::string host){
    if(status != Status::disconnected)
        return R::security;
    this->host = host;
    return R::ok;
}

Ssh::R Ssh::setUser(std::string user){
    if(status != Status::disconnected)
        return R::security;
    this->user = user;
    return R::ok;
}

Ssh::Status Ssh::getStatus(){return status;}

std::string Ssh::getHash(){return hash;}
