#include "filetree.h"
#include <algorithm>
#include <string>

void debug(FileTree *t, int level) {
  string indent = string(2 * (level + 1), ' ');
  cout << indent << t->id << ", " << t->name << endl;
  for (FileTree nested : t->folders) {
    debug(&nested, level + 1);
  }
  for (auto file : t->files) {
    cout << indent << file.filename << endl;
  }
}
void debug(FileTree *t) {
  cerr << "FileTree" << endl;
  debug(t, 0);
}

FileTree::FileTree(const int id, const char *name) {
  this->name = name;
  this->id = id;
}

FileTree::FileTree(const int id, const string name) {
  this->name = name;
  this->id = id;
}

FileTree::FileTree(Folder *f) {
  this->name = f->name;
  this->id = f->id;
}

FileTree::FileTree(Course *c) {
  this->name = c->name;
  this->id = c->id;
}

void FileTree::to_string(string *state) {
  *state += "{(" + std::to_string(this->id) + ',' + this->name + "):";
  for (FileTree nested : this->folders) {
    nested.to_string(state);
  }
  *state += '}';
}

string FileTree::to_string() {
  string state = "";
  this->to_string(&state);
  return state;
}

bool compareFolderPath(Folder f1, Folder f2) {
  return f1.full_name < f2.full_name;
}

void FileTree::insert_folder(Folder *f) {
  this->insert_tree(new FileTree(f), f->full_name);
}

void FileTree::insert_folders(vector<Folder> folders) {
  sort(folders.begin(), folders.end(), compareFolderPath);
  for (Folder f : folders)
    this->insert_folder(&f);
}

void FileTree::insert_tree(FileTree *t, string state) {
  int slash_idx = state.find('/');

  // no more folders to traverse. insert here.
  if (slash_idx == string::npos) {
    this->folders.push_back(*t);
    return;
  }

  // find the next folder to go recurse into.
  string query = state.substr(0, slash_idx);
  int size = this->folders.size();
  for (int i = 0; i < size; i++) {
    if (this->folders[i].name != query)
      continue;
    this->folders[i].insert_tree(t, state.substr(slash_idx + 1));
    break;
  }
}

void FileTree::insert_tree(FileTree *t) { this->insert_tree(t, t->name); }
