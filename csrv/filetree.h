#ifndef CANVAS_SYNC_FILETREE_H
#define CANVAS_SYNC_FILETREE_H

#include "types.h"
#include <vector> // for std::vector

using namespace std;

class FileTree {
private:
  // for recursion with the public variant
  void insert_tree(FileTree *, string);
  void to_string(string *);

public:
  FileTree() {
    this->id = 0;
    this->name = "root";
  }
  FileTree(const int, const char *);
  FileTree(const int, const string);
  FileTree(Folder *);
  FileTree(Course *);
  int id; // either course id or folder id.
  string name;
  vector<FileTree> folders;
  vector<File> files;
  void insert_folder(Folder *);
  void insert_folders(vector<Folder>);
  void insert_tree(FileTree *);
  void insert_file(File *);
  string to_string();
};
void debug(FileTree *);

#endif
