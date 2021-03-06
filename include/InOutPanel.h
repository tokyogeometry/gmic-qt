/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file InOutPanel.h
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
#ifndef _GMIC_QT_INOUTPANEL_H
#define _GMIC_QT_INOUTPANEL_H

#include <QGroupBox>
#include "gmic_qt.h"
#include "host.h"

namespace Ui {
class InOutPanel;
}

class QSettings;
class FilterThread;

class InOutPanel : public QGroupBox
{
  Q_OBJECT

public:
  explicit InOutPanel(QWidget *parent = 0);
  ~InOutPanel();

public:
  GmicQt::InputMode inputMode() const;
  GmicQt::OutputMode outputMode() const;
  GmicQt::PreviewMode previewMode() const;
  void save(QSettings &);
  void load(QSettings &);
  void reset();
  GmicQt::OutputMessageMode outputMessageMode() const;
  QString gmicEnvString() const;

signals:
  void inputModeChanged(GmicQt::InputMode);
  void outputMessageModeChanged(GmicQt::OutputMessageMode);
  void previewModeChanged(GmicQt::PreviewMode);

public slots:
  void onInputModeSelected(int);
  void onOutputModeSelected(int);
  void onOutputMessageSelected(int);
  void onPreviewModeSelected(int);

private:
  Ui::InOutPanel *ui;
  static const int NoSelection = -1;
};

#endif // _GMIC_QT_INOUTPANEL_H
