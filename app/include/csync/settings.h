#ifndef CANVAS_SYNC_SETTINGS_H
#define CANVAS_SYNC_SETTINGS_H

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

class Settings : private QSettings
{
  Q_OBJECT

public:
  Settings(const QString &settings_file = "canvas-sync-settings.ini")
      : QSettings(dir + '/' + settings_file, QSettings::IniFormat){};

  enum Type { TRACKED, LOCAL_DIR };
  static const QString dir; // defined elsewhere

private:
  QString type(const Type t)
  {
    if (t == TRACKED) return "tracked";
    if (t == LOCAL_DIR) return "local-dir";
    return "";
  }

public:
  QString get(const QString &g, const QString &k);
  QString get(const QString &k) const { return value(k).toString(); }
  QString get(const QString &g, const Type &t) { return get(g, type(t)); }
  void set(const QString &k, const QString &v);
  void set(const QString &g, const QString &k, const QString &v);
  void set(const QString &g, const QString &v, const Type &t)
  {
    set(g, type(t), v);
  };
  void remove(const QString &k);
  void remove(const QString &g, const QString &k);

  // derivative functions
  bool is_tracked(const QString &folder_id);
  bool has_local_dir(const QString &folder_id);

  QString path() const { return QSettings::fileName(); }
};

#endif
