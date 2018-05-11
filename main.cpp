
//          Copyright blackgiulia.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "handle.hpp"

// Update keys to UI
void addKeys(QObject *const rootObject, const handle &pydriveHandle) {
  auto keys = pydriveHandle.get_keys();
  for (auto &i : keys) {
    QMetaObject::invokeMethod(
        rootObject, "appendKey",
        Q_ARG(QVariant, QString::fromStdString(i.first)),
        Q_ARG(QVariant, QString::fromStdString(i.second)));
  }
  return;
}

// Update restored info to UI
void addDir(QObject *const rootObject, const QString &dir) {
  QMetaObject::invokeMethod(rootObject, "appendDir", Q_ARG(QVariant, dir));
  return;
}

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

  auto root_object = engine.rootObjects().first();
  addKeys(root_object, pydriveHandle);

  auto config_path = boost::filesystem::current_path();
  config_path += boost::filesystem::path("/handle.json");

  if (boost::filesystem::exists(config_path)) {
    pydriveHandle.updateStatusText(getTime() + " - " +
                                   QString("handle.json is detected"));

    auto root = readFromJson(config_path);

    for (auto &i : root.get_child("pydrive")) {
      auto dir = QString::fromStdString(i.second.get<std::string>("sourceDir"));
      addDir(root_object, dir);
    }

    pydriveHandle.updateStatusText(
        getTime() + " - " +
        QString("use Select Directory box to perform a backup or restore "
                "operation"));
  }

  return app.exec();
}
