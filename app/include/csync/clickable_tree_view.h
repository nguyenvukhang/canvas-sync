#ifndef CLICKABLE_TREE_VIEW_H
#define CLICKABLE_TREE_VIEW_H

#include "tree_model.h"

#include <QAction>
#include <QCheckBox>
#include <QMenu>
#include <QTreeView>

class Actions
{
public:
  QAction *clear;
  QAction *track_folder;
};

class ClickableTreeView : public QTreeView
{
  Q_OBJECT

public:
  ClickableTreeView(QWidget *parent = nullptr);
  void context_menu(const QPoint &pos);
  void setModel(TreeModel *model);
  TreeModel *model() const;
  void expand_tracked();
  void prettify();
  QMenu menu;
  Actions actions;

signals:
  void cleared(const QModelIndex &index);
  void track_folder(const QModelIndex index);
};

#endif
