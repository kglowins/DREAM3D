/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "DREAM3DToolbox.h"

#include "Applications/DREAM3D/DREAM3DApplication.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3DToolbox::DREAM3DToolbox(QWidget* parent) :
QWidget(parent)
{
  setupUi(this);

  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3DToolbox::~DREAM3DToolbox()
{
  writeSettings();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3DToolbox::setupGui()
{  
  // This will set the initial filter list into the toolbox
  filterLibraryWidget->refreshFilterGroups();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3DToolbox::writeSettings()
{
  DREAM3DSettings prefs;

  prefs.beginGroup("DockWidgetSettings");
  prefs.beginGroup("Toolbox");

  prefs.setValue(objectName(), isVisible());

  QByteArray geo_data = saveGeometry();
  prefs.setValue(QString("WindowGeometry"), geo_data);

  prefs.endGroup();
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3DToolbox::readSettings()
{
  DREAM3DSettings prefs;

  prefs.beginGroup("DockWidgetSettings");
  prefs.beginGroup("Toolbox");

  bool b = prefs.value(objectName(), false).toBool();
  dream3dApp->on_actionShowToolbox_triggered(b);

  if (prefs.contains(QString("WindowGeometry")))
  {
    QByteArray geo_data = prefs.value("WindowGeometry", "").toByteArray();
    bool ok = restoreGeometry(geo_data);
    if (!ok)
    {
      qDebug() << "Error Restoring DREAM3D Toolbox's Window Geometry" << "\n";
    }
  }
  else
  {
    move(0, 0);
  }

  prefs.endGroup();
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BookmarksWidget* DREAM3DToolbox::getBookmarksWidget()
{
  return bookmarksWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterListWidget* DREAM3DToolbox::getFilterListWidget()
{
  return filterListWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterLibraryWidget* DREAM3DToolbox::getFilterLibraryWidget()
{
  return filterLibraryWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PrebuiltsWidget* DREAM3DToolbox::getPrebuiltsWidget()
{
  return prebuiltsWidget;
}



