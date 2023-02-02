#include "tree.h"

namespace fs = std::filesystem;

void resolve_all_folders(TreeItem *item, fs::path *local_base_dir,
                         fs::path *cwd, std::vector<Update> *list)
{
  std::string local_dir = get_local_dir(*item).toStdString();

  fs::path new_cwd;
  fs::path new_base = *local_base_dir;

  if (!local_base_dir->empty()) {
    new_cwd = *cwd / get_remote_dir(*item).toStdString();
  }

  // one-time thing. on any path downwards it is guaranteed to only have
  // one occurrence of a non-empty local path.
  if (!local_dir.empty()) {
    new_base = local_dir;
  }

  if (!new_base.empty()) {
    Update u(get_id(*item).toInt());
    u.local_dir = new_base / new_cwd;
    u.folder_id = get_id(*item).toInt();
    u.course_id = -1;
    list->push_back(u);
  }

  auto children = item->childrenItems();
  for (auto child : children)
    resolve_all_folders(child, &new_base, &new_cwd, list);
}

// assume here that `item` contains information about the module itself,
// and thus is not included in the resolution of the path.
//
// All updates from one call of this function belongs to the same course
std::vector<Update> resolve_all_folders(TreeItem *item)
{
  std::vector<Update> ul;
  auto children = item->childrenItems();
  for (auto child : children) {
    fs::path p = "";
    fs::path base = "";
    resolve_all_folders(child, &base, &p, &ul);
  }
  size_t n = ul.size(), id = get_id(*item).toInt();
  for (size_t i = 0; i < n; i++) {
    ul[i].course_id = id;
  }
  return ul;
}

std::vector<Update> gather_tracked(TreeModel *model)
{
  std::vector<Update> all;
  size_t n = model->childrenCount();
  for (size_t i = 0; i < n; i++) {
    std::vector<Update> u = resolve_all_folders(model->item(i));
    for (auto u : u)
      all.push_back(std::move(u));
  }
  return all;
}

void insert(TreeItem *item, FileTree *tree, QSettings *settings)
{
  size_t child_count = tree->folders.size();
  if (child_count == 0)
    return;
  for (size_t i = 0; i < child_count; i++) {
    auto f = tree->folders[i];
    QString id = QString::fromStdString(std::to_string(f.id));
    QString name = QString::fromStdString(f.name);
    QString dir = settings->value(id).toString();
    item->appendChild(new TreeItem(QStringList() << name << dir << id));
    insert(item->child(i), &f, settings);
  }
}

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

QString get_id(const TreeItem &item)
{
  return item.data(FOLDER_ID).toString();
}

QString get_local_dir(const TreeItem &item)
{
  return item.data(LOCAL_DIR).toString();
}

QString get_remote_dir(const TreeItem &item)
{
  return item.data(REMOTE_DIR).toString();
}

void on_all_parents(TreeItem *item, ItemOperator func)
{
  TreeItem *p = item->parent();
  while (p) {
    func(p);
    p = p->parent();
  }
}

void on_all_children(TreeItem *item, ItemOperator func)
{
  int n = item->childrenItems().size();
  for (int i = 0; i < n; i++) {
    func(item->child(i));
    on_all_children(item->child(i), func);
  }
}

TreeModel *newTreeModel()
{
  return new TreeModel({"canvas folder", "local folder"});
}
