#include "filetree.h"
#include <algorithm>
#include <string>

void debug(FileTree *t, int level)
{
  std::string indent = std::string(2 * (level + 1), ' ');
  std::cout << indent << t->id << ", " << t->name << std::endl;
  for (FileTree nested : t->folders) {
    debug(&nested, level + 1);
  }
  for (auto file : t->files) {
    std::cout << indent << file.filename << std::endl;
  }
  // if (t->files.empty()) {
  //   cout << indent << "[no files]" << endl;
  // }
}
void debug(FileTree *t)
{
  std::cerr << "FileTree" << std::endl;
  debug(t, 0);
}

void FileTree::to_string(std::string *state)
{
  *state += "{(" + std::to_string(this->id) + ',' + this->name + "):";
  for (FileTree nested : this->folders) {
    nested.to_string(state);
  }
  *state += '}';
}

std::string FileTree::to_string()
{
  std::string state = "";
  this->to_string(&state);
  return state;
}

bool compareFolderPath(Folder f1, Folder f2)
{
  return f1.full_name < f2.full_name;
}

void FileTree::insert_folder(Folder *f)
{
  this->insert_tree(new FileTree(f), f->full_name);
}

void FileTree::insert_folders(std::vector<Folder> folders)
{
  sort(folders.begin(), folders.end(), compareFolderPath);
  for (Folder f : folders)
    this->insert_folder(&f);
}

void FileTree::insert_tree(FileTree *t, std::string state)
{
  size_t slash_idx = state.find('/');

  // no more folders to traverse. insert here.
  if (slash_idx == std::string::npos) {
    this->folders.push_back(*t);
    return;
  }

  // find the next folder to go recurse into.
  std::string query = state.substr(0, slash_idx);
  size_t size = this->folders.size();
  for (size_t i = 0; i < size; i++) {
    if (this->folders[i].name != query)
      continue;
    this->folders[i].insert_tree(t, state.substr(slash_idx + 1));
    break;
  }
}

void FileTree::insert_tree(FileTree *t)
{
  this->insert_tree(t, t->name);
}
