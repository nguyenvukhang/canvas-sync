#include "settings.h"

const QString Settings::dir =
    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

QString Settings::get(const QString &g, const QString &k)
{
  beginGroup(g);
  QString result = value(k).toString();
  endGroup();
  return result;
}

void Settings::set(const QString &k, const QString &v)
{
  if (k.isEmpty()) {
    qDebug() << "Trying to set an empty setting k" << v;
    return;
  }
  setValue(k, v);
  sync();
}

void Settings::set(const QString &g, const QString &k, const QString &v)
{
  beginGroup(g);
  set(k, v);
  endGroup();
}

void Settings::remove(const QString &k)
{
  QSettings::remove(k);
  sync();
}

void Settings::remove(const QString &g, const QString &k)
{
  beginGroup(g);
  Settings::remove(k);
  endGroup();
}

QString Settings::local_dir(const QString &folder_id)
{
  return get(folder_id, Type::LOCAL_DIR);
}

bool Settings::has_local_dir(const QString &folder_id)
{
  return !local_dir(folder_id).isEmpty();
}

bool Settings::is_tracked(const QString &folder_id)
{
  return get(folder_id, Type::TRACKED) == "true";
}
