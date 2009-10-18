#ifndef __BFDNSSD_IMPL_H__
#define __BFDNSSD_IMPL_H__

#include "bfdnssd.h"

#include "dns_sd.h"

#include "nsITimer.h"
#include "nsStringAPI.h"
#include "nsCOMPtr.h"

#define BFDNSSD_CONTRACTID "@bonjourfoxy.net/bfdnssd;1"
#define BFDNSSD_CLASSNAME "BFDNSSD"
#define BFDNSSD_CID \
  {0xe43a0e26, 0x42c2, 0x4a0f, \
    { 0x8a, 0x57, 0x53, 0x84, 0x24, 0xcc, 0x7e, 0x49 }}

class BFDNSSD : public BFIDNSSD
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_BFIDNSSD

  BFDNSSD();

private:
  ~BFDNSSD();
  void PollSelect(void *inContext);
  nsresult StartTimer();
  void Log(const PRUnichar* message);
  nsCOMPtr<nsITimer>                mTimer;
  static void TimeoutHandler(nsITimer *aTimer, void *aClosure);
  /* Define callback functions initiated by DNSServiceProcessResult */
  static void DNSSD_API EnumerateCallback (
    DNSServiceRef                   inSdRef,
    DNSServiceFlags                 inFlags,
    uint32_t                        inInterfaceIndex,
    DNSServiceErrorType             inErrorCode,
    const char *                    inReplyDomain,
    void *                          inContext);
  static void DNSSD_API BrowseCallback (
    DNSServiceRef                   inSdRef,
    DNSServiceFlags                 inFlags,
    uint32_t                        inInterfaceIndex,
    DNSServiceErrorType             inErrorCode,
    const char *                    inReplyName,
    const char *                    inReplyType,
    const char *                    inReplyDomain,
    void *                          inContext);
  static void DNSSD_API ResolveCallback(
    DNSServiceRef                   inSdRef,
    DNSServiceFlags                 inFlags,
    uint32_t                        inInterfaceIndex,
    DNSServiceErrorType             inErrorCode,
    const char *                    inFQDN, 
    const char *                    inHostname, 
    uint16_t                        inPort,
    uint16_t                        inTxtLen,
    const char *                    inTxtRecord,
    void *                          inContext);
  static void DNSSD_API RegisterCallback(
    DNSServiceRef                   inSdRef,
    DNSServiceFlags                 inFlags,
    DNSServiceErrorType             inErrorCode,
    const char *                    inName,
    const char *                    inRegType,
    const char *                    inRegDomain,
    void *                          inContext );

protected:
  PRBool                            mStarted;
  /* used by all */
  DNSServiceRef                     mSdRef;
  long                              mInterfaceIndex;
  
  /* only used with enumerate */
  long                              mDomainType;

  /* used by register/resolve/browse */
  nsString                          mRegistrationType;
  nsString                          mRegistrationDomain;

  /* used by register/resolve */
  nsString                          mServiceName;
  
  /* purely used with registration */
  PRBool                            mAutorename;
  nsString                          mTargetHost;
  long                              mTargetPort;
  nsString                          mTxtRecordKey;
  nsString                          mTxtRecordValue;
  
  /* callbacks */
  nsCOMPtr<BFIEnumerateCallback>    mEnumerateCallback;
  nsCOMPtr<BFIBrowseCallback>       mBrowseCallback;
  nsCOMPtr<BFIResolveCallback>      mResolveCallback;
  nsCOMPtr<BFIRegisterCallback>     mRegisterCallback;
};

#endif