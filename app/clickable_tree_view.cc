#include "clickable_tree_view.h"

ClickableTreeView::ClickableTreeView(QWidget *parent) : QTreeView(parent)
{
  this->hideColumn(TreeIndex::FOLDER_ID);
}

TreeModel *ClickableTreeView::model() const
{
  return static_cast<TreeModel *>(QTreeView::model());
};

void ClickableTreeView::setModel(TreeModel *model)
{
  QTreeView::setModel(model);
  hideColumn(TreeIndex::FOLDER_ID);
  expand_tracked();
};

bool expand_tracked_inner(ClickableTreeView *tv, const QModelIndex &index)
{
  if (!index.isValid()) return false;
  TreeModel *model = tv->model();
  bool expand = false;
  int n = model->childrenCount();
  for (int i = 0; i < n; i++) {
    expand |= expand_tracked_inner(tv, model->index(i, 0, index));
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
