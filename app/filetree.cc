#include "filetree.h"

void FileTree::insert_folder(Folder &f, std::string &state)
{
  size_t slash_idx = state.find('/');

  // no more folders to traverse. insert here.
  if (slash_idx == std::string::npos) {
    this->folders.push_back(*new FileTree(f));
    return;
  }

  // find the next folder to go recurse into.
  std::string query = state.substr(0, slash_idx);
  for (size_t i = 0; i < this->folders.size(); i++) {
    if (this->folders[i].name != query)
      continue;
    std::string next_state = state.substr(slash_idx + 1);
    this->folders[i].insert_folder(f, next_state);
    break;
  }
}

void FileTree::insert_folder(Folder &f)
{
  insert_folder(f, f.full_name);
};

void FileTree::insert_course_tree(FileTree &t)
{
  this->folders.push_back(std::move(t));
}

void FileTree::to_string(std::string &state)
{
  state += "{(" + std::to_string(this->id) + ',' + this->name + "):";
  for (FileTree t : this->folders)
    t.to_string(state);
  state += '}';
}

std::string FileTree::to_string()
{
  std::string state = "";
  this->to_string(state);
  return state;
}

void debug(const FileTree &t, int level)
{
  std::string indent = std::string(2 * (level + 1), ' ');
  std::cout << indent << t.id << ", " << t.name << std::endl;
  for (FileTree nested : t.folders)
    debug(nested, level + 1);
  for (auto file : t.files)
    std::cout << indent << file.filename << std::endl;
}
void debug(const FileTree &t)
{
  std::cerr << "FileTree" << std::endl;
  debug(t, 0);
}
