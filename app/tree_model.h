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

QString get_id(const QModelIndex &);
QString get_local_dir(const QModelIndex &);
QString get_remote_dir(const QModelIndex &);
QString get_course(const QModelIndex &);
QString get_ancestry(const QModelIndex &, const char *delimiter);
const QModelIndex get_child(const QModelIndex &, const int index);
const int count_children(const QModelIndex &);

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

private:
  void resolve_folders(TreeItem *item, std::filesystem::path *local_base_dir,
                       std::filesystem::path *cwd, std::vector<Folder> *list);

public:
  std::vector<Folder> resolve_folders();

  enum TreeCol { REMOTE_DIR, LOCAL_DIR, FOLDER_ID };
  QString get_id() const { return data(FOLDER_ID).toString(); };
  QString get_local_dir() const { return data(LOCAL_DIR).toString(); };
  QString get_remote_dir() const { return data(REMOTE_DIR).toString(); };

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
