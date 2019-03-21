#include "mitkCppRestSdkQtActivator.h"
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usModuleSettings.h>
#include <QCoreApplication>

void MitkCppRestSdkQtActivator::Load(us::ModuleContext *context)
{
  // Registration of the RESTManagerMicroservice
  m_RESTManagerQt.reset(new mitk::RESTManagerQt);
 
  us::ServiceProperties props;
  if (QCoreApplication::instance!=nullptr)
  {
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;
  }
  else
  {
    props[us::ServiceConstants::SERVICE_RANKING()] = 0;
  }
  context->RegisterService<mitk::IRESTManager>(m_RESTManagerQt.get(),props);
}

void MitkCppRestSdkQtActivator::Unload(us::ModuleContext *) {}

US_EXPORT_MODULE_ACTIVATOR(MitkCppRestSdkQtActivator)
