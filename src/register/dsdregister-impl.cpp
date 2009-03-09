#include "dsdregister-impl.h"

//#include "nsIServiceManager.h"

#include "nsComponentManagerUtils.h"

// #include "nsStringAPI.h"

// #include "dns_sd.h"

#include <stdio.h>

#include "dsdmanager.h"

#include "nsIArray.h"
#include "nsVoidArray.h"
#include "nsIMutableArray.h"
#include "nsIVariant.h"
// #include "nsISupportsPrimitives.h"

#include <string>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <errno.h>			// For errno, EINTR

typedef union { unsigned char b[2]; unsigned short NotAnInteger; } Opaque16;


NS_IMPL_ISUPPORTS1(CDSDREGISTER, IDSDREGISTER)

CDSDREGISTER::CDSDREGISTER()
{
  /* member initializers and constructor code */
  mSdRef = NULL;
  mInterfaceIndex = NULL;
  mAutorename = PR_TRUE;
  mName = NS_LITERAL_STRING("");
  mRegistrationType = NS_LITERAL_STRING("");
  mTargetHost = NS_LITERAL_STRING("");
  mTargetPort = NULL;
  mTxtRecordKey = NS_LITERAL_STRING("");
  mTxtRecordValue = NS_LITERAL_STRING("");
  mLastFlags = NULL;
  mLastErrorcode = NULL;
  mLastName = NS_LITERAL_STRING("");
  mLastRegType = NS_LITERAL_STRING("");
  mLastDomain = NS_LITERAL_STRING("");
  mInstanceId = NS_LITERAL_STRING("");
}

CDSDREGISTER::~CDSDREGISTER()
{
  /* destructor code */
  mStatus = 99;
  if (mTimer)
    mTimer->Cancel();
  if (mSdRef)
    DNSServiceRefDeallocate(mSdRef); 
}

/* void start (); */
NS_IMETHODIMP CDSDREGISTER::Start()
{
    DNSServiceErrorType err = kDNSServiceErr_Unknown;
    if (mStatus == 99) {
        return NS_ERROR_FAILURE;
    }
	DNSServiceFlags flags = 0;
	if (!mAutorename)	{
		flags = kDNSServiceFlagsNoAutoRename;
	}
	
	Opaque16 registerPort = { { mTargetPort >> 8, mTargetPort & 0xFF } };
	TXTRecordRef txt;
	TXTRecordCreate(&txt,0,0);	
	void* tTxtRecordValue = ToNewUTF8String(mTxtRecordValue);
	err = TXTRecordSetValue(&txt, ToNewUTF8String(mTxtRecordKey), mTxtRecordValue.Length(), tTxtRecordValue); 
	err = DNSServiceRegister(&mSdRef, mInterfaceIndex, flags, ToNewUTF8String(mName), ToNewUTF8String(mRegistrationType), ToNewUTF8String(mRegistrationDomain), ToNewUTF8String(mTargetHost), registerPort.NotAnInteger, TXTRecordGetLength(&txt),TXTRecordGetBytesPtr(&txt), (DNSServiceRegisterReply) Callback, this);	
	if (err != kDNSServiceErr_NoError) {
        mLastErrorcode = err;
        mStatus = 99;
		//printf("%d\n", err);
        return NS_ERROR_FAILURE;
	}
	mStatus = 1;
	StartTimer();
    return NS_OK;
}

/* void stop (); */
NS_IMETHODIMP CDSDREGISTER::Stop()
{
    mStatus = 0;
    if (mTimer)
      mTimer->Cancel();
/*    if (mSdRef)
      DNSServiceRefDeallocate(mSdRef);  */
    return NS_OK;
}

/* attribute long interfaceIndex; */
NS_IMETHODIMP CDSDREGISTER::GetInterfaceIndex(PRInt32 *aInterfaceIndex)
{
	*aInterfaceIndex = mInterfaceIndex;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetInterfaceIndex(PRInt32 aInterfaceIndex)
{
	mInterfaceIndex = aInterfaceIndex;
    return NS_OK;
}

/* attribute PRBool autorename; */
NS_IMETHODIMP CDSDREGISTER::GetAutorename(PRBool *aAutorename)
{
	*aAutorename = mAutorename;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetAutorename(PRBool aAutorename)
{
	mAutorename = aAutorename;
    return NS_OK;
}

/* attribute AString instanceId; */
NS_IMETHODIMP CDSDREGISTER::GetInstanceId(nsAString & aInstanceId)
{
	aInstanceId = mInstanceId;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetInstanceId(const nsAString & aInstanceId)
{
	mInstanceId = aInstanceId;
    return NS_OK;
}

/* attribute AString name; */
NS_IMETHODIMP CDSDREGISTER::GetName(nsAString & aName)
{
	aName = mName;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetName(const nsAString & aName)
{
	mName = aName;
    return NS_OK;
}

/* attribute AString registrationType; */
NS_IMETHODIMP CDSDREGISTER::GetRegistrationType(nsAString & aRegistrationType)
{
	aRegistrationType = mRegistrationType;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetRegistrationType(const nsAString & aRegistrationType)
{
	mRegistrationType = aRegistrationType;
    return NS_OK;
}

/* attribute AString registrationDomain; */
NS_IMETHODIMP CDSDREGISTER::GetRegistrationDomain(nsAString & aRegistrationDomain)
{
	aRegistrationDomain = mRegistrationDomain;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetRegistrationDomain(const nsAString & aRegistrationDomain)
{
	mRegistrationDomain = aRegistrationDomain;
    return NS_OK;
}


/* attribute AString targetHost; */
NS_IMETHODIMP CDSDREGISTER::GetTargetHost(nsAString & aTargetHost)
{
	aTargetHost = mTargetHost;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetTargetHost(const nsAString & aTargetHost)
{
	mTargetHost = aTargetHost;
    return NS_OK;
}

/* attribute long port; */
NS_IMETHODIMP CDSDREGISTER::GetTargetPort(PRInt32 *aTargetPort)
{
	*aTargetPort = mTargetPort;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetTargetPort(PRInt32 aTargetPort)
{
	mTargetPort = aTargetPort;
    return NS_OK;
}

/* attribute AString txtRecordKey; */
NS_IMETHODIMP CDSDREGISTER::GetTxtRecordKey(nsAString & aTxtRecordKey)
{
	aTxtRecordKey = mTxtRecordKey;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetTxtRecordKey(const nsAString & aTxtRecordKey)
{
	mTxtRecordKey = aTxtRecordKey;
    return NS_OK;
}

/* attribute AString txtRecordValue; */
NS_IMETHODIMP CDSDREGISTER::GetTxtRecordValue(nsAString & aTxtRecordValue)
{
	aTxtRecordValue = mTxtRecordValue;
    return NS_OK;
}
NS_IMETHODIMP CDSDREGISTER::SetTxtRecordValue(const nsAString & aTxtRecordValue)
{
	mTxtRecordValue = aTxtRecordValue;
    return NS_OK;
}

/* readonly attribute long lastFlags; */
NS_IMETHODIMP CDSDREGISTER::GetLastFlags(PRInt32 *aLastFlags)
{
	*aLastFlags = mLastFlags;
    return NS_OK;
}

/* readonly attribute long lastErrorcode; */
NS_IMETHODIMP CDSDREGISTER::GetLastErrorcode(PRInt32 *aLastErrorcode)
{
	*aLastErrorcode = mLastErrorcode;
    return NS_OK;
}

/* readonly attribute AString lastName; */
NS_IMETHODIMP CDSDREGISTER::GetLastName(nsAString & aLastName)
{
	aLastName = mLastName;
    return NS_OK;
}

/* readonly attribute AString lastRegType; */
NS_IMETHODIMP CDSDREGISTER::GetLastRegType(nsAString & aLastRegType)
{
	aLastRegType = mLastRegType;
    return NS_OK;
}

/* readonly attribute AString lastDomain; */
NS_IMETHODIMP CDSDREGISTER::GetLastDomain(nsAString & aLastDomain)
{
	aLastDomain = mLastDomain;
    return NS_OK;
}

/* readonly attribute long status; */
NS_IMETHODIMP CDSDREGISTER::GetStatus(PRInt32 *aStatus)
{
	*aStatus = mStatus;
    return NS_OK;
}

void DNSSD_API CDSDREGISTER::Callback(
		DNSServiceRef			inSdRef,
		DNSServiceFlags			inFlags,
		DNSServiceErrorType		inErrorCode,
		const char *			inName,	
		const char *			inRegType,
		const char *			inRegDomain,
		void *					inContext )
{
    //printf("CDSDREGISTER::Callback()\n");
	CDSDREGISTER * self;
	self = reinterpret_cast <CDSDREGISTER*>( inContext );
	nsCOMPtr<nsIServiceManager> servMan;
    nsresult rv = NS_GetServiceManager(getter_AddRefs(servMan)); 
    if (NS_FAILED(rv)) {
        self->mStatus = 99;
    } else {
        nsCOMPtr<IDSDMANAGER> dsdmanager;
        rv = servMan->GetServiceByContractID("@andrew.tj.id.au/dsdmanager;1", NS_GET_IID(IDSDMANAGER), getter_AddRefs(dsdmanager));

        nsCOMPtr<nsIMutableArray> array = do_CreateInstance(NS_ARRAY_CONTRACTID);

        self->mLastErrorcode = inErrorCode;
        
        nsCOMPtr<nsIWritableVariant> oInstanceId = do_CreateInstance("@mozilla.org/variant;1");        
        oInstanceId->SetAsAUTF8String(NS_ConvertUTF16toUTF8(self->mInstanceId));
        array->AppendElement(oInstanceId, PR_FALSE);
        
        if (inErrorCode == kDNSServiceErr_NoError)
        {
            nsCOMPtr<nsIWritableVariant> oFlags = do_CreateInstance("@mozilla.org/variant;1");
    		if (inFlags & kDNSServiceFlagsAdd)    {
                oFlags->SetAsAUTF8String(NS_ConvertUTF16toUTF8(NS_LITERAL_STRING("add")));
    		} else {
                oFlags->SetAsAUTF8String(NS_ConvertUTF16toUTF8(NS_LITERAL_STRING("rmv")));
    		}
            array->AppendElement(oFlags, PR_FALSE);
            self->mLastFlags = inFlags;
        
            nsCOMPtr<nsIWritableVariant> oName = do_CreateInstance("@mozilla.org/variant;1");        
            oName->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inName)) );
            array->AppendElement(oName, PR_FALSE);
            self->mLastName = NS_ConvertUTF8toUTF16(inName);  
    
            nsCOMPtr<nsIWritableVariant> oRegType = do_CreateInstance("@mozilla.org/variant;1");        
            oRegType->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inRegType)) );
            array->AppendElement(oRegType, PR_FALSE);
            self->mLastName = NS_ConvertUTF8toUTF16(inRegType);  
    
            nsCOMPtr<nsIWritableVariant> oRegDomain = do_CreateInstance("@mozilla.org/variant;1");        
            oRegDomain->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inRegDomain)) );
            array->AppendElement(oRegDomain, PR_FALSE);
            self->mLastDomain = NS_ConvertUTF8toUTF16(inRegDomain);  

        	dsdmanager->HandleEvent(NS_LITERAL_STRING("register"),PR_FALSE,array);
        } 
        else if (inErrorCode == kDNSServiceErr_NameConflict) 
        {
            nsCOMPtr<nsIWritableVariant> oFlags = do_CreateInstance("@mozilla.org/variant;1");
            oFlags->SetAsAUTF8String(NS_ConvertUTF16toUTF8(NS_LITERAL_STRING("conflict")));

            array->AppendElement(oFlags, PR_FALSE);
            self->mLastFlags = inFlags;
        
            nsCOMPtr<nsIWritableVariant> oName = do_CreateInstance("@mozilla.org/variant;1");        
            oName->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inName)) );
            array->AppendElement(oName, PR_FALSE);
            self->mLastName = NS_ConvertUTF8toUTF16(inName);  
    
            nsCOMPtr<nsIWritableVariant> oRegType = do_CreateInstance("@mozilla.org/variant;1");        
            oRegType->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inRegType)) );
            array->AppendElement(oRegType, PR_FALSE);
            self->mLastName = NS_ConvertUTF8toUTF16(inRegType);  
    
            nsCOMPtr<nsIWritableVariant> oRegDomain = do_CreateInstance("@mozilla.org/variant;1");        
            oRegDomain->SetAsAUTF8String( NS_ConvertUTF16toUTF8(NS_ConvertUTF8toUTF16(inRegDomain)) );
            array->AppendElement(oRegDomain, PR_FALSE);
            self->mLastDomain = NS_ConvertUTF8toUTF16(inRegDomain);  

        	dsdmanager->HandleEvent(NS_LITERAL_STRING("register"),PR_FALSE,array);                
        } else {

            nsCOMPtr<nsIWritableVariant> oErrorCode = do_CreateInstance("@mozilla.org/variant;1");
            oErrorCode->SetAsInt32(inErrorCode);
            array->AppendElement(oErrorCode, PR_FALSE);        
            self->mLastErrorcode = inErrorCode;
            
            self->mStatus = 99;
            if (self->mTimer)
                self->mTimer->Cancel();
            // DNSServiceRefDeallocate(self->mSdRef);
        	dsdmanager->HandleEvent(NS_LITERAL_STRING("register"),PR_TRUE,array);
        }
    }
}

void CDSDREGISTER::PollSelect(void *inContext)
{
    //printf("CDSDREGISTER::PollSelect()\n");
	CDSDREGISTER * self;
	self = reinterpret_cast <CDSDREGISTER*>( inContext );
	struct timeval tv;
	int result;	
	fd_set readfds;

	int dns_sd_fd = DNSServiceRefSockFD(self->mSdRef);	
	int nfds = dns_sd_fd + 1;

	FD_ZERO(&readfds);
	FD_SET(dns_sd_fd, &readfds);
	tv.tv_sec = 0;
	tv.tv_usec = 1;

	result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

	if (result > 0)
		{
		//printf("Results: %d\n", result);
		DNSServiceErrorType err = kDNSServiceErr_NoError;
		if (self->mSdRef && FD_ISSET(dns_sd_fd , &readfds)) {
			err = DNSServiceProcessResult(self->mSdRef);
			}
        /*
		if (err) {
			//printf("DNSServiceProcessResult returned %d\n", err);
			}
		*/
		}
	else if (result == 0)
	    {
		//printf("No Results\n");
	    }
	else
		{		  
		//printf("select() returned %d errno %d %s\n", result, errno, strerror(errno));
		if (errno != EINTR) self->mStatus = 99;
		}
    if (self->mStatus != 1)    {
        DNSServiceRefDeallocate(self->mSdRef);
		if (mTimer)
			mTimer->Cancel();
    }
}    

nsresult CDSDREGISTER::StartTimer()
{
    //printf("CDSDREGISTER::StartTimer()\n");
    mTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mTimer)
        return NS_ERROR_FAILURE;
    return mTimer->InitWithFuncCallback(this->TimeoutHandler, this, 100, 
                                        nsITimer::TYPE_REPEATING_SLACK);
}


void CDSDREGISTER::TimeoutHandler(nsITimer *aTimer, void *aClosure)
{
    //printf("CDSDREGISTER::TimeoutHandler()\n");
    CDSDREGISTER *self = reinterpret_cast<CDSDREGISTER*>(aClosure);
    if (!self) {
        NS_ERROR("no self\n");
        return;
    }
    self->PollSelect(self);
}