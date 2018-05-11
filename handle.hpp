
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <QByteArray>
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

  // Do restore
  Q_INVOKABLE void performRestore() const;

  // Do backup
  Q_INVOKABLE void performBackup(const bool &isFull) const;

  // Show status from handle.json
  Q_INVOKABLE void showLastStatus(const QString sourceDir_) const;

  // Set Duplicity path
  void getDup();

  // Read system GPG keys
  std::vector<std::pair<std::string, std::string>> get_keys() const;

  boost::property_tree::ptree writeToPT() const;

signals:
  // Update to UI status text
  void updateStatusText(const QString newStatus) const;

public slots:

private:
  QString statusMsg; // For status text update
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

// Get local time for status text update
QString getTime() noexcept;

// Encode string to base64 string
std::string encode64(const std::string &pass) noexcept;

// Decode base64 string to string
std::string decode64(const std::string &encoded) noexcept;

#endif // HANDLE_HPP
