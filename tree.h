#ifndef CANVAS_SYNC_TREE_H
#define CANVAS_SYNC_TREE_H

#include "filetree.h"
#include "tree_model.h"

#include <filesystem>

enum TreeCol { REMOTE_DIR, LOCAL_DIR, FOLDER_ID };

void insert(TreeItem *item, FileTree *tree, QSettings *settings);

QString get_id(const QModelIndex &);
QString get_local_dir(const QModelIndex &);
QString get_remote_dir(const QModelIndex &);
QString get_course(const QModelIndex &);
QString get_ancestry(const QModelIndex &, const char *delimiter);

QString get_id(const TreeItem &);
QString get_local_dir(const TreeItem &);
QString get_remote_dir(const TreeItem &);

std::vector<Folder> resolve_all_folders(TreeItem *item);
std::vector<Folder> gather_tracked(TreeModel *model);

using ItemOperator = std::function<void(TreeItem *item)>;
void on_all_children(TreeItem *item, ItemOperator);
void on_all_parents(TreeItem *item, ItemOperator);

TreeModel *newTreeModel();

#endif
