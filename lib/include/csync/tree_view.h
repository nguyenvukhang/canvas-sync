#ifndef treeview
#define treeview

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

class TreeView : public QTreeView
{
  Q_OBJECT

public:
  TreeView(QWidget *parent = nullptr);
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
