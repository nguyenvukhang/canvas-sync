#include "clickable_tree_view.h"
#include <QAction>
#include <QMenu>

ClickableTreeView::ClickableTreeView(QWidget *parent) : QTreeView(parent)
{

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &ClickableTreeView::customContextMenuRequested, this,
          [&](const QPoint &pos) {
            QModelIndex index = indexAt(pos);
            if (!index.isValid())
              return;
            QMenu menu;
            auto addChildAction = menu.addAction("Clear");
            auto selectedAction = menu.exec(mapToGlobal(pos));
            switch (menu.actions().indexOf(selectedAction)) {
            case 0:
              clear(index);
              break;
            default:
              break;
            }
          });

  setEditTriggers(QAbstractItemView::NoEditTriggers);
}

ClickableTreeView::~ClickableTreeView()
{
}

TreeModel *ClickableTreeView::model() const
{
  return static_cast<TreeModel *>(QTreeView::model());
}

void ClickableTreeView::setModel(TreeModel *model)
{
  QTreeView::setModel(model);
}

void ClickableTreeView::clear(QModelIndex index)
{
  auto model = this->model();
  model->itemFromIndex(index)->setData(1, "");
  emit cleared(index);
}
