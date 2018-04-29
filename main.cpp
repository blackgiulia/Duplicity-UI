
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QString>
#include <QtDebug>

#include "handle.hpp"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  handle pydriveHandle;

  pydriveHandle.getDup();

  engine.rootContext()->setContextProperty("handler", &pydriveHandle);

  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty()) return -1;

  pydriveHandle.updateStatusText(getTime() + " - " + QString("Hello, Dear!"));

  auto keys = get_keys();
  for (auto &i : keys) {
    pydriveHandle.updateKeys(QString::fromStdString(i.first),
                             QString::fromStdString(i.second));
  }

  boost::property_tree::ptree root;
  auto config_path = boost::filesystem::current_path();
  config_path += boost::filesystem::path("/handle.json");

  if (boost::filesystem::exists(config_path)) {
    pydriveHandle.updateStatusText(getTime() + " - " +
                                   QString("handle.json is detected"));

    auto root = readFromJson(config_path);

    pydriveHandle.updateHandle(
        root.get<std::string>("targetDir"), root.get<std::string>("sourceDir"),
        root.get<std::string>("encryptKey"), root.get<std::string>("signKey"),
        root.get<std::string>("passphrase"),
        root.get<std::string>("signPassphrase"));
    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("reloading handle.json successfully"));

    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("last full backup date is ") +
        QString::fromStdString(root.get<std::string>("lastFullDate")));

    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("last full backup size is ") +
        QString::fromStdString(root.get<std::string>("lastFullSize")) +
        QString(" bytes"));

    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("last incremental backup date is ") +
        QString::fromStdString(root.get<std::string>("lastIncrDate")));

    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("total incremental backup size is ") +
        QString::fromStdString(root.get<std::string>("totalIncrSize")) +
        QString(" bytes"));

    pydriveHandle.updateStatusText(
        getTime() + " - " + QString("an incremental backup can be performed"));
  }

  return app.exec();
}
