#ifndef CANVAS_SYNC_TREE_H
#define CANVAS_SYNC_TREE_H

#include "filetree.h"
#include "tree_model.h"

#include <filesystem>

enum TreeCol { REMOTE_DIR, LOCAL_DIR, FOLDER_ID };

QString get_id(const QModelIndex &);
QString get_local_dir(const QModelIndex &);
QString get_remote_dir(const QModelIndex &);
QString get_course(const QModelIndex &);
QString get_ancestry(const QModelIndex &, const char *delimiter);

TreeModel *newTreeModel();

#endif
