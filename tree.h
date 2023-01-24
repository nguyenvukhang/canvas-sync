#ifndef CANVAS_SYNC_TREE_H
#define CANVAS_SYNC_TREE_H

#include "filetree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"

#include <QSettings>

void insert(TreeItem *item, FileTree *tree, QSettings *settings);

QString get_id(const QModelIndex &index);
QString get_local_dir(const QModelIndex &index);

void expand_tracked(ClickableTreeView *tree);
void fix_tree(Ui::MainWindow *ui);

void clear_children(TreeItem *item, int index);

TreeModel *newTreeModel();

#endif
