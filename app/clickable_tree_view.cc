#include "clickable_tree_view.h"

ClickableTreeView::ClickableTreeView(QWidget *parent) : QTreeView(parent)
{
  setContextMenuPolicy(Qt::ActionsContextMenu);
  actions.clear = menu.addAction("Clear");
  actions.track_folder = menu.addAction("Track Folder");
  addAction(actions.track_folder);
  addAction(actions.clear);

  actions.clear->setDisabled(true);
  connect(&this->menu, &QMenu::triggered, this, [=]() {
    qDebug() << "TRIGGER CTX";
    // this->bindableObjectName
  });
  connect(this, &ClickableTreeView::activated, this, [=]() {
    qDebug() << "ACTIVATED";
    // this->bindableObjectName
  });
  this->hideColumn(TreeIndex::FOLDER_ID);
}

void ClickableTreeView::context_menu(const QPoint &pos)
{
  const QModelIndex index = indexAt(pos);
  if (!index.isValid() || !index.parent().isValid()) return;
  this->menu.clear();

  // create menu
  if (index.column() == TreeIndex::REMOTE_DIR) {
    this->menu.addAction("Track Folder");
  }
  if (!TreeIndex(index).local_dir().isEmpty()) {
    this->menu.addAction("Clear");
  }
  auto e = this->menu.exec(mapToGlobal(pos));
  if (e == nullptr) return;

  // get selected item
  QString target = e->text();

  if (target == "Track Folder") {
    emit track_folder(index);
  }
  if (target == "Clear") {
    emit cleared(index);
  }
}

TreeModel *ClickableTreeView::model() const
{
  return static_cast<TreeModel *>(QTreeView::model());
};

void ClickableTreeView::setModel(TreeModel *model)
{
  QTreeView::setModel(model);
  this->hideColumn(TreeIndex::FOLDER_ID);
  this->expand_tracked();
};

bool expand_tracked_inner(ClickableTreeView *tv, const QModelIndex &index)
{
  if (!index.isValid()) return false;
  TreeModel *model = tv->model();
  bool expand = false;
  int n = model->childrenCount();
  for (int i = 0; i < n; i++) {
    QModelIndex child = model->index(i, 0, index);
    expand |= expand_tracked_inner(tv, child);
  }
  if (!TreeIndex(index).local_dir().isEmpty() || expand) {
    tv->expand(index.parent());
    return true;
  }
  return expand;
}

void ClickableTreeView::expand_tracked()
{
  TreeModel *model = this->model();
  int n = model->childrenCount();
  for (int i = 0; i < n; i++) {
    QModelIndex child = model->index(i, 0);
    if (expand_tracked_inner(this, child)) {
      this->expand(child);
    }
  }
}

void ClickableTreeView::prettify()
{
  this->resizeColumnToContents(TreeIndex::REMOTE_DIR);
  if (columnWidth(TreeIndex::REMOTE_DIR) > this->width() * 0.6) {
    setColumnWidth(TreeIndex::REMOTE_DIR, this->width() * 0.6);
  }
};
