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

bool expand_tracked(ClickableTreeView *tree, QModelIndex &index)
{
  TreeModel *model = tree->model();
  bool expand = false;
  int i = 0;
  QModelIndex child = model->index(i++, 0, index);
  while (child.isValid()) {
    expand |= expand_tracked(tree, child);
    child = model->index(i++, 0, index);
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
  int i = 0;
  QModelIndex child = model->index(i++, 0);
  while (child.isValid()) {
    if (expand_tracked(tree, child)) {
      tree->expand(child);
    }
    child = model->index(i++, 0);
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

void clear_children(TreeItem *item, int data_index)
{
  item->setData(data_index, "");
  int child_count = item->childrenItems().size();
  for (int i = 0; i < child_count; i++) {
    clear_children(item->child(i), data_index);
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
