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

using ItemOperator = std::function<void(TreeItem *item)>;
void on_all_children(TreeItem *item, ItemOperator);
void on_all_parents(TreeItem *item, ItemOperator);

TreeModel *newTreeModel();

#endif
