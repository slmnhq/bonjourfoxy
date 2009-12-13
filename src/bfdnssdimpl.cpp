#include "bfdnssdimpl.h"

#include "nsComponentManagerUtils.h"  // for do_CreateInstance
#include "nsServiceManagerUtils.h"    // for do_GetService
#include "nsIConsoleService.h"
#include "nsIPrefService.h"
#include <errno.h>                    // For errno, EINTR
#include <stdio.h>                    // for snprintf - applies on win32?

#ifndef _WIN32
#include <sys/time.h>
#else
#define snprintf _snprintf
#endif

#include <errno.h>                    // For errno, EINTR

#define EMPTY_STRING NS_LITERAL_STRING("")

typedef union { unsigned char b[2]; unsigned short NotAnInteger; } Opaque16;

/* Implementation file */
NS_IMPL_ISUPPORTS1(BFDNSSD, BFIDNSSD)

BFDNSSD::BFDNSSD()
{
  /* member initializers and constructor code */
  mStarted = PR_FALSE;
}

BFDNSSD::~BFDNSSD()
{
  /* destructor code */
  if (mStarted) DNSServiceRefDeallocate(mSdRef);
  if (mTimer) mTimer->Cancel();
}

/* attribute long interfaceIndex; */
NS_IMETHODIMP BFDNSSD::GetInterfaceIndex(PRInt32 *aInterfaceIndex)
{
    *aInterfaceIndex = mInterfaceIndex;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetInterfaceIndex(PRInt32 aInterfaceIndex)
{
    if (mStarted == PR_FALSE)
        mInterfaceIndex = aInterfaceIndex;
    return NS_OK;
}

/* attribute PRBool type; */
NS_IMETHODIMP BFDNSSD::GetDomainType(PRBool *aDomainType)
{
    *aDomainType = mDomainType;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetDomainType(PRBool aDomainType)
{
    if (mStarted == PR_FALSE)
        mDomainType = aDomainType;
    return NS_OK;
}

/* attribute PRBool autorename; */
NS_IMETHODIMP BFDNSSD::GetAutorename(PRBool *aAutorename)
{
    *aAutorename = mAutorename;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetAutorename(PRBool aAutorename)
{
    if (mStarted == PR_FALSE)
        mAutorename = aAutorename;
    return NS_OK;
}

/* attribute AString serviceName; */
NS_IMETHODIMP BFDNSSD::GetServiceName(nsAString & aServiceName)
{
    aServiceName = mServiceName.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetServiceName(const nsAString & aServiceName)
{
    if (mStarted == PR_FALSE)
        mServiceName.Assign(aServiceName);
    return NS_OK;
}

/* attribute AString registrationType; */
NS_IMETHODIMP BFDNSSD::GetRegistrationType(nsAString & aRegistrationType)
{
    aRegistrationType = mRegistrationType.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetRegistrationType(const nsAString & aRegistrationType)
{
    if (mStarted == PR_FALSE)
        mRegistrationType.Assign(aRegistrationType);
    return NS_OK;
}

/* attribute AString registrationDomain; */
NS_IMETHODIMP BFDNSSD::GetRegistrationDomain(nsAString & aRegistrationDomain)
{
    aRegistrationDomain = mRegistrationDomain.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetRegistrationDomain(const nsAString & aRegistrationDomain)
{
    if (mStarted == PR_FALSE)
        mRegistrationDomain.Assign(aRegistrationDomain);
    return NS_OK;
}

/* attribute AString targetHost; */
NS_IMETHODIMP BFDNSSD::GetTargetHost(nsAString & aTargetHost)
{
    aTargetHost = mTargetHost.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetTargetHost(const nsAString & aTargetHost)
{
    if (mStarted == PR_FALSE)
        mTargetHost.Assign(aTargetHost);
    return NS_OK;
}

/* attribute long targetPort; */
NS_IMETHODIMP BFDNSSD::GetTargetPort(PRInt32 *aTargetPort)
{
    *aTargetPort = mTargetPort;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetTargetPort(PRInt32 aTargetPort)
{
    if (mStarted == PR_FALSE)
        mTargetPort = aTargetPort;
    return NS_OK;
}

/* attribute AString txtRecordKey; */
NS_IMETHODIMP BFDNSSD::GetTxtRecordKey(nsAString & aTxtRecordKey)
{
    aTxtRecordKey = mTxtRecordKey.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetTxtRecordKey(const nsAString & aTxtRecordKey)
{
    if (mStarted == PR_FALSE)
        mTxtRecordKey.Assign(aTxtRecordKey);
    return NS_OK;
}

/* attribute AString txtRecordValue; */
NS_IMETHODIMP BFDNSSD::GetTxtRecordValue(nsAString & aTxtRecordValue)
{
    aTxtRecordValue = mTxtRecordValue.get();
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetTxtRecordValue(const nsAString & aTxtRecordValue)
{
    if (mStarted == PR_FALSE)
        mTxtRecordValue.Assign(aTxtRecordValue);
    return NS_OK;
}

/* attribute BFIEnumerateCallback enumerateCallback; */
NS_IMETHODIMP BFDNSSD::GetEnumerateCallback(BFIEnumerateCallback * *aEnumerateCallback)
{
    *aEnumerateCallback = mEnumerateCallback;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetEnumerateCallback(BFIEnumerateCallback * aEnumerateCallback)
{
    if (mStarted == PR_FALSE)
        mEnumerateCallback = aEnumerateCallback;
    return NS_OK;
}

/* void enumerate (); */
NS_IMETHODIMP BFDNSSD::Enumerate()
{
    Log(ToNewUnicode(NS_LITERAL_STRING("Component Started Enumerate")));
    DNSServiceErrorType err = kDNSServiceErr_Unknown;
    if (mDomainType == PR_TRUE)
    {
        err = DNSServiceEnumerateDomains(&mSdRef, kDNSServiceFlagsRegistrationDomains, mInterfaceIndex, (DNSServiceDomainEnumReply) EnumerateCallback, this);
    }
    else
    {
        err = DNSServiceEnumerateDomains(&mSdRef, kDNSServiceFlagsBrowseDomains, mInterfaceIndex, (DNSServiceDomainEnumReply) EnumerateCallback, this);
    }    
    if (err != kDNSServiceErr_NoError) return NS_ERROR_FAILURE;
    StartTimer();
    return NS_OK;
}


void DNSSD_API BFDNSSD::EnumerateCallback (
    DNSServiceRef inSdRef,
    DNSServiceFlags inFlags,
    uint32_t inInterfaceIndex,
    DNSServiceErrorType inErrorCode,
    const char * inReplyDomain,
    void * inContext)
{
    BFDNSSD * self;
    self = reinterpret_cast <BFDNSSD*>( inContext );
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Callback to Enumerate")));
    if (self->mEnumerateCallback)
    {
        if (inErrorCode == kDNSServiceErr_NoError)
        {
            PRBool oAdd = PR_FALSE;
            nsString oRegistrationDomain;
            if (inFlags & kDNSServiceFlagsAdd)
            {
                oAdd = PR_TRUE;
            }
            oRegistrationDomain.Assign(NS_ConvertUTF8toUTF16(inReplyDomain));
            self->mEnumerateCallback->Callback(0, oAdd, inInterfaceIndex, self->mDomainType, oRegistrationDomain);
        }
        else
        {
            self->mEnumerateCallback->Callback(99, PR_FALSE, -1, PR_FALSE, EMPTY_STRING);
        }
    }
}


/* attribute BFIBrowseCallback browseCallback; */
NS_IMETHODIMP BFDNSSD::GetBrowseCallback(BFIBrowseCallback * *aBrowseCallback)
{
    *aBrowseCallback = mBrowseCallback;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetBrowseCallback(BFIBrowseCallback * aBrowseCallback)
{
    if (mStarted == PR_FALSE)
        mBrowseCallback = aBrowseCallback;
    return NS_OK;
}

/* void browse (); */
NS_IMETHODIMP BFDNSSD::Browse()
{
    Log(ToNewUnicode(NS_LITERAL_STRING("Component Started Browse")));
    if (!mBrowseCallback) return NS_ERROR_FAILURE;
    if (mStarted) return NS_ERROR_FAILURE;
    mStarted = PR_TRUE;
    DNSServiceErrorType err = kDNSServiceErr_Unknown;
    err = DNSServiceBrowse(&mSdRef, 0, mInterfaceIndex, ToNewUTF8String(mRegistrationType), ToNewUTF8String(mRegistrationDomain), (DNSServiceBrowseReply) BrowseCallback, this);
    Log(ToNewUnicode(mRegistrationType));
    if (err != kDNSServiceErr_NoError) return NS_ERROR_FAILURE;
    StartTimer();
    return NS_OK;
}

void DNSSD_API BFDNSSD::BrowseCallback(
    DNSServiceRef           inSdRef,
    DNSServiceFlags         inFlags,
    uint32_t                inInterfaceIndex,
    DNSServiceErrorType     inErrorCode,
    const char *            inReplyName,
    const char *            inReplyType,
    const char *            inReplyDomain,
    void *                  inContext)
{

    BFDNSSD * self;
    self = reinterpret_cast <BFDNSSD*>( inContext );
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Callback to Browse")));
    if (self->mBrowseCallback)
    {
        if (inErrorCode == kDNSServiceErr_NoError)
        {
            PRBool oAdd = PR_FALSE;
            if (inFlags & kDNSServiceFlagsAdd) oAdd = PR_TRUE;
            nsString oServiceName;
            nsString oRegistrationType;
            nsString oRegistrationDomain;
            oServiceName.Assign(NS_ConvertUTF8toUTF16(inReplyName));
            oRegistrationType.Assign(NS_ConvertUTF8toUTF16(inReplyType));
            oRegistrationDomain.Assign(NS_ConvertUTF8toUTF16(inReplyDomain));
            self->mBrowseCallback->Callback(0, oAdd, inInterfaceIndex, oServiceName, oRegistrationType, oRegistrationDomain);
        }
        else
        {
            if (self->mTimer) self->mTimer->Cancel();
            self->mBrowseCallback->Callback(99, PR_FALSE, -1, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
        }
    }
}

/* attribute BFIResolveCallback resolveCallback; */
NS_IMETHODIMP BFDNSSD::GetResolveCallback(BFIResolveCallback * *aResolveCallback)
{
    *aResolveCallback = mResolveCallback;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetResolveCallback(BFIResolveCallback * aResolveCallback)
{
    if (mStarted == PR_FALSE)
        mResolveCallback = aResolveCallback;
    return NS_OK;
}

/* void resolve (); */
NS_IMETHODIMP BFDNSSD::Resolve()
{
    Log(ToNewUnicode(NS_LITERAL_STRING("Component Started Resolve")));
    if (!mResolveCallback) return NS_ERROR_FAILURE;
    DNSServiceErrorType err = kDNSServiceErr_Unknown;
    err = DNSServiceResolve(&mSdRef, 0, mInterfaceIndex, ToNewUTF8String(mServiceName) , ToNewUTF8String(mRegistrationType), ToNewUTF8String(mRegistrationDomain), (DNSServiceResolveReply) ResolveCallback, this);
    if (err != kDNSServiceErr_NoError) return NS_ERROR_FAILURE;
    StartTimer();
    return NS_OK;
}

void DNSSD_API BFDNSSD::ResolveCallback(
        DNSServiceRef           inSdRef,
        DNSServiceFlags         inFlags,
        uint32_t                inInterfaceIndex,
        DNSServiceErrorType     inErrorCode,
        const char *            inFQDN, 
        const char *            inHostname, 
        uint16_t                inPort,
        uint16_t                inTxtLen,
        const char *            inTxtRecord,
        void *                  inContext )
{
    BFDNSSD * self;
    self = reinterpret_cast <BFDNSSD*>( inContext );
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Callback to Resolve")));
    if (inErrorCode == kDNSServiceErr_NoError)
    {
        if (self->mTimer) self->mTimer->Cancel();
        if (self->mResolveCallback)
        {
            nsString oHostname;
            oHostname.Assign(NS_ConvertUTF8toUTF16(inHostname));
            union { uint16_t s; u_char b[2]; } port = { inPort };
            uint16_t oPort = ((uint16_t)port.b[0]) << 8 | port.b[1];
            nsString oTxtKey;
            nsString oTxtValue;
            char tKey[256];
            int index=0;
            uint8_t valueLen;
            const void *voidValue = 0;            
            // Bonjour API indicates only the 'primary text record' is supplied to the callback
            if (TXTRecordGetItemAtIndex(inTxtLen,
                                        inTxtRecord,
                                        index++,
                                        256,
                                        tKey,
                                        &valueLen,
                                        &voidValue) == kDNSServiceErr_NoError) 
            {
                char *tValue = NULL;
                tValue = new char[valueLen+1];
                snprintf (tValue, valueLen+1, "%.*s\n", (int) valueLen, voidValue);
                tValue[valueLen] = '\0';
                oTxtKey.Assign(NS_ConvertUTF8toUTF16(tKey));
                oTxtValue.Assign(NS_ConvertUTF8toUTF16(tValue));
            }
            self->mResolveCallback->Callback(0, inInterfaceIndex, oHostname, oPort, oTxtKey, oTxtValue);
        }
    }
    else
    {
        if (self->mTimer) self->mTimer->Cancel();
        if (self->mResolveCallback)
        {
            self->mResolveCallback->Callback(99, -1, EMPTY_STRING, -1, EMPTY_STRING, EMPTY_STRING);
        }
    }
}


/* attribute BFIRegisterCallback registerCallback; */
NS_IMETHODIMP BFDNSSD::GetRegisterCallback(BFIRegisterCallback * *aRegisterCallback)
{
    *aRegisterCallback = mRegisterCallback;
    return NS_OK;
}
NS_IMETHODIMP BFDNSSD::SetRegisterCallback(BFIRegisterCallback * aRegisterCallback)
{
    if (mStarted == PR_FALSE)
        mRegisterCallback = aRegisterCallback;
    return NS_OK;
}

/* void register (); */
NS_IMETHODIMP BFDNSSD::Register()
{
    Log(ToNewUnicode(NS_LITERAL_STRING("Component Started Register")));
    if (!mRegisterCallback) return NS_ERROR_FAILURE;
    DNSServiceErrorType err = kDNSServiceErr_Unknown;
    DNSServiceFlags flags = 0;
    if (mAutorename == PR_FALSE)
    {
        flags |= kDNSServiceFlagsNoAutoRename;
    }
    Opaque16 registerPort = { { mTargetPort >> 8, mTargetPort & 0xFF } };
    TXTRecordRef txt;
    TXTRecordCreate(&txt,0,0);
    void* tTxtRecordValue = ToNewUTF8String(mTxtRecordValue);
    err = TXTRecordSetValue(&txt, ToNewUTF8String(mTxtRecordKey), mTxtRecordValue.Length(), tTxtRecordValue); 
    if (err != kDNSServiceErr_NoError) return NS_ERROR_FAILURE;
    err = DNSServiceRegister(&mSdRef,
                             mInterfaceIndex,
                             flags,
                             ToNewUTF8String(mServiceName),
                             ToNewUTF8String(mRegistrationType),
                             ToNewUTF8String(mRegistrationDomain),
                             ToNewUTF8String(mTargetHost),
                             registerPort.NotAnInteger,
                             TXTRecordGetLength(&txt),
                             TXTRecordGetBytesPtr(&txt),
                             (DNSServiceRegisterReply) RegisterCallback,
                             this);
    if (err != kDNSServiceErr_NoError) return NS_ERROR_FAILURE;
    StartTimer();
    return NS_OK;
}

void DNSSD_API BFDNSSD::RegisterCallback(
                DNSServiceRef                   inSdRef,
                DNSServiceFlags                 inFlags,
                DNSServiceErrorType             inErrorCode,
                const char *                    inName, 
                const char *                    inRegType,
                const char *                    inRegDomain,
                void *                          inContext )
{
    BFDNSSD * self;
    self = reinterpret_cast <BFDNSSD*>( inContext );
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Callback to Register")));
    if (self->mRegisterCallback)
    {
        if (inErrorCode == kDNSServiceErr_NoError)
        {
            PRBool oAdd = PR_FALSE;
            if (inFlags & kDNSServiceFlagsAdd) oAdd = PR_TRUE;
            self->mRegisterCallback->Callback(0, self->mInterfaceIndex, oAdd, NS_ConvertUTF8toUTF16(inName), NS_ConvertUTF8toUTF16(inRegType), NS_ConvertUTF8toUTF16(inRegDomain));
        }
        else
        {
            printf("error: %d\n", inErrorCode);
            self->mRegisterCallback->Callback(99, -1, PR_FALSE, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
        }
    }
}


void BFDNSSD::Log(const PRUnichar* message)
{
    printf("%s\n", NS_ConvertUTF16toUTF8(message).get());
    nsCOMPtr<nsIPrefService> prefService = do_GetService("@mozilla.org/preferences-service;1");
    if (prefService)
    {
        nsCOMPtr<nsIPrefBranch> prefs;
        prefService->GetBranch("extensions.bonjourfoxy.log", getter_AddRefs(prefs));
        PRBool doConsoleLog;
        PRBool doSystemLog;
        prefs->GetBoolPref("console", &doConsoleLog);
        prefs->GetBoolPref("system", &doSystemLog);
        if (doConsoleLog == PR_TRUE)
        {
            nsCOMPtr<nsIConsoleService> consoleService = do_GetService("@mozilla.org/consoleservice;1");
            if (consoleService)
            {
                consoleService->LogStringMessage(message);
            }
        }
        if (doSystemLog == PR_TRUE)
        {
            printf("%s\n", NS_ConvertUTF16toUTF8(message).get());
        }
    }
}

void BFDNSSD::StartTimer()
{
    mTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mTimer)
    {
        Log(ToNewUnicode(NS_LITERAL_STRING("Component was unable to get an instance of Timer")));
    }
    else
    {
        Log(ToNewUnicode(NS_LITERAL_STRING("Component StartTimer got a Timer instance")));
        mTimer->InitWithFuncCallback(this->TimeoutHandler,
                                            this,
                                            100, 
                                            nsITimer::TYPE_REPEATING_SLACK);
    }
}

void BFDNSSD::TimeoutHandler(nsITimer *aTimer, void *aClosure)
{
    BFDNSSD *self = reinterpret_cast<BFDNSSD*>(aClosure);
    if (!self) {
        NS_ERROR("no self\n");
        return;
    }
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Timeout Handler Fired")));
    self->PollSelect(self);
}


void BFDNSSD::PollSelect(void *inContext)
{
    BFDNSSD * self;
    self = reinterpret_cast <BFDNSSD*>( inContext );
    self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Polling for Result")));
    struct timeval tv;
    int result; 
    fd_set readfds;

    int dns_sd_fd = DNSServiceRefSockFD(self->mSdRef);  
    int nfds = dns_sd_fd + 1;

    FD_ZERO(&readfds);
    FD_SET(dns_sd_fd, &readfds);
    tv.tv_sec = 0;  // don't block
    tv.tv_usec = 0; // at all

    result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

    if (result >= 0)
    {
        //printf("Results: %d\n", result);
        if (result > 0)
        {
            self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Results Recieved > 0 Results")));
            DNSServiceErrorType err = kDNSServiceErr_NoError;
            if (self->mSdRef && FD_ISSET(dns_sd_fd , &readfds)) err = DNSServiceProcessResult(self->mSdRef);
        }
        else
        {
            self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Results Recieved 0 Results")));
        }
    }
    else
    {         
        //printf("select() returned %d errno %d %s\n", result, errno, strerror(errno));
        if (errno != EINTR)
        {
            self->Log(ToNewUnicode(NS_LITERAL_STRING("Component Recieved an error other than Interrupted System Call (EINTR)")));
            if (mTimer) mTimer->Cancel();
        }
    }
    FD_CLR(dns_sd_fd, &readfds);
}