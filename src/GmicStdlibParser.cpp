/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file GmicStdlibParser.cpp
 *
 *  Copyright 2017 Sebastien Fourey
 *
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "GmicStdlibParser.h"
#include "Common.h"
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QList>
#include <QString>
#include <QRegExp>
#include <QBuffer>
#include <QTreeView>
#include <QStandardItem>
#include "FiltersTreeFolderItem.h"
#include "FiltersTreeFilterItem.h"
#include "gmic.h"

QByteArray GmicStdLibParser::GmicStdlib;

GmicStdLibParser::GmicStdLibParser()
{
}

void GmicStdLibParser::buildFiltersTree(QTreeView * treeView, QStandardItemModel & model)
{
  if ( GmicStdlib.isEmpty() ) {
    loadStdLib();
  }
  QBuffer stdlib(&GmicStdlib);
  stdlib.open(QBuffer::ReadOnly);
  QList<QStandardItem*> treeFoldersStack;
  QList<QString> filterPath;

  treeView->setModel(&model);
  model.setHorizontalHeaderItem(0,new QStandardItem(QObject::tr("Available filters")));

  treeFoldersStack.push_back(model.invisibleRootItem());

  QString language;

  QList<QString> languages = QLocale().uiLanguages();
  if ( languages.size() ) {
    language = languages.front().split("-").front();
  } else {
    language = "void";
  }
  if ( ! GmicStdlib.contains(QString("#@gui_%1").arg(language).toLocal8Bit()) ) {
    language = "en";
  }

  // Use _en locale if not localization for the language is found.

  QString buffer = stdlib.readLine(4096);
  QString line;

  QRegExp folderRegexpNoLanguage("^..gui[ ][^:]+$");
  QRegExp folderRegexpLanguage( QString("^..gui_%1[ ][^:]+$").arg(language));

  QRegExp filterRegexpNoLanguage("^..gui[ ][^:]+[ ]*:.*");
  QRegExp filterRegexpLanguage( QString("^..gui_%1[ ][^:]+[ ]*:.*").arg(language));

  int maxDepth = 1;
  const QChar WarningPrefix('!');
  do {
    line = buffer.trimmed();
    if ( line.startsWith("#@gui") ) {
      if ( folderRegexpNoLanguage.exactMatch(line) || folderRegexpLanguage.exactMatch(line) ) {
        //
        // A folder
        //
        QString folderName = line;
        folderName.replace(QRegExp("^..gui[_a-zA-Z]{0,3}[ ]"), "" );

        while ( folderName.startsWith("_") && (treeFoldersStack.size() > 1) ) {
          folderName.remove(0,1);
          treeFoldersStack.pop_back();
          filterPath.pop_back();
        }
        while ( folderName.startsWith("_") ) {
          folderName.remove(0,1);
        }
        const bool warning = folderName.startsWith(WarningPrefix);
        if ( warning ) {
          folderName.remove(0,1);
        }
        if ( ! folderName.isEmpty() ) {
          // Does this folder already exists
          FiltersTreeFolderItem * folderItem = 0;
          {
            QStandardItem * parentFolder = treeFoldersStack.last();
            int n = parentFolder->rowCount();
            for (int i = 0; i < n && !folderItem; ++i) {
              FiltersTreeFolderItem * folder  = dynamic_cast<FiltersTreeFolderItem*>(parentFolder->child(i));
              if (folder && folder->name() == folderName) {
                folderItem = folder;
              }
            }
          }
          if ( ! folderItem ) {
            treeFoldersStack.last()->appendRow(folderItem = new FiltersTreeFolderItem(folderName,FiltersTreeFolderItem::NormalFolder));
            folderItem->setWarningFlag(warning);
          }
          treeFoldersStack.push_back(folderItem);
          filterPath.push_back(folderName);
        }
        maxDepth = std::max( maxDepth, treeFoldersStack.size() );
        buffer = stdlib.readLine(4096);
      } else if ( filterRegexpNoLanguage.exactMatch(line) || filterRegexpLanguage.exactMatch(line) ) {
        //
        // A filter
        //
        QString filterName = line;
        filterName.replace( QRegExp("[ ]*:.*$"), "" );
        filterName.replace( QRegExp("^..gui[_a-zA-Z]{0,3}[ ]"), "" );

        const bool warning = filterName.startsWith(WarningPrefix);
        if ( warning ) {
          filterName.remove(0,1);
        }

        QString filterCommands = line;
        filterCommands.replace(QRegExp("^..gui[_a-zA-Z]{0,3}[ ][^:]+[ ]*:[ ]*"),"");

        QList<QString> commands = filterCommands.split(",");

        QString filterCommand = commands[0].trimmed();
        QList<QString> preview = commands[1].trimmed().split("(");
        float previewFactor = GmicQt::PreviewFactorAny;
        bool accurateIfZoomed = true;
        if ( preview.size() >= 2 ) {
          if (preview[1].endsWith("+")) {
            accurateIfZoomed = true;
            preview[1].chop(1);
          } else {
            accurateIfZoomed = false;
          }
          previewFactor = preview[1].replace(QRegExp("\\).*"),"").toFloat();
        }
        QString filterPreviewCommand = preview[0].trimmed();
        FiltersTreeFilterItem * filterItem = new FiltersTreeFilterItem(filterName,
                                                                       filterCommand,
                                                                       filterPreviewCommand,
                                                                       previewFactor,
                                                                       accurateIfZoomed);
        filterItem->setWarningFlag(warning);
        treeFoldersStack.back()->appendRow(filterItem);

        QString start = line;
        start.replace(QRegExp(" .*")," :");

        // Read parameters
        QString parameters;
        do {
          buffer = stdlib.readLine(4096);
          if ( buffer.startsWith(start) ) {
            QString parameterLine = buffer;
            parameterLine.replace(QRegExp("^..gui[_a-zA-Z]{0,3}[ ]*:[ ]*"), "");
            parameters += parameterLine;
          }
        } while ( (buffer.startsWith(start)
                   || buffer.startsWith("#")
                   || (buffer.trimmed().isEmpty() && ! stdlib.atEnd()))
                  && !folderRegexpNoLanguage.exactMatch(buffer)
                  && !folderRegexpLanguage.exactMatch(buffer)
                  && !filterRegexpNoLanguage.exactMatch(buffer)
                  && !filterRegexpLanguage.exactMatch(buffer));
        filterItem->setParameters(parameters);
      } else {
        buffer = stdlib.readLine(4096);
      }
    } else {
      buffer = stdlib.readLine(4096);
    }
  } while ( ! buffer.isEmpty() );

  int count = FiltersTreeAbstractItem::countLeaves(model.invisibleRootItem());
  model.setHorizontalHeaderItem(0,new QStandardItem(QString(QObject::tr("Available filters (%1)")).arg(count)));
}

void
GmicStdLibParser::loadStdLib()
{
  QFile stdlib(QString("%1update%2.gmic").arg(GmicQt::path_rc(false)).arg(gmic_version));
  if ( ! stdlib.open(QFile::ReadOnly) ) {
    gmic_image<char> stdlib_h = gmic::decompress_stdlib();
    GmicStdlib = QByteArray::fromRawData(stdlib_h,stdlib_h.size());
    GmicStdlib[GmicStdlib.size()-1] = '\n';
  } else {
    GmicStdlib = stdlib.readAll();
  }
}

QStringList GmicStdLibParser::parseStatus(QString str)
{
  if ( !str.startsWith(QChar(24)) || !str.endsWith(QChar(25 )) ) {
    return QStringList();
  }
  QList<QString> list = str.split(QString("%1%2").arg(QChar(25)).arg(QChar(24)));
  list[0].replace(QChar(24),QString());
  list.back().replace(QChar(25),QString());
  return list;
}
