#ifndef CANVAS_SYNC_FILETREE_H
#define CANVAS_SYNC_FILETREE_H

#include "types.h"
#include <vector> // for std::vector

class FileTree
{
private:
  // for recursion with the public variant
  void insert_tree(FileTree *, std::string);
  void to_string(std::string *);

public:
  FileTree()
  {
    this->id = 0;
    this->name = "root";
  }
  FileTree(const int id, const char *n) : id(id), name(n){};
  FileTree(const int id, const std::string n) : id(id), name(n){};
  FileTree(const Folder *f) : id(f->id), name(f->name){};
  FileTree(const Course *c) : id(c->id), name(c->name){};
  int id; // either course id or folder id.
  std::string name;
  std::vector<FileTree> folders;
  std::vector<File> files;
  void insert_folder(Folder *);
  void insert_folders(std::vector<Folder>);
  void insert_tree(FileTree *);
  void insert_file(File *);
  std::string to_string();
};
void debug(FileTree *);

#endif
