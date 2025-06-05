#pragma once

#include <string>

namespace hyper_block {

extern int send_email(std::string const &from_email, std::string const &to_email, std::string const &subject,
                      std::string const &body, std::string const &smtp_user, std::string const &smtp_pwd,
                      unsigned short starttls_port, std::string const &smtp_server);

extern std::string sha256(const std::string &str);

extern std::string uuid();

extern void run_validation_service(unsigned short port = 80);

}   // namespace hyper_block
