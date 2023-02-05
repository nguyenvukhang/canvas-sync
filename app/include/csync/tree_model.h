#ifndef TREE_MODEL_H
#define TREE_MODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "filetree.h"

class TreeIndex : public QModelIndex
{
  QString data(int i) const { return siblingAtColumn(i).data().toString(); }

public:
  enum TreeCol { REMOTE_DIR, LOCAL_DIR, FOLDER_ID };
  TreeIndex(const QModelIndex i) : QModelIndex(i){};

  const QModelIndex index() { return this->index(); }
  QString id() const { return data(TreeCol::FOLDER_ID); }
  QString local_dir() const { return data(TreeCol::LOCAL_DIR); }
  QString remote_dir() const { return data(TreeCol::REMOTE_DIR); }

  QString course() const
  {
    const TreeIndex *a = this;
    while (a->parent().isValid())
      a = new TreeIndex(a->parent());
    return a->remote_dir();
  }

  QString get_ancestry(const char *delimiter)
  {
    QString path;
    TreeIndex *a = this;
    path = this->remote_dir();
    while (a->parent().isValid()) {
      a = new TreeIndex(a->parent());
      QString d = a->remote_dir();
      if (d.size() > 10) {
        d.truncate(10);
        d.push_back("...");
      }
      path = d + delimiter + path;
    }
    return path;
  }

  const TreeIndex child(const int index)
  {
    return TreeIndex(model()->index(index, 0, this->index()));
  }

  const int children_count()
  {
    int i = 0;
    const QAbstractItemModel *m = this->model();
    while (m->index(i, 0, this->index()).isValid())
      i++;
    return i;
  }
};

class TreeItem
{
public:
  TreeItem(const QVector<QVariant> &d, TreeItem *p = nullptr);
  TreeItem(const QStringList &data, TreeItem *parentItem = nullptr);
  ~TreeItem() { qDeleteAll(childItems); };

  void appendChild(TreeItem *child);
  void setData(const QVector<QVariant> &data) { itemData = data; };
  void setParent(TreeItem *parent);

  TreeItem *child(int row);
  QVector<TreeItem *> childrenItems() { return childItems; };
  int rowCount() const { return childItems.count(); };
  int columnCount() const { return itemData.count(); };
  int childrenColumnCount() const;
  QVariant data(int column) const;
  QVector<QVariant> data() const { return itemData; };
  int row() const;
  void removeRow(int row);
  void removeRows(int row, int count);
  void removeChild(TreeItem *child) { childItems.removeOne(child); };
  void removeAllChildren() { childItems.clear(); };
  TreeItem *parent() { return parentItem; };
  void removeColumns(int position, int columns);
  void setData(int column, const QVariant &value);
  void insertColumns(int position, int columns);
  void insertRows(int position, int count, int columns);
  void insert(const FileTree &, const QSettings &);
  void on_all_parents(std::function<void(TreeItem &item)>);
  void on_all_children(std::function<void(TreeItem &item)>);
  void track_folder(const TreeIndex &, const QString &, QSettings &);

private:
  void resolve_folders(TreeItem *item, std::filesystem::path *local_base_dir,
                       std::filesystem::path *cwd, std::vector<Folder> *list);
  QString str_data(TreeIndex::TreeCol i) const { return data(i).toString(); };

public:
  std::vector<Folder> resolve_folders();
  QString get_id() const { return str_data(TreeIndex::FOLDER_ID); };
  QString get_local_dir() const { return str_data(TreeIndex::LOCAL_DIR); };
  QString get_remote_dir() const { return str_data(TreeIndex::REMOTE_DIR); };

private:
  QVector<QVariant> itemData;
  TreeItem *parentItem;
  QVector<TreeItem *> childItems;
};

class TreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  TreeModel(const QStringList &headers, QObject *parent = nullptr);
  ~TreeModel();

  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  QModelIndex indexFromItem(TreeItem *item) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  void appendRow(const QStringList &data);
  void setHorizontalHeaderLabels(const QStringList &labels);
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::EditRole) override;
  bool insertColumns(int position, int columns,
                     const QModelIndex &parent = QModelIndex()) override;
  bool removeColumns(int position, int columns,
                     const QModelIndex &parent = QModelIndex()) override;
  bool insertRows(int position, int rows,
                  const QModelIndex &parent = QModelIndex()) override;
  bool removeRows(int position, int rows,
                  const QModelIndex &parent = QModelIndex()) override;

  TreeItem *item(int row, int column = 0) const;
  TreeItem *itemFromIndex(const QModelIndex &index) const;
  void setReadOnly(bool flag);
  bool isReadOnly() const;

  // custom functions
  int childrenCount() { return this->rowCount(); }
  std::vector<Folder> gather_tracked();

private:
  TreeItem *rootItem;
  bool readOnly;
};

#endif // TREEMODEL_H
