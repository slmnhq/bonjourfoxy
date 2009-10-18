#include "nsIGenericFactory.h"
#include "bfdnssdimpl.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(BFDNSSD)

static nsModuleComponentInfo components[] =
{
    {
       BFDNSSD_CLASSNAME, 
       BFDNSSD_CID,
       BFDNSSD_CONTRACTID,
       BFDNSSDConstructor,
    }
};

NS_IMPL_NSGETMODULE("BFIDNSSDModule", components) 

