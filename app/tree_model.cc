#include "tree_model.h"

QVector<QVariant> stringListToVariantList(const QStringList &data)
{
  QVector<QVariant> variantData;
  variantData.reserve(data.count());
  for (auto &string : data) {
    variantData.append(QVariant(string));
  }
  return variantData;
}

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
    : itemData(data), parentItem(parent){};

TreeItem::TreeItem(const QStringList &data, TreeItem *parent)
    : TreeItem(stringListToVariantList(data), parent){};

void TreeItem::appendChild(TreeItem *item)
{
  if (!item) return;
  TreeItem *currentParent = item->parent();
  if (currentParent) currentParent->removeChild(item);
  item->setParent(this);
  childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
  return row < 0 || row >= childItems.size() ? nullptr : childItems.at(row);
}

int TreeItem::childrenColumnCount() const
{
  int maxCount = 0;
  for (auto &child : childItems)
    maxCount = std::max(maxCount, child->columnCount());
  return maxCount;
}

QVariant TreeItem::data(int col) const
{
  return col < 0 || col >= columnCount() ? QVariant() : itemData.at(col);
}

int TreeItem::row() const
{
  if (!parentItem) return 0;
  return parentItem->childItems.indexOf(const_cast<TreeItem *>(this));
}

void TreeItem::removeRow(int row)
{
  if (row < 0 || row >= rowCount()) return;
  childItems.remove(row);
}

void TreeItem::removeRows(int row, int count)
{
  for (int i = 0; i < count; ++i)
    childItems.remove(row);
}

void TreeItem::setParent(TreeItem *parent)
{
  if (!parent) return;
  parentItem = parent;
}

void TreeItem::removeColumns(int position, int columns)
{
  if (position < 0 || position + columns > itemData.size()) {
    throw std::out_of_range("removeColumns: invalid arguments");
  }

  for (int column = 0; column < columns; column++) {
    itemData.remove(position);
  }

  for (const auto &child : childItems) {
    child->removeColumns(position, columns);
  }
}

void TreeItem::insertColumns(int position, int columns)
{
  if (position < 0 || position > itemData.size()) {
    throw std::out_of_range("insertColumns: invalid arguments");
  }

  for (int column = 0; column < columns; ++column) {
    itemData.insert(position, QVariant());
  }

  for (const auto &child : childItems) {
    child->insertColumns(position, columns);
  }
}

void TreeItem::insertRows(int position, int count, int columns)
{
  if (position < 0 || position > childItems.size()) {
    throw std::out_of_range("insertRows: invalid arguments");
  }

  for (int row = 0; row < count; ++row) {
    QVector<QVariant> data(columns);
    auto item = new TreeItem(data, this);
    childItems.insert(position, item);
  }
}

void TreeItem::insert(const FileTree &tree, Settings &settings)
{
  size_t child_count = tree.folders.size();
  if (child_count == 0) return;
  for (size_t i = 0; i < child_count; i++) {
    FileTree child = tree.folders[i];
    QString id = QString::fromStdString(std::to_string(child.id));
    QString name = QString::fromStdString(child.name);
    QString dir = settings.local_dir(id);
    TreeItem ti(QStringList() << name << dir << id);
    this->appendChild(new TreeItem(QStringList() << name << dir << id));
    this->child(i)->setChecked(settings.is_tracked(id));
    this->child(i)->insert(child, settings);
  }
}

void TreeItem::on_all_parents(std::function<void(TreeItem &item)> f)
{
  TreeItem *p = this->parent();
  while (p) {
    f(*p);
    p = p->parent();
  }
}

void TreeItem::on_all_children(std::function<void(TreeItem &item)> f)
{
  int n = this->childrenItems().size();
  for (int i = 0; i < n; i++) {
    f(*this->child(i));
    this->child(i)->on_all_children(f);
  }
}

void TreeItem::track_folder(const TreeIndex &ti, const QString &dir,
                            Settings &settings)
{
  // clear children maps
  on_all_children([&](TreeItem &child) {
    child.setData(TreeIndex::LOCAL_DIR, "");
    QString folder_id = child.id();
    if (!folder_id.isEmpty()) settings.set(folder_id, Settings::TRACKED, false);
  });

  // clear parent maps
  on_all_parents([&](TreeItem &parent) {
    parent.setData(TreeIndex::LOCAL_DIR, "");
    QString folder_id = parent.id();
    if (!folder_id.isEmpty()) settings.set(folder_id, Settings::TRACKED, false);
  });

  // update itself
  this->setData(TreeIndex::LOCAL_DIR, dir);
  settings.set(ti.id(), Settings::LOCAL_DIR, dir);
  settings.set(ti.id(), Settings::TRACKED, true);
}

void resolve_all_folders(TreeItem *item, std::filesystem::path *local_base_dir,
                         std::filesystem::path *cwd, std::vector<Folder> *list)
{
  std::string local_dir = item->local_dir().toStdString();

  std::filesystem::path new_cwd;
  std::filesystem::path new_base = *local_base_dir;

  if (!local_base_dir->empty()) {
    new_cwd = *cwd / item->remote_dir().toStdString();
  }

  // one-time thing. on any path downwards it is guaranteed to only have
  // one occurrence of a non-empty local path.
  if (!local_dir.empty()) {
    new_base = local_dir;
  }

  if (!new_base.empty()) {
    list->push_back(Folder(item->id().toInt(), new_base / new_cwd));
  }

  auto children = item->childrenItems();
  for (auto child : children)
    resolve_all_folders(child, &new_base, &new_cwd, list);
}

// assume here that `this` contains information about a course,
// and thus is not included in the resolution of the path.
//
// All updates from one call of this method belongs to the same course
std::vector<Folder> TreeItem::resolve_folders()
{
  std::vector<Folder> fl;
  auto children = this->childrenItems();
  for (auto child : children) {
    std::filesystem::path p = "";
    std::filesystem::path base = "";
    resolve_all_folders(child, &base, &p, &fl);
  }
  int id = this->id().toInt();
  for (size_t i = 0; i < fl.size(); i++) {
    fl[i].course_id = id;
  }
  return fl;
}

void TreeItem::setData(int column, const QVariant &value)
{
  if (column < 0 || column >= itemData.size()) {
    throw std::out_of_range("setData: invalid arguments");
  }

  itemData[column] = value;
}

TreeModel::TreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent), readOnly(false)
{
  QVector<QVariant> rootData;
  for (const QString &header : headers) {
    rootData << header;
  }

  rootItem = new TreeItem(rootData);
}

TreeModel::~TreeModel()
{
  delete rootItem;
}

void TreeModel::setReadOnly(bool flag)
{
  readOnly = flag;
}

bool TreeModel::isReadOnly() const
{
  return readOnly;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return static_cast<TreeItem *>(parent.internalPointer())
        ->childrenColumnCount();
  }
  return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }
  TreeItem *item = static_cast<TreeItem *>(index.internalPointer());

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return item->data(index.column());
  }

  if (role == Qt::CheckStateRole && index.column() == TreeIndex::REMOTE_DIR)
    return item->isChecked() ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::NoItemFlags;
  if (!index.isValid()) return flags;
  if (readOnly) return QAbstractItemModel::flags(index);

  flags |= Qt::ItemIsEnabled;
  TreeItem *item = itemFromIndex(index);

  if (index.column() == TreeIndex::LOCAL_DIR) {
    if (item->has_local_dir()) flags |= Qt::ItemIsSelectable;

    TreeItem *remote =
        itemFromIndex(index.siblingAtColumn(TreeIndex::REMOTE_DIR));
    if (!remote->isChecked()) flags &= ~Qt::ItemIsEnabled;
  }
  if (index.column() == TreeIndex::REMOTE_DIR) flags |= Qt::ItemIsUserCheckable;
  return flags;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return rootItem->data(section);
  }

  return QVariant();
}

QModelIndex TreeModel::index(int row, int column,
                             const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  TreeItem *parentItem;

  if (!parent.isValid()) {
    parentItem = rootItem;
  } else {
    parentItem = static_cast<TreeItem *>(parent.internalPointer());
  }

  auto childItem = parentItem->child(row);

  if (childItem) {
    return createIndex(row, column, childItem);
  }

  return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid()) return QModelIndex();

  TreeItem *child = static_cast<TreeItem *>(index.internalPointer());
  TreeItem *parent = child->parent();

  if (parent == rootItem) return QModelIndex();

  return createIndex(parent->row(), 0, parent);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0) return 0;
  TreeItem *p = parent.isValid()
                    ? static_cast<TreeItem *>(parent.internalPointer())
                    : this->rootItem;
  return p->rowCount();
}

void TreeModel::appendRow(const QStringList &data)
{
  rootItem->appendChild(new TreeItem(stringListToVariantList(data), rootItem));
}

void TreeModel::setHorizontalHeaderLabels(const QStringList &labels)
{
  rootItem->setData(stringListToVariantList(labels));
}

TreeItem *TreeModel::itemFromIndex(const QModelIndex &index) const
{
  if (index.isValid()) {
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    if (item) return item;
  }
  return rootItem;
}

TreeItem *TreeModel::item(int row, int column) const
{
  return itemFromIndex(index(row, column));
}

QModelIndex TreeModel::indexFromItem(TreeItem *item) const
{
  if (item == rootItem) return QModelIndex();
  return createIndex(item->row(), 0, item);
}

bool TreeModel::removeColumns(int position, int columns,
                              const QModelIndex &parent)
{
  beginRemoveColumns(parent, position, position + columns - 1);
  try {
    rootItem->removeColumns(position, columns);
  } catch (std::out_of_range &e) {
    endRemoveColumns();
    return false;
  }
  endRemoveColumns();
  if (rootItem->columnCount() == 0) removeRows(0, rowCount());
  return true;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
  TreeItem *parentItem = itemFromIndex(parent);
  if (!parentItem) return false;

  beginRemoveRows(parent, position, position + rows - 1);
  try {
    parentItem->removeRows(position, rows);
  } catch (std::out_of_range &e) {
    endRemoveRows();
    return false;
  }
  endRemoveRows();

  return true;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
  TreeItem *item = itemFromIndex(index);
  if (role == Qt::CheckStateRole) {
    item->setChecked(!item->isChecked());
    emit dataChanged(index.siblingAtColumn(TreeIndex::REMOTE_DIR),
                     index.siblingAtColumn(TreeIndex::LOCAL_DIR));
    return true;
  };
  return true;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
  if (role != Qt::EditRole || orientation != Qt::Horizontal) {
    return false;
  }

  try {
    rootItem->setData(section, value);
  } catch (std::out_of_range &e) {
    return false;
  }

  emit headerDataChanged(orientation, section, section);
  return true;
}

bool TreeModel::insertColumns(int position, int columns,
                              const QModelIndex &parent)
{
  beginInsertColumns(parent, position, position + columns - 1);
  try {
    rootItem->insertColumns(position, columns);
  } catch (std::out_of_range &e) {
    endInsertColumns();
    return false;
  }
  endInsertColumns();
  return true;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  auto parentItem = itemFromIndex(parent);
  if (!parentItem) {
    return false;
  }

  beginInsertRows(parent, position, position + rows - 1);
  try {
    parentItem->insertRows(position, rows, rootItem->columnCount());
  } catch (std::out_of_range &e) {
    endInsertRows();
    return false;
  }
  endInsertRows();

  return true;
}

std::vector<Folder> TreeModel::gather_tracked()
{
  std::vector<Folder> all;
  size_t n = this->childrenCount();
  for (size_t i = 0; i < n; i++) {
    std::vector<Folder> u = this->item(i)->resolve_folders();
    for (auto u : u)
      all.push_back(std::move(u));
  }
  return all;
}
