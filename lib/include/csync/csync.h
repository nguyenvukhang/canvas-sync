#ifndef CSYNC_H
#define CSYNC_H

#include "canvas.h"
#include "convert.h"
#include "tree_view.h"
#include "filetree.h"
#include "tree_model.h"
#include "types.h"

#include <QObject>
#include <QString>

class CSync : public QObject
{
  Q_OBJECT

public:
  CSync();
  QString send();
};

#endif
