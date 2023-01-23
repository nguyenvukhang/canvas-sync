#include "filetree.h"
#include <gtest/gtest.h>

class FileTreeTest : public ::testing::Test {
protected:
  void SetUp() override {}
  // convenience routine to create an identifiable folder
  Folder f(const int id, const char *full_name) {
    Folder f;
    f.full_name = full_name;
    f.id = id;
    int idx = f.full_name.rfind('/');
    f.name = idx == string::npos ? full_name : f.full_name.substr(idx + 1);
    return f;
  }
  // create a tree from the list of folders
  FileTree t(Folder *f, int arr_len) {
    FileTree *tree = new FileTree(0, "root");
    vector<Folder> folders;
    for (int i = 0; i < arr_len; i++)
      folders.push_back(f[i]);
    tree->insert_folders(folders);
    return *tree;
  }
};

TEST_F(FileTreeTest, InsertSorted) {
  Folder a[] = {f(1, "foo"), f(2, "foo/bar"), f(3, "foo/bar/baz")};
  EXPECT_EQ(t(a, 3).to_string(), "{(0,root):{(1,foo):{(2,bar):{(3,baz):}}}}");
}

TEST_F(FileTreeTest, Unsorted) {
  Folder a[] = {f(2, "foo/bar"), f(1, "foo"), f(3, "foo/bar/baz")};
  EXPECT_EQ(t(a, 3).to_string(), "{(0,root):{(1,foo):{(2,bar):{(3,baz):}}}}");
}
