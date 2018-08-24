/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKSEMANTICRELATIONSCONTEXTMENU_H
#define QMITKSEMANTICRELATIONSCONTEXTMENU_H

// semantic relations plugin
#include "QmitkDataNodeControlPointAction.h"
#include "QmitkDataNodeInformationTypeAction.h"
#include "QmitkDataNodeRemoveFromSemanticRelationsAction.h"
#include "QmitkDataNodeOpenInAction.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// mitk render window manager module
#include <mitkRenderWindowLayerUtilities.h>

// blueberry ui qt plugin
#include <berryIWorkbenchPartSite.h>

//qt
#include <QMenu>

class QmitkSemanticRelationsContextMenu : public QMenu
{
  Q_OBJECT

public:

  QmitkSemanticRelationsContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent = nullptr);
  virtual ~QmitkSemanticRelationsContextMenu() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer);

public Q_SLOTS:

  void OnContextMenuRequested(const QPoint&);

private:

  void InitDefaultActions();

  QWidget* m_Parent;
  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;

  QmitkDataNodeControlPointAction* m_ControlPointAction;
  QmitkDataNodeInformationTypeAction* m_InformationTypeAction;
  QmitkDataNodeRemoveFromSemanticRelationsAction* m_RemoveFromSemanticRelationsAction;
  QmitkDataNodeOpenInAction* m_OpenInAction;

};

#endif // QMITKSEMANTICRELATIONSCONTEXTMENU_H
