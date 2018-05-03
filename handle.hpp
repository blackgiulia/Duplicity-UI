
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <QObject>
#include <QString>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cryptopp/base64.h>

class handle : public QObject {
  Q_OBJECT
public:
  explicit handle(QObject *parent = nullptr);

  // Update from UI
  Q_INVOKABLE void
  updateHandleFromQML(const QString &targetDir_, const QString &sourceDir_,
                      const QString &encryptKey_, const QString &signKey_,
                      const QString &passphrase_,
                      const QString &signPassphrase_, const QString &dir_);

  // Do backup
  Q_INVOKABLE void performBackup(const bool &isFull) const;

  // Show status from handle.json
  Q_INVOKABLE void showLastStatus(QString sourceDir_) const;

  // Set Duplicity path
  void getDup();

  boost::property_tree::ptree writeToPT() const;

  //  void test() {
  //    std::cout << targetDir << '\n'
  //              << sourceDir << '\n'
  //              << encryptKey << '\n'
  //              << signKey << '\n'
  //              << passphrase << '\n'
  //              << signPassphrase << std::endl;
  //  }

signals:
  // Update to UI status text
  void updateStatusText(const QString newStatus) const;

  // Update system GPG keys to UI, only display key uid information
  void updateKeys(const QString uid, const QString key) const;

  // Update directories in handle.json to UI
  void updateDir(const QString dir) const;

public slots: // Let qml listen
  void doStatusChange();

  void doKeysChange();

  void doDirChange();

private:
  QString statusMsg; // For status text update
  QString uid;
  QString key;
  QString dir;
  std::string targetDir;
  std::string sourceDir;
  std::string encryptKey;
  std::string signKey;
  std::string backend;
  std::string passphrase;
  std::string signPassphrase;
  boost::filesystem::path p_duplicity;
};

// Read from handle.json
boost::property_tree::ptree
readFromJson(const boost::filesystem::path &config_path);

// Write to handle.json
void writeToJson(const boost::property_tree::ptree &root,
                 const boost::filesystem::path &config_path);

// Read system GPG keys
std::vector<std::pair<std::string, std::string>> get_keys();

// Get local time for status text update
QString getTime();

// Encode string to base64 string
std::string encode64(const std::string &pass);

// Decode base64 string to string
std::string decode64(const std::string &pass);

#endif // HANDLE_HPP
