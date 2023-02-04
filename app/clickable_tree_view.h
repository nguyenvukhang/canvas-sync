#ifndef CLICKABLE_TREE_VIEW_H
#define CLICKABLE_TREE_VIEW_H

#include "tree_model.h"

#include <QAction>
#include <QMenu>
#include <QTreeView>

class ClickableTreeView : public QTreeView
{
  Q_OBJECT

private slots:
  void context_menu(const QPoint &pos);

public:
  ClickableTreeView(QWidget *parent = nullptr);
  void setModel(TreeModel *model);
  TreeModel *model() const;
  void expand_tracked();
  void prettify();

signals:
  void cleared(const QModelIndex &index);
  void track_folder(const QModelIndex index);
};

#endif