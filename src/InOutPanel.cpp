/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file InOutPanel.cpp
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
#include <QSettings>
#include "ui_inoutpanel.h"
#include "InOutPanel.h"
#include "gmic.h"
#include <cstdio>

InOutPanel::InOutPanel(QWidget *parent) :
  QGroupBox(parent),
  ui(new Ui::InOutPanel)
{
  ui->setupUi(this);

  ui->inputLayers->setToolTip(tr("Input layers"));
  ui->inputLayers->addItem(tr("Input layers..."),NoSelection);
  ui->inputLayers->addItem(tr("-"),NoSelection);
  ui->inputLayers->addItem(tr("None"),GmicQt::NoInput);
  ui->inputLayers->addItem(tr("Active (default)"),GmicQt::Active);
  ui->inputLayers->addItem(tr("All"),GmicQt::All);
  ui->inputLayers->addItem(tr("Active and below"),GmicQt::ActiveAndBelow);
  ui->inputLayers->addItem(tr("Active and above"),GmicQt::ActiveAndAbove);
  ui->inputLayers->addItem(tr("All visible"),GmicQt::AllVisibles);
  ui->inputLayers->addItem(tr("All invisible"),GmicQt::AllInvisibles);
  ui->inputLayers->addItem(tr("All visible (decr.)"),GmicQt::AllVisiblesDesc);
  ui->inputLayers->addItem(tr("All invisible (decr.)"),GmicQt::AllInvisiblesDesc);
  ui->inputLayers->addItem(tr("All (decr.)"),GmicQt::AllDesc);

  ui->outputMode->setToolTip(tr("Output mode"));
  ui->outputMode->addItem(tr("Output mode..."),NoSelection);
  ui->outputMode->addItem(tr("-"),NoSelection);
  ui->outputMode->addItem(tr("In place (default)"),GmicQt::InPlace);
  ui->outputMode->addItem(tr("New layer(s)"),GmicQt::NewLayers);
  ui->outputMode->addItem(tr("New active layer(s)"),GmicQt::NewActiveLayers);
  ui->outputMode->addItem(tr("New image"),GmicQt::NewImage);

  ui->outputMessages->setToolTip(tr("Output messages"));
  ui->outputMessages->addItem(tr("Output messages..."),NoSelection);
  ui->outputMessages->addItem(tr("-"),NoSelection);
  ui->outputMessages->addItem(tr("Quiet (default)"),GmicQt::Quiet);
  ui->outputMessages->addItem(tr("Verbose (layer name)"),GmicQt::VerboseLayerName);
  ui->outputMessages->addItem(tr("Verbose (console)"),GmicQt::VerboseConsole);
  ui->outputMessages->addItem(tr("Verbose (log file)"),GmicQt::VerboseLogFile);
  ui->outputMessages->addItem(tr("Very verbose (console)"),GmicQt::VeryVerboseConsole);
  ui->outputMessages->addItem(tr("Very verbose (log file)"),GmicQt::VeryVerboseLogFile);
  ui->outputMessages->addItem(tr("Debug (console)"),GmicQt::DebugConsole);
  ui->outputMessages->addItem(tr("Debug (log file)"),GmicQt::DebugLogFile);

  ui->previewMode->setToolTip(tr("Preview mode"));
  ui->previewMode->addItem(tr("Preview mode..."),NoSelection);
  ui->previewMode->addItem(tr("-"),NoSelection);
  ui->previewMode->addItem(tr("1st ouput (default)"),GmicQt::FirstOutput);
  ui->previewMode->addItem(tr("2cd ouput"),GmicQt::SecondOutput);
  ui->previewMode->addItem(tr("3rd ouput"),GmicQt::ThirdOutput);
  ui->previewMode->addItem(tr("4th ouput"),GmicQt::FourthOutput);
  ui->previewMode->addItem(tr("1st -> 2cd ouput"),GmicQt::First2SecondOutput);
  ui->previewMode->addItem(tr("1st -> 3rd ouput"),GmicQt::First2ThirdOutput);
  ui->previewMode->addItem(tr("1st -> 4th ouput"),GmicQt::First2FourthOutput);
  ui->previewMode->addItem(tr("All ouputs"),GmicQt::AllOutputs);

  connect(ui->inputLayers,SIGNAL(currentIndexChanged(int)),
          this,SLOT(onInputModeSelected(int)));
  connect(ui->outputMode,SIGNAL(currentIndexChanged(int)),
          this,SLOT(onOutputModeSelected(int)));
  connect(ui->outputMessages,SIGNAL(currentIndexChanged(int)),
          this,SLOT(onOutputMessageSelected(int)));
  connect(ui->previewMode,SIGNAL(currentIndexChanged(int)),
          this,SLOT(onPreviewModeSelected(int)));
}

InOutPanel::~InOutPanel()
{
  delete ui;
}

GmicQt::InputMode InOutPanel::inputMode() const
{
  int mode = ui->inputLayers->currentData().toInt();
  return (mode==NoSelection) ? GmicQt::Active : static_cast<GmicQt::InputMode>(mode);
}

GmicQt::OutputMode InOutPanel::outputMode() const
{
  int mode = ui->outputMode->currentData().toInt();
  return (mode==NoSelection) ? GmicQt::InPlace : static_cast<GmicQt::OutputMode>(mode);
}

GmicQt::PreviewMode InOutPanel::previewMode() const
{
  int mode = ui->previewMode->currentData().toInt();
  return (mode==NoSelection) ? GmicQt::FirstOutput : static_cast<GmicQt::PreviewMode>(mode);
}

GmicQt::OutputMessageMode
InOutPanel::outputMessageMode() const
{
  int mode = ui->outputMessages->currentData().toInt();
  return (mode==NoSelection) ? GmicQt::Quiet : static_cast<GmicQt::OutputMessageMode>(mode);
}

QString InOutPanel::gmicEnvString() const
{
  return QString("_input_layers=%1 _output_mode=%2 _output_messages=%3 _preview_mode=%4")
      .arg(inputMode())
      .arg(outputMode())
      .arg(outputMessageMode())
      .arg(previewMode());
}

void
InOutPanel::save(QSettings & settings)
{
  settings.setValue("OutputMessageModeIndex",ui->outputMessages->currentIndex());
  settings.setValue("OutputMessageModeValue",ui->outputMessages->currentData().toInt());
  settings.setValue("InputLayers",ui->inputLayers->currentIndex());
  settings.setValue("OutputMode",ui->outputMode->currentIndex());
  settings.setValue("PreviewMode",ui->previewMode->currentIndex());
}

void
InOutPanel::load(QSettings & settings)
{
  ui->outputMessages->setCurrentIndex(settings.value("OutputMessageModeIndex",0).toInt());
  ui->inputLayers->setCurrentIndex(settings.value("InputLayers",0).toInt());
  ui->outputMode->setCurrentIndex(settings.value("OutputMode",0).toInt());
  ui->previewMode->setCurrentIndex(settings.value("PreviewMode",0).toInt());
}

void InOutPanel::reset()
{
  ui->outputMessages->setCurrentIndex(0);
  ui->inputLayers->setCurrentIndex(0);
  ui->outputMode->setCurrentIndex(0);
  ui->previewMode->setCurrentIndex(0);
}

void InOutPanel::onInputModeSelected(int)
{
  int mode = ui->inputLayers->currentData().toInt();
  if ( mode == NoSelection ) {
    ui->inputLayers->setCurrentIndex(3);
  }
  emit inputModeChanged(static_cast<GmicQt::InputMode>(ui->inputLayers->currentData().toInt()));
}

void
InOutPanel::onOutputModeSelected(int)
{
  int mode = ui->outputMode->currentData().toInt();
  if ( mode == NoSelection ) {
    ui->outputMode->setCurrentIndex(2);
  }
}

void InOutPanel::onOutputMessageSelected(int)
{
  int mode = ui->outputMessages->currentData().toInt();
  if ( mode == NoSelection ) {
    ui->outputMessages->setCurrentIndex(2);
  }
  emit outputMessageModeChanged(static_cast<GmicQt::OutputMessageMode>(mode));
}

void InOutPanel::onPreviewModeSelected(int)
{
  int mode = ui->previewMode->currentData().toInt();
  if ( mode == NoSelection ) {
    ui->previewMode->setCurrentIndex(2);
  }
  emit previewModeChanged(static_cast<GmicQt::PreviewMode>(mode));
}
