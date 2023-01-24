#ifndef CLICKABLE_TREE_VIEW_H
#define CLICKABLE_TREE_VIEW_H

#include "tree_model.h"
#include <QTreeView>

class ClickableTreeView : public QTreeView
{
  Q_OBJECT

public:
  ClickableTreeView(QWidget *parent = nullptr);
  ~ClickableTreeView();
  void clear(QModelIndex index);
  void setModel(TreeModel *model);
  TreeModel *model() const;

signals:
  void cleared(const QModelIndex &index);
};

#endif
