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
  void sync() { QSettings::sync(); }
  QString get(const QString &group, const QString &key);
  QString get(const QString &key) const { return value(key).toString(); }
  QString get(const QString &key, const Type &t) { return get(key, type(t)); }
  void set(const QString &key, const QString &val);
  void set(const QString &group, const QString &key, const QString &val);
  void set(const QString &key, const Type &t, const QString &val)
  {
    set(key, type(t), val);
  };
  void set(const QString &key, const Type &t, const bool &val)
  {
    set(key, type(t), val ? "true" : "false");
  };
  void remove(const QString &key);
  void remove(const QString &group, const QString &key);
  void remove(const QString &key, const Type &t) { remove(key, type(t)); };

  // derivative functions
  QString local_dir(const QString &folder_id);
  bool has_local_dir(const QString &folder_id);
  bool is_tracked(const QString &folder_id);

  QString path() const { return QSettings::fileName(); }
};

#endif
