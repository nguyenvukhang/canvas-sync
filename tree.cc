#include "tree.h"

QString get_id(const QModelIndex &index)
{
  return index.siblingAtColumn(FOLDER_ID).data().toString();
}

QString get_local_dir(const QModelIndex &index)
{
  return index.siblingAtColumn(LOCAL_DIR).data().toString();
}

QString get_remote_dir(const QModelIndex &index)
{
  return index.siblingAtColumn(REMOTE_DIR).data().toString();
}

QString get_course(const QModelIndex &index)
{
  QModelIndex a = index;
  while (a.parent().isValid()) {
    a = a.parent();
  }
  return a.siblingAtColumn(REMOTE_DIR).data().toString();
}

QString get_ancestry(const QModelIndex &index, const char *delimiter)
{
  QString path;
  QModelIndex a = index;
  path = get_remote_dir(index);
  while (a.parent().isValid()) {
    a = a.parent();
    QString d = get_remote_dir(a);
    if (d.size() > 10) {
      d.truncate(10);
      d.push_back("...");
    }
    path = d + delimiter + path;
  }
  return path;
}

TreeModel *newTreeModel()
{
  return new TreeModel({"canvas folder", "local folder"});
}
