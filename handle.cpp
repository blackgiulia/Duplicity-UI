
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "handle.hpp"

handle::handle(QObject *parent)
    : QObject(parent), statusMsg(""), uid(""), key(""), dir(""), targetDir(""),
      sourceDir(""), encryptKey(""), signKey(""),
      // TODO: add more backends
      backend("pydrive+gdocs://developer.gserviceaccount.com/"), passphrase(""),
      signPassphrase(""), p_duplicity("/") {}

void handle::updateHandleFromQML(const QString &targetDir_,
                                 const QString &sourceDir_,
                                 const QString &encryptKey_,
                                 const QString &signKey_,
                                 const QString &passphrase_,
                                 const QString &signPassphrase_,
                                 const QString &dir_) {
  if (dir_ == QString("Select directory:")) { // Update from QML
    targetDir = targetDir_.toStdString();
    // Convert /path/to/dir to path/to/dir
    if (targetDir[0] == '/') {
      targetDir = targetDir.substr(1);
    }
    sourceDir = sourceDir_.toStdString().substr(7);
    encryptKey = encryptKey_.toStdString();
    signKey = signKey_.toStdString();
    passphrase = passphrase_.toStdString();
    signPassphrase = signPassphrase_.toStdString();
  } else { // Update from handle.json
    std::string d = dir_.toStdString();

    auto config_path = boost::filesystem::current_path();
    config_path += boost::filesystem::path("/handle.json");

    if (boost::filesystem::exists(config_path)) {
      auto root = readFromJson(config_path);

      for (auto &i : root.get_child("pydrive")) {
        if (i.second.get<std::string>("sourceDir") == d) {
          targetDir = i.second.get<std::string>("targetDir");
          sourceDir = i.second.get<std::string>("sourceDir");
          encryptKey = i.second.get<std::string>("encryptKey");
          signKey = i.second.get<std::string>("signKey");
          passphrase = decode64(i.second.get<std::string>("passphrase"));
          signPassphrase =
              decode64(i.second.get<std::string>("signPassphrase"));
          break;
        }
      }
    }
  }

  return;
}

void handle::performRestore() const {
  // Do restore
  try {
    boost::process::child c(
        p_duplicity, "restore", "--encrypt-key", encryptKey, "--sign-key",
        signKey, backend + targetDir, sourceDir,
        boost::process::std_out > boost::process::null,
        boost::process::env["PASSPHRASE"] = passphrase,
        boost::process::env["SIGN_PASSPHRASE"] = signPassphrase);
    c.wait();
  } catch (std::system_error &err) {
    updateStatusText(getTime() + " - restore failed. Make sure duplicity, "
                                 "PyDrive and GPG are installed");
  }

  updateStatusText(getTime() + " - " + QString::fromStdString(targetDir) +
                   " has been restored to " +
                   QString::fromStdString(sourceDir));

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

  // Do backup
  try {
    boost::process::child c1(
        p_duplicity, argv, "--encrypt-key", encryptKey, "--sign-key", signKey,
        "--gpg-options", "--cipher-algo=AES256", "--allow-source-mismatch",
        sourceDir, backend + targetDir, boost::process::std_out > is,
        boost::process::env["PASSPHRASE"] = passphrase,
        boost::process::env["SIGN_PASSPHRASE"] = signPassphrase);
    c1.wait();
  } catch (std::system_error &err) {
    updateStatusText(
        getTime() +
        " - backup failed. Make sure duplicity, PyDrive and GPG are installed");
  }

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

  // Remove previous backup
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

  if (boost::filesystem::exists(config_path)) {
    if (!isFull) { // Update incremental backup status to handle.json
      auto root = readFromJson(config_path);
      for (auto &i : root.get_child("pydrive")) {
        if (i.second.get<std::string>("targetDir") == targetDir) {
          i.second.put("lastIncrDate", s_time);
          auto last_incr_size = i.second.get<uint64_t>("totalIncrSize");
          i.second.put("totalIncrSize", backupSize + last_incr_size);
          break;
        }
      }
      writeToJson(root, config_path);
      return;
    } else { // Update full backup status to handle.json
      auto root = readFromJson(config_path);
      for (auto &i : root.get_child("pydrive")) {
        // Modify corresponding status
        if (i.second.get<std::string>("targetDir") == targetDir) {
          i.second.put("lastFullDate", s_time);
          i.second.put("lastFullSize", backupSize);
          i.second.put("lastIncrDate", "none");
          i.second.put("totalIncrSize", 0);
          writeToJson(root, config_path);
          return;
        }
      }
      // Add a new status
      auto r = writeToPT();
      r.put("lastFullDate", s_time);
      r.put("lastFullSize", backupSize);
      r.put("lastIncrDate", "none");
      r.put("totalIncrSize", 0);
      r.put("passphrase", encode64(r.get<std::string>("passphrase")));
      r.put("signPassphrase", encode64(r.get<std::string>("signPassphrase")));
      root.get_child("pydrive").push_back(make_pair("", r));
      writeToJson(root, config_path);
      return;
    }
  } else { // Create new handle.json
    auto r = writeToPT();
    r.put("lastFullDate", s_time);
    r.put("lastFullSize", backupSize);
    r.put("lastIncrDate", "none");
    r.put("totalIncrSize", 0);
    r.put("passphrase", encode64(r.get<std::string>("passphrase")));
    r.put("signPassphrase", encode64(r.get<std::string>("signPassphrase")));
    boost::property_tree::ptree root;
    root.put("pydrive", "");
    root.get_child("pydrive").push_back(make_pair("", r));
    writeToJson(root, config_path);
    return;
  }

  return;
}

void handle::showLastStatus(const QString sourceDir_) const {
  std::string d = sourceDir_.toStdString();

  auto config_path = boost::filesystem::current_path();
  config_path += boost::filesystem::path("/handle.json");

  if (boost::filesystem::exists(config_path)) {
    auto root = readFromJson(config_path);

    for (auto &i : root.get_child("pydrive")) {
      if (i.second.get<std::string>("sourceDir") == d) {
        updateStatusText(
            QString("    last full backup date is ") +
            QString::fromStdString(i.second.get<std::string>("lastFullDate")));

        updateStatusText(
            QString("    last full backup size is ") +
            QString::fromStdString(i.second.get<std::string>("lastFullSize")) +
            QString(" bytes"));

        updateStatusText(
            QString("    last incremental backup date is ") +
            QString::fromStdString(i.second.get<std::string>("lastIncrDate")));

        updateStatusText(
            QString("    total incremental backup size is ") +
            QString::fromStdString(i.second.get<std::string>("totalIncrSize")) +
            QString(" bytes"));

        updateStatusText(sourceDir_ + " last status:");
        break;
      }
    }
  }

  return;
}

void handle::getDup() {
  p_duplicity = boost::process::search_path("duplicity");
  return;
}

std::vector<std::pair<std::string, std::string>> handle::get_keys() const {
  std::vector<std::pair<std::string, std::string>> keys;

  auto p_gpg = boost::process::search_path("gpg");

  boost::process::ipstream is;
  try {
    boost::process::child c1(p_gpg, "--list-keys",
                             boost::process::std_out > is);
    c1.wait();
  } catch (std::system_error &err) {
    updateStatusText(getTime() + " - GPG keys are not detected");
  }

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
    // Get GPG keys, show only uid to UI
    if (tokens.size() == 1) {
      key = tokens[0];
    }
    if (tokens[0] == "uid") {
      uid = "uid";
      for_each(tokens.begin() + 1, tokens.end(),
               [&](auto &i) { uid += (" " + i); });
      keys.emplace_back(uid, key);
    }
  }

  return keys;
}

boost::property_tree::ptree handle::writeToPT() const {
  boost::property_tree::ptree root;

  root.put("targetDir", targetDir);
  root.put("sourceDir", sourceDir);
  root.put("encryptKey", encryptKey);
  root.put("signKey", signKey);
  root.put("backend", backend);
  root.put("passphrase", passphrase);
  root.put("signPassphrase", signPassphrase);

  return root;
}

void handle::doStatusChange() { emit updateStatusText(statusMsg); }

void handle::doKeysChange() { emit updateKeys(uid, key); }

void handle::doDirChange() { emit updateDir(dir); }

boost::property_tree::ptree
readFromJson(const boost::filesystem::path &config_path) {
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

QString getTime() noexcept {
  boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
  auto s = boost::posix_time::to_iso_extended_string(t).substr(11, 8);
  return QString::fromStdString(s);
}

std::string encode64(const std::string &pass) noexcept {
  std::string res;
  CryptoPP::Base64Encoder encoder;
  CryptoPP::byte *decoded = (CryptoPP::byte *)pass.c_str();
  encoder.Put(decoded, pass.size());
  encoder.MessageEnd();
  CryptoPP::word64 size = encoder.MaxRetrievable();
  if (size) {
    res.resize(size);
    encoder.Get((CryptoPP::byte *)&res[0], res.size());
  }
  return res;
}

std::string decode64(const std::string &encoded) noexcept {
  std::string decoded;
  CryptoPP::Base64Decoder decoder;
  decoder.Put((CryptoPP::byte *)encoded.data(), encoded.size());
  decoder.MessageEnd();

  CryptoPP::word64 size = decoder.MaxRetrievable();
  if (size && size <= SIZE_MAX) {
    decoded.resize(size);
    decoder.Get((CryptoPP::byte *)&decoded[0], decoded.size());
  }
  return decoded;
}
