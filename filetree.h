#ifndef CANVAS_SYNC_FILETREE_H
#define CANVAS_SYNC_FILETREE_H

#include "types.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

class FileTree
{
public:
  int id; // either course id or folder id.
  std::string name;
  std::vector<FileTree> folders;
  std::vector<File> files;

private:
  // for recursion with the public variants
  void insert_folder(Folder &, std::string &);
  void insert_folder_tree(FileTree &, std::string &);
  void to_string(std::string &);

  // constructors
public:
  FileTree() : id(0), name("root"){};
  FileTree(const int id, const char *n) : id(id), name(n){};
  FileTree(const int id, const std::string n) : id(id), name(n){};
  FileTree(const Folder *f) : id(f->id), name(f->name){};
  FileTree(const Folder &f) : id(f.id), name(f.name){};
  FileTree(const Course *c) : id(c->id), name(c->name){};
  FileTree(const Course *c, std::vector<Folder> &f) : id(c->id), name(c->name)
  {
    this->insert_folders(f);
  };

  // insertions
  void insert_folder(Folder &);
  void insert_course_tree(FileTree &);

  // iterated versions
  void insert_folders(std::vector<Folder> &folders)
  {
    sort(folders.begin(), folders.end(),
         [](Folder &a, Folder &b) { return a.full_name < b.full_name; });
    for (Folder f : folders)
      this->insert_folder(f);
  }
  void insert_course_trees(std::vector<FileTree> &t)
  {
    sort(t.begin(), t.end(),
         [](FileTree &a, FileTree &b) { return a.name < b.name; });
    for (FileTree t : t)
      this->insert_course_tree(t);
  };

public:
  // debug tools
  std::string to_string();
};
void debug(const FileTree &);

#endif
