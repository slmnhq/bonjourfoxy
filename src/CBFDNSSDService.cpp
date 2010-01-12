/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2009 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *	   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CBFDNSSDService.h"
#include "nsThreadUtils.h"
#include "nsIEventTarget.h"
#include "private/pprio.h"
#include <string>
#include <stdio.h>

/* To support XULRunner 1.9.0.x */
#ifndef NS_OUTPARAM
#define NS_OUTPARAM
#endif

NS_IMPL_ISUPPORTS2(CBFDNSSDService, IBFDNSSDService, nsIRunnable)

CBFDNSSDService::CBFDNSSDService()
:
	m_threadPool( NULL ),
	m_sdRef( NULL ),
	m_listener( NULL ),
	m_fileDesc( NULL ),
	m_job( NULL ),
	m_stopped( PR_FALSE )
{
}


CBFDNSSDService::CBFDNSSDService( nsISupports * listener )
:
	m_threadPool( NULL ),
	m_sdRef( NULL ),
	m_listener( listener ),
	m_fileDesc( NULL ),
	m_job( NULL ),
	m_stopped ( PR_FALSE )
{
}


CBFDNSSDService::~CBFDNSSDService()
{
	Cleanup();
}


void
CBFDNSSDService::Cleanup()
{
	m_stopped = PR_TRUE;
	if ( m_job )
	{
		PR_CancelJob( m_job );
		m_job = NULL;
	}
	
	if ( m_threadPool != NULL )
	{
		PR_ShutdownThreadPool( m_threadPool );
		m_threadPool = NULL;
	}
	
	if ( m_fileDesc != NULL )
	{
		PR_Close( m_fileDesc );
		m_fileDesc = NULL;
	}
	
	if ( m_sdRef )
	{
		DNSServiceRefDeallocate( m_sdRef );
		m_sdRef = NULL;
	}
}


nsresult
CBFDNSSDService::SetupNotifications()
{
	if ( m_stopped ) return NS_OK;
	m_iod.socket	= m_fileDesc;
	m_iod.timeout	= PR_INTERVAL_NO_TIMEOUT;
	m_job			= PR_QueueJob_Read( m_threadPool, &m_iod, Read, this, PR_FALSE );
	return ( m_job ) ? NS_OK : NS_ERROR_FAILURE;
}

/* IBFDNSSDService enumerate (in long interfaceIndex, in PRBool domainType, in IBFDNSSDEnumerateListener listener); */
NS_IMETHODIMP
CBFDNSSDService::Enumerate(PRInt32 interfaceIndex, PRBool domainType, IBFDNSSDEnumerateListener *listener, IBFDNSSDService **_retval)
{
	CBFDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;
	
	try
	{
		service = new CBFDNSSDService( listener );
	}
	catch ( ... )
	{
		service = NULL;
	}
	
	if ( service == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	service->m_enuDomainType = domainType;
	dnsErr = DNSServiceEnumerateDomains( &service->m_sdRef, domainType ? kDNSServiceFlagsBrowseDomains : kDNSServiceFlagsRegistrationDomains, interfaceIndex, ( DNSServiceDomainEnumReply ) EnumerateReply, service );
	if ( dnsErr != kDNSServiceErr_NoError )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( service->m_sdRef ) ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	err = service->SetupNotifications();
	if ( err != NS_OK)
	{
	    goto exit;
	}
	listener->AddRef();
	service->AddRef();
	*_retval = service;
	err = NS_OK;

exit:

	if ( err && service )
	{
		delete service;
		service = NULL;
	}
	
	return err;
}

/* IBFDNSSDService browse (in long interfaceIndex, in AString regtype, in AString domain, in IBFDNSSDBrowseListener listener); */
NS_IMETHODIMP
CBFDNSSDService::Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IBFDNSSDBrowseListener *listener, IBFDNSSDService **_retval NS_OUTPARAM)
{
	CBFDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;
	
	try
	{
		service = new CBFDNSSDService( listener );
	}
	catch ( ... )
	{
		service = NULL;
	}
		
	if ( service == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	service->m_regType.Assign( regtype );
	dnsErr = DNSServiceBrowse( &service->m_sdRef, 0, interfaceIndex, NS_ConvertUTF16toUTF8( regtype ).get(), NS_ConvertUTF16toUTF8( domain ).get(), ( DNSServiceBrowseReply ) BrowseReply, service);
	if ( dnsErr != kDNSServiceErr_NoError )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( service->m_sdRef ) ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	err = service->SetupNotifications();
	if ( err != NS_OK)
	{
	    goto exit;
	}
	listener->AddRef();
	service->AddRef();
	*_retval = service;
	err = NS_OK;

exit:

	if ( err && service )
	{
		delete service;
		service = NULL;
	}
	
	return err;
}


/* IBFDNSSDService resolve (in long interfaceIndex, in AString name, in AString regtype, in AString domain, in IBFDNSSDResolveListener listener); */
NS_IMETHODIMP
CBFDNSSDService::Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, const nsAString & key, IBFDNSSDResolveListener *listener, IBFDNSSDService **_retval NS_OUTPARAM)
{
	CBFDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;

	try
	{
		service = new CBFDNSSDService( listener );
	}
	catch ( ... )
	{
		service = NULL;
	}
	
	if ( service == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	service->m_svcTxtKey.Assign( key );
	dnsErr = DNSServiceResolve( &service->m_sdRef, 0, interfaceIndex, NS_ConvertUTF16toUTF8( name ).get(), NS_ConvertUTF16toUTF8( regtype ).get(), NS_ConvertUTF16toUTF8( domain ).get(), ( DNSServiceResolveReply ) ResolveReply, service);
	if ( dnsErr != kDNSServiceErr_NoError )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( service->m_sdRef ) ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
		
	err = service->SetupNotifications();
	if ( err != NS_OK)
	{
	    goto exit;
	}
	listener->AddRef();
	service->AddRef();
	*_retval = service;
	err = NS_OK;

exit:

	if ( err && service )
	{
		delete service;
		service = NULL;
	}
	return err;
}

/* IBFDNSSDService register (in long interfaceIndex, in AString name, in AString regtype, in AString domain, in AString key, in AString value, in IBFDNSSDRegisterListener listener); */
NS_IMETHODIMP
CBFDNSSDService::Register(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, const nsAString & targetHost, PRInt16 targetPort, const nsAString & txtKey, const nsAString & txtValue, IBFDNSSDRegisterListener *listener, IBFDNSSDService **_retval NS_OUTPARAM)
{
	CBFDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;
	TXTRecordRef txt;
	TXTRecordCreate(&txt,0,0);
	void* txtRecordValue = ToNewUTF8String( txtValue );

	try
	{
		service = new CBFDNSSDService( listener );
	}
	catch ( ... )
	{
		service = NULL;
	}
		
	if ( service == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	dnsErr = TXTRecordSetValue(&txt, NS_ConvertUTF16toUTF8( txtKey ).get(), txtValue.Length(), txtRecordValue);
	if ( dnsErr != kDNSServiceErr_NoError )
	{
	    err = NS_ERROR_FAILURE;
	    goto exit;
	}
	dnsErr = DNSServiceRegister( &service->m_sdRef, interfaceIndex, 0, NS_ConvertUTF16toUTF8( name ).get(), NS_ConvertUTF16toUTF8( regtype ).get(), NS_ConvertUTF16toUTF8( domain ).get(), NS_ConvertUTF16toUTF8( targetHost ).get(), htons(targetPort), TXTRecordGetLength(&txt), TXTRecordGetBytesPtr(&txt), ( DNSServiceRegisterReply ) RegisterReply, service);
	if ( dnsErr != kDNSServiceErr_NoError )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( service->m_sdRef ) ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
	if ( ( service->m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
	{
		err = NS_ERROR_FAILURE;
		goto exit;
	}
		
	err = service->SetupNotifications();
	if ( err != NS_OK)
	{
		goto exit;
	}
	listener->AddRef();
	service->AddRef();
	*_retval = service;
	err = NS_OK;

exit:

	if ( err && service )
	{
		delete service;
		service = NULL;
	}
	return err;
}

/* void stop (); */
NS_IMETHODIMP
CBFDNSSDService::Stop()
{
	m_stopped = PR_TRUE;
	if ( m_job != NULL )
	{
		PR_CancelJob( m_job );
		m_job = NULL;
	}
	if ( m_fileDesc != NULL )
	{
		PR_Close( m_fileDesc );
		m_fileDesc = NULL;
	}
	
	if ( m_sdRef )
	{
		DNSServiceRefDeallocate( m_sdRef );
		m_sdRef = NULL;
	}
	return NS_OK;
}


void
CBFDNSSDService::Read( void * arg )
{
	NS_PRECONDITION( arg != NULL, "arg is NULL" );
	NS_DispatchToMainThread( ( CBFDNSSDService* ) arg );
}


NS_IMETHODIMP
CBFDNSSDService::Run()
{
	nsresult err;
	
	NS_PRECONDITION( m_sdRef != NULL, "m_sdRef is NULL" );

	m_job = NULL;
	err = NS_OK;

	if (PR_Available( m_fileDesc ) > 0 && m_sdRef != NULL)
	{
		if ( DNSServiceProcessResult( m_sdRef ) == kDNSServiceErr_NoError )
		{
			err = SetupNotifications();
		}
		else
		{
			err = NS_ERROR_FAILURE;
		}
	}
	return err;
}


void DNSSD_API
CBFDNSSDService::EnumerateReply
		(
		DNSServiceRef		sdRef,
		DNSServiceFlags		flags,
		uint32_t			interfaceIndex,
		DNSServiceErrorType	errorCode,
		const char		*	domain,
		void			*	context
		)
{
	CBFDNSSDService * self = ( CBFDNSSDService* ) context;
	
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IBFDNSSDEnumerateListener * listener = ( IBFDNSSDEnumerateListener* ) self->m_listener;

		// Same for this

		if ( listener != NULL )
		{
			listener->OnEnumerate( self, ( flags & kDNSServiceFlagsAdd ) ? PR_TRUE : PR_FALSE, interfaceIndex, errorCode, self->m_enuDomainType, NS_ConvertUTF8toUTF16( domain ) );
		}
	}
}

void DNSSD_API
CBFDNSSDService::BrowseReply
		(
		DNSServiceRef		sdRef,
		DNSServiceFlags		flags,
		uint32_t			interfaceIndex,
		DNSServiceErrorType errorCode,
		const char		*	serviceName,
		const char		*	regtype,
		const char		*	replyDomain,
		void			*	context
		)
{
	CBFDNSSDService * self = ( CBFDNSSDService* ) context;
	
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IBFDNSSDBrowseListener * listener = ( IBFDNSSDBrowseListener* ) self->m_listener;

		// Same for this

		if ( listener != NULL )
		{
			listener->OnBrowse( self, ( flags & kDNSServiceFlagsAdd ) ? PR_TRUE : PR_FALSE, interfaceIndex, errorCode, NS_ConvertUTF8toUTF16( serviceName ), self->m_regType.Equals( NS_LITERAL_STRING("_services._dns-sd._udp") ) ? NS_ConvertUTF8toUTF16 ( regtype ) : self->m_regType, NS_ConvertUTF8toUTF16( replyDomain ) );
		}
	}
}


void DNSSD_API
CBFDNSSDService::ResolveReply
		(
		DNSServiceRef			sdRef,
		DNSServiceFlags			flags,
		uint32_t				interfaceIndex,
		DNSServiceErrorType		errorCode,
		const char			*	fullname,
		const char			*	hosttarget,
		uint16_t				port,
		uint16_t				txtLen,
		const unsigned char *	txtRecord,
		void				*	context
		)
{
	CBFDNSSDService * self = ( CBFDNSSDService* ) context;
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IBFDNSSDResolveListener * listener = ( IBFDNSSDResolveListener* ) self->m_listener;

		// Same for this

		if ( listener != NULL )
		{
			std::string		value = "";
			char txtKey[256];
			const void	*	txtValue = NULL;
			uint8_t			txtValueLen = 0;
			
			if (self->m_svcTxtKey == NS_LITERAL_STRING(""))
			{
				TXTRecordGetItemAtIndex( txtLen, txtRecord, 1, 256, txtKey, &txtValueLen, &txtValue);
				self->m_svcTxtKey.Assign( NS_ConvertUTF8toUTF16( txtKey ) );
			}
			else 
			{
				txtValue = TXTRecordGetValuePtr( txtLen, txtRecord, NS_ConvertUTF16toUTF8 ( self->m_svcTxtKey ).get(), &txtValueLen );
			}
			if ( txtValue && txtValueLen )
			{
				char * temp;
				
				temp = new char[ txtValueLen + 1 ];
				
				if ( temp )
				{
					memset( temp, 0, txtValueLen + 1 );
					memcpy( temp, txtValue, txtValueLen );
					value = temp;
					delete [] temp;
				}
			}
			listener->OnResolve( self, interfaceIndex, errorCode, NS_ConvertUTF8toUTF16( fullname ), NS_ConvertUTF8toUTF16( hosttarget ) , ntohs( port ), self->m_svcTxtKey, NS_ConvertUTF8toUTF16( value.c_str() ) );
		}
	}
}

void DNSSD_API
CBFDNSSDService::RegisterReply
		(
		DNSServiceRef			sdRef,
		DNSServiceFlags			flags,
		DNSServiceErrorType		errorCode,
		const char			*	serviceName, 
		const char			*	regtype,
		const char			*	replyDomain,
		void				*	context
		)
{
	CBFDNSSDService * self = ( CBFDNSSDService* ) context;
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IBFDNSSDRegisterListener * listener = ( IBFDNSSDRegisterListener* ) self->m_listener;

		// Same for this

		if ( listener != NULL )
		{
			listener->OnRegister( self, ( flags & kDNSServiceFlagsAdd ) ? PR_TRUE : PR_FALSE, errorCode, NS_ConvertUTF8toUTF16( serviceName ), NS_ConvertUTF8toUTF16( regtype), NS_ConvertUTF8toUTF16( replyDomain ) );
		}
	}
}
