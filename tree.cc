#include "tree.h"

void insert(TreeItem *item, FileTree *tree, QSettings *settings)
{
  int child_count = tree->folders.size();
  if (child_count == 0)
    return;
  for (int i = 0; i < child_count; i++) {
    auto f = tree->folders[i];
    QString id = QString::fromStdString(to_string(f.id));
    QString name = QString::fromStdString(f.name);
    QString dir = settings->value(id).toString();
    item->appendChild(new TreeItem(QStringList() << name << dir << id));
    insert(item->child(i), &f, settings);
  }
}

QString get_id(const QModelIndex &index)
{
  return index.siblingAtColumn(2).data(2).toString();
}

QString get_local_dir(const QModelIndex &index)
{
  return index.siblingAtColumn(1).data(2).toString();
}

QString get_local_dir(const TreeItem &item)
{
  return item.data(1).toString();
}

QString get_id(const TreeItem &item)
{
  return item.data(2).toString();
}

bool expand_tracked(ClickableTreeView *tree, QModelIndex &index)
{
  if (!index.isValid())
    return false;
  TreeModel *model = tree->model();
  bool expand = false;
  int n = model->childrenCount();
  for (int i = 0; i < n; i++) {
    QModelIndex child = model->index(i, 0, index);
    expand |= expand_tracked(tree, child);
  }
  if (!get_local_dir(index).isEmpty() || expand) {
    tree->expand(index.parent());
    return true;
  }
  return expand;
}

void expand_tracked(ClickableTreeView *tree)
{
  TreeModel *model = tree->model();
  int n = model->childrenCount();
  for (int i = 0; i < n; i++) {
    QModelIndex child = model->index(i, 0);
    if (expand_tracked(tree, child)) {
      tree->expand(child);
    }
  }
}

void fix_tree(Ui::MainWindow *ui)
{
  auto tree_view = ui->treeView;
  tree_view->resizeColumnToContents(0);
  expand_tracked(tree_view);
  // // FIXME: after debugging, hide ids from user
  // // tree_view->setColumnHidden(2, true);
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
  auto headers = QStringList() << "canvas folder"
                               << "local folder"
                               << "id";
  TreeModel *model = new TreeModel(headers);
  return model;
}
