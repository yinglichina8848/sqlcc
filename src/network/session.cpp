#include "network/session.h"

namespace sqlcc {
namespace network {

// Session实现
Session::Session(int session_id)
    : session_id_(session_id),
      authenticated_(false),
      encryption_disabled_(false),
      authentication_disabled_(false),
      aes_encryptor_(nullptr) {}

void Session::SetEncryptionDisabled(bool disabled) {
    encryption_disabled_ = disabled;
}

bool Session::IsEncryptionDisabled() const {
    return encryption_disabled_;
}

void Session::SetAuthenticationDisabled(bool disabled) {
    authentication_disabled_ = disabled;
}

bool Session::IsAuthenticationDisabled() const {
    return authentication_disabled_;
}

void Session::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
}

std::shared_ptr<class AESEncryptor> Session::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool Session::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr && !encryption_disabled_;
}

void Session::SetAuthenticated(const std::string& username) {
    authenticated_ = true;
    username_ = username;
}

} // namespace network
} // namespace sqlcc
