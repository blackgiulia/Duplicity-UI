
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
  if (engine.rootObjects().isEmpty())
    return -1;

  pydriveHandle.updateStatusText(getTime() + " - " + QString("Hello, Dear!"));

  auto keys = get_keys();
  for (auto &i : keys) {
    pydriveHandle.updateKeys(QString::fromStdString(i.first),
                             QString::fromStdString(i.second));
  }

  auto config_path = boost::filesystem::current_path();
  config_path += boost::filesystem::path("/handle.json");

  if (boost::filesystem::exists(config_path)) {
    pydriveHandle.updateStatusText(getTime() + " - " +
                                   QString("handle.json is detected"));

    auto root = readFromJson(config_path);

    for (auto &i : root.get_child("pydrive")) {
      pydriveHandle.updateDir(
          QString::fromStdString(i.second.get<std::string>("sourceDir")));
    }

    pydriveHandle.updateStatusText(
        getTime() + " - " +
        QString("an incremental backup can be performed on Select Directory"));
  }

  return app.exec();
}
