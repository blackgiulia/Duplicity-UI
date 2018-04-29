
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

class handle : public QObject {
  Q_OBJECT
 public:
  explicit handle(QObject *parent = nullptr);

  // Update from UI
  Q_INVOKABLE void updateHandle(const QString &_targetDir,
                                const QString &_sourceDir,
                                const QString &_encryptKey,
                                const QString &_signKey,
                                const QString &_passphrase,
                                const QString &_signPassphrase);

  // Update from handle.json
  void updateHandle(const std::string &_targetDir,
                    const std::string &_sourceDir,
                    const std::string &_encryptKey, const std::string &_signKey,
                    const std::string &_passphrase,
                    const std::string &_signPassphrase);

  // Do backup
  Q_INVOKABLE void performBackup(const bool &isFull) const;

  // Set Duplicity path
  void getDup();

  boost::property_tree::ptree writeToPT() const;

  //  Q_INVOKABLE void test() const {
  //    std::cout << targetDir << '\n'
  //              << sourceDir << '\n'
  //              << encryptKey << '\n'
  //              << signKey << '\n'
  //              << backend << '\n'
  //              << passphrase << '\n'
  //              << signPassphrase << '\n'
  //              << p_duplicity << std::endl;
  //  }

 signals:
  // Update to UI status text
  void updateStatusText(QString newStatus) const;

  // Update system GPG keys to UI, only display key uid information
  void updateKeys(QString uid, QString key) const;

 public slots:  // Let qml listen
  void doStatusChange();

  void doKeysChange();

 private:
  QString statusMsg;  // For status text update
  QString uid;
  QString key;
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
boost::property_tree::ptree readFromJson(
    const boost::filesystem::path &config_path);

// Write to handle.json
void writeToJson(const boost::property_tree::ptree &root,
                 const boost::filesystem::path &config_path);

// Read system GPG keys
std::vector<std::pair<std::string, std::string>> get_keys();

// Get local time for status text update
QString getTime();

#endif  // HANDLE_HPP
