/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef CHERRYWORKBENCHSERVICEREGISTRY_H_
#define CHERRYWORKBENCHSERVICEREGISTRY_H_

#include "cherryServiceLocator.h"

#include "../cherryAbstractSourceProvider.h"
#include "../services/cherryIServiceFactory.h"

#include <cherryIExtensionPoint.h>
#include <cherryIConfigurationElement.h>

#include <vector>

namespace cherry
{

/**
 * This class will create a service from the matching factory. If the factory
 * doesn't exist, it will try and load it from the registry.
 *
 * @since 3.4
 */
class WorkbenchServiceRegistry
{ // implements IExtensionChangeHandler {

private:

  /**
   *
   */
  static const std::string WORKBENCH_LEVEL; // = "workbench"; //$NON-NLS-1$

  static const std::string EXT_ID_SERVICES; // = "org.opencherry.ui.services"; //$NON-NLS-1$

  static WorkbenchServiceRegistry* registry; // = 0;

  WorkbenchServiceRegistry();

  struct ServiceFactoryHandle
  {
    IServiceFactory* factory;
    // TODO used in removeExtension to react to bundles being unloaded
    //WeakHashMap serviceLocators = new WeakHashMap();
    std::vector<std::string> serviceNames;

    ServiceFactoryHandle(IServiceFactory* factory);
  };

  std::map<std::string, ServiceFactoryHandle*> factories;

  ServiceFactoryHandle* LoadFromRegistry(const std::string& key);

  const IExtensionPoint* GetExtensionPoint();

  static const std::string* const supportedLevels();
  static const unsigned int supportedLevelsCount;

  void ProcessVariables(
      const std::vector<IConfigurationElement::Pointer>& children);

  struct GlobalParentLocator: public IServiceLocator
  {
    Object::Pointer GetService(const std::string& api) const;
    bool HasService(const std::string& api) const;
  };

public:

  static WorkbenchServiceRegistry* GetRegistry();

  /**
   * Used as the global service locator's parent.
   */
  static const IServiceLocator* GLOBAL_PARENT; // = new GlobalParentLocator();

  Object::Pointer GetService(const std::string& key,
      const IServiceLocator* parentLocator, const ServiceLocator* locator);

  std::vector<ISourceProvider*> GetSourceProviders();

  //  void addExtension(IExtensionTracker tracker, IExtension extension) {
  //    // we don't need to react to adds because we are not caching the extensions we find -
  //    // next time a service is requested, we will look at all extensions again in
  //    // loadFromRegistry
  //  }

  //  void removeExtension(IExtension extension, Object[] objects) {
  //    for (int i = 0; i < objects.length; i++) {
  //      Object object = objects[i];
  //      if (object instanceof ServiceFactoryHandle) {
  //        ServiceFactoryHandle handle = (ServiceFactoryHandle) object;
  //        Set locatorSet = handle.serviceLocators.keySet();
  //        ServiceLocator[] locators = (ServiceLocator[]) locatorSet.toArray(new ServiceLocator[locatorSet.size()]);
  //        Arrays.sort(locators, new Comparator(){
  //          public int compare(Object o1, Object o2) {
  //            ServiceLocator loc1 = (ServiceLocator) o1;
  //            ServiceLocator loc2 = (ServiceLocator) o2;
  //            int l1 = ((IWorkbenchLocationService) loc1
  //                .getService(IWorkbenchLocationService.class))
  //                .getServiceLevel();
  //            int l2 = ((IWorkbenchLocationService) loc2
  //                .getService(IWorkbenchLocationService.class))
  //                .getServiceLevel();
  //            return l1 < l2 ? -1 : (l1 > l2 ? 1 : 0);
  //          }
  //        });
  //        for (int j = 0; j < locators.length; j++) {
  //          ServiceLocator serviceLocator = locators[j];
  //          if (!serviceLocator.isDisposed()) {
  //            serviceLocator.unregisterServices(handle.serviceNames);
  //          }
  //        }
  //        handle.factory = null;
  //        for (int j = 0; j < handle.serviceNames.length; j++) {
  //          String serviceName = handle.serviceNames[j];
  //          if (factories.get(serviceName) == handle) {
  //            factories.remove(serviceName);
  //          }
  //        }
  //      }
  //    }
  //  }
};

}

#endif /* CHERRYWORKBENCHSERVICEREGISTRY_H_ */
