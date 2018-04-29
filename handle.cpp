
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "handle.hpp"

handle::handle(QObject *parent)
    : QObject(parent),
      statusMsg(""),
      uid(""),
      key(""),
      targetDir(""),
      sourceDir(""),
      encryptKey(""),
      signKey(""),
      // TODO: add more backends
      backend("pydrive+gdocs://developer.gserviceaccount.com/"),
      passphrase(""),
      signPassphrase(""),
      p_duplicity("/") {}

void handle::updateHandle(const QString &_targetDir, const QString &_sourceDir,
                          const QString &_encryptKey, const QString &_signKey,
                          const QString &_passphrase,
                          const QString &_signPassphrase) {
  // Already have a handle.json and reloaded
  if (!targetDir.empty()) return;

  targetDir = _targetDir.toStdString();
  // Convert /path/to/dir to path/to/dir
  if (targetDir[0] == '/') {
    targetDir = targetDir.substr(1);
  }
  sourceDir = _sourceDir.toStdString().substr(7);
  encryptKey = _encryptKey.toStdString();
  signKey = _signKey.toStdString();
  passphrase = _passphrase.toStdString();
  signPassphrase = _signPassphrase.toStdString();
  return;
}

void handle::updateHandle(const std::string &_targetDir,
                          const std::string &_sourceDir,
                          const std::string &_encryptKey,
                          const std::string &_signKey,
                          const std::string &_passphrase,
                          const std::string &_signPassphrase) {
  targetDir = _targetDir;
  sourceDir = _sourceDir;
  encryptKey = _encryptKey;
  signKey = _signKey;
  passphrase = _passphrase;
  signPassphrase = _signPassphrase;
  return;
}

// TODO: Asynchronous backup process for different folders feature
void handle::performBackup(const bool &isFull) const {
  std::string argv;

  if (isFull) {
    argv = "full";
  } else {
    argv = "incr";
  }

  boost::process::ipstream is;

  // Max 1,677,216 TB
  uint64_t backupSize = 0;

  boost::process::child c1(
      p_duplicity, argv, "--encrypt-key", encryptKey, "--sign-key", signKey,
      "--gpg-options", "--cipher-algo=AES256", "--allow-source-mismatch",
      sourceDir, backend + targetDir, boost::process::std_out > is,
      boost::process::env["PASSPHRASE"] = passphrase,
      boost::process::env["SIGN_PASSPHRASE"] = signPassphrase);
  c1.wait();

  std::string line;

  while (is && std::getline(is, line)) {
    if (line.empty()) {
      continue;
    }

    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string s;

    while (getline(ss, s, ' ')) {
      tokens.push_back(s);
    }

    if (tokens[0] == "TotalDestinationSizeChange") {
      std::stringstream sz(tokens[1]);
      sz >> backupSize;
      break;
    }
  }

  updateStatusText(getTime() + " - " + QString("new ") +
                   QString::fromStdString(isFull ? "full" : "incremental") +
                   QString(" backup size is ") +
                   QString::fromStdString(std::to_string(backupSize)) +
                   " bytes");

  // In case backup files are brocken
  boost::process::child c2(
      p_duplicity, "cleanup", "--force", backend + targetDir,
      boost::process::std_out > boost::process::null,
      boost::process::env["PASSPHRASE"] = passphrase,
      boost::process::env["SIGN_PASSPHRASE"] = signPassphrase);
  c2.wait();

  if (argv == "full") {
    boost::process::child c3(p_duplicity, "remove-all-but-n-full", "1",
                             "--force", backend + targetDir,
                             boost::process::std_out > boost::process::null);
    c3.wait();
    updateStatusText(getTime() + " - " +
                     QString("previous full backup is deleted"));
  }

  boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
  auto s_time = boost::posix_time::to_iso_extended_string(t);

  auto config_path = boost::filesystem::current_path();
  config_path += boost::filesystem::path("/handle.json");

  if (isFull) {
    auto root = writeToPT();
    root.put("lastFullDate", s_time);
    root.put("lastFullSize", backupSize);
    root.put("lastIncrDate", "none");
    root.put("totalIncrSize", 0);
    writeToJson(root, config_path);
  } else {
    auto root = readFromJson(config_path);
    root.put("lastIncrDate", s_time);
    auto last_incr_size = root.get<uint64_t>("totalIncrSize");
    root.put("totalIncrSize", backupSize + last_incr_size);
    writeToJson(root, config_path);
  }

  return;
}

void handle::getDup() {
  auto _p_duplicity = boost::process::search_path("duplicity");
  p_duplicity = _p_duplicity;
  return;
}

boost::property_tree::ptree handle::writeToPT() const {
  boost::property_tree::ptree _root;

  _root.put("targetDir", targetDir);
  _root.put("sourceDir", sourceDir);
  _root.put("encryptKey", encryptKey);
  _root.put("signKey", signKey);
  _root.put("backend", backend);
  _root.put("passphrase", passphrase);
  _root.put("signPassphrase", signPassphrase);
  _root.put("duplicity_path", p_duplicity.string());

  return _root;
}

void handle::doStatusChange() { emit updateStatusText(statusMsg); }

void handle::doKeysChange() { emit updateKeys(uid, key); }

boost::property_tree::ptree readFromJson(
    const boost::filesystem::path &config_path) {
  boost::property_tree::ptree root;
  boost::filesystem::ifstream file;
  file.open(config_path, boost::filesystem::ifstream::in);
  boost::property_tree::read_json(file, root);
  file.close();
  return root;
}

void writeToJson(const boost::property_tree::ptree &root,
                 const boost::filesystem::path &config_path) {
  boost::filesystem::ofstream file;
  if (!boost::filesystem::exists(config_path)) {
    file.open(config_path, boost::filesystem::ofstream::out);
  } else {
    file.open(config_path, boost::filesystem::ofstream::out |
                               boost::filesystem::ofstream::trunc);
  }
  boost::property_tree::write_json(file, root);
  file.close();
  return;
}

std::vector<std::pair<std::string, std::string>> get_keys() {
  std::vector<std::pair<std::string, std::string>> keys;

  auto p_gpg = boost::process::search_path("gpg");

  boost::process::ipstream is;
  boost::process::child c1(p_gpg, "--list-keys", boost::process::std_out > is);
  c1.wait();

  std::string line, uid, key;

  while (is && std::getline(is, line)) {
    if (line.empty()) {
      continue;
    }
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string s;
    while (getline(ss, s, ' ')) {
      if (!s.empty()) {
        tokens.push_back(s);
      }
    }

    if (tokens.size() == 1) {
      key = tokens[0];
    }
    if (tokens[0] == "uid") {
      uid = "uid";
      for_each(tokens.begin() + 1, tokens.end(),
               [&](auto &i) { uid += (" " + i); });
      keys.push_back(std::make_pair(uid, key));
    }
  }

  return keys;
}

QString getTime() {
  boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
  auto s = boost::posix_time::to_iso_extended_string(t).substr(11, 8);
  return QString::fromStdString(s);
}
