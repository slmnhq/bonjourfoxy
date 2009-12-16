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

#include "CDNSSDService.h"
#include "nsThreadUtils.h"
#include "nsIEventTarget.h"
#include "private/pprio.h"
#include <string>
#include <stdio.h>


/* To support XULRunner 1.9.0.x */
#ifndef NS_OUTPARAM
#define NS_OUTPARAM
#endif

NS_IMPL_ISUPPORTS2(CDNSSDService, IDNSSDService, nsIRunnable)

CDNSSDService::CDNSSDService()
:
	m_threadPool( NULL ),
	m_sdRef( NULL ),
	m_listener( NULL ),
	m_fileDesc( NULL ),
	m_job( NULL )
{
}


CDNSSDService::CDNSSDService( nsISupports * listener )
:
	m_threadPool( NULL ),
	m_sdRef( NULL ),
	m_listener( listener ),
	m_fileDesc( NULL ),
	m_job( NULL )
{
}


CDNSSDService::~CDNSSDService()
{
	Cleanup();
}


void
CDNSSDService::Cleanup()
{
	if ( m_listener != NULL )
	{
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
}


nsresult
CDNSSDService::SetupNotifications()
{
	NS_PRECONDITION( m_threadPool != NULL, "m_threadPool is NULL" );
	NS_PRECONDITION( m_fileDesc != NULL, "m_fileDesc is NULL" );
	NS_PRECONDITION( m_job == NULL, "m_job is not NULL" );

	m_iod.socket	= m_fileDesc;
	m_iod.timeout	= PR_INTERVAL_MAX;
	m_job			= PR_QueueJob_Read( m_threadPool, &m_iod, Read, this, PR_FALSE );	
	return ( m_job ) ? NS_OK : NS_ERROR_FAILURE;
}

/* IDNSSDService browse (in long interfaceIndex, in AString regtype, in AString domain, in IDNSSDBrowseListener listener); */
NS_IMETHODIMP
CDNSSDService::Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IDNSSDBrowseListener *listener, IDNSSDService **_retval NS_OUTPARAM)
{
	CDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;
	
	if (m_listener == NULL)
	{
		try
		{
			service = new CDNSSDService( listener );
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
		service->Browse(interfaceIndex, regtype, domain, listener, _retval);
		listener->AddRef();
		service->AddRef();
	}
	else
	{
		dnsErr = DNSServiceBrowse( &m_sdRef, 0, interfaceIndex, NS_ConvertUTF16toUTF8( regtype ).get(), NS_ConvertUTF16toUTF8( domain ).get(), ( DNSServiceBrowseReply ) BrowseReply, this );
		if ( dnsErr != kDNSServiceErr_NoError )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		if ( ( m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( m_sdRef ) ) ) == NULL )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		if ( ( m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		err = SetupNotifications();
		if ( err != NS_OK)
		{
		    goto exit;
		}
		*_retval = service;

	}
	

exit:

	if ( err && service )
	{
		delete service;
		service = NULL;
	}
	
	return err;
}


/* IDNSSDService resolve (in long interfaceIndex, in AString name, in AString regtype, in AString domain, in IDNSSDResolveListener listener); */
NS_IMETHODIMP
CDNSSDService::Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, IDNSSDResolveListener *listener, IDNSSDService **_retval NS_OUTPARAM)
{
	CDNSSDService	*	service	= NULL;
	DNSServiceErrorType dnsErr	= 0;
	nsresult			err		= 0;

	*_retval = NULL;

	if (m_listener == NULL)
	{
		try
		{
			service = new CDNSSDService( listener );
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
		service->Resolve(interfaceIndex, name, regtype, domain, listener, _retval);
		listener->AddRef();
		service->AddRef();
	}
	else
	{
		dnsErr = DNSServiceResolve( &m_sdRef, 0, interfaceIndex, NS_ConvertUTF16toUTF8( name ).get(), NS_ConvertUTF16toUTF8( regtype ).get(), NS_ConvertUTF16toUTF8( domain ).get(), ( DNSServiceResolveReply ) ResolveReply, this);
		if ( dnsErr != kDNSServiceErr_NoError )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		if ( ( m_fileDesc = PR_ImportTCPSocket( DNSServiceRefSockFD( m_sdRef ) ) ) == NULL )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		if ( ( m_threadPool = PR_CreateThreadPool( 1, 1, 8192 ) ) == NULL )
		{
			err = NS_ERROR_FAILURE;
			goto exit;
		}
		
		err = SetupNotifications();
		if ( err != NS_OK)
		{
		    goto exit;
		}
		*_retval = service;
	}
	

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
CDNSSDService::Stop()
{
	if ( m_sdRef )
	{
		DNSServiceRefDeallocate( m_sdRef );
		m_sdRef = NULL;
	}
	
	return NS_OK;
}


void
CDNSSDService::Read( void * arg )
{
	NS_PRECONDITION( arg != NULL, "arg is NULL" );
	
	NS_DispatchToMainThread( ( CDNSSDService* ) arg );
}


NS_IMETHODIMP
CDNSSDService::Run()
{
	nsresult err;
	
	NS_PRECONDITION( m_sdRef != NULL, "m_sdRef is NULL" );

	m_job = NULL;

	if ( DNSServiceProcessResult( m_sdRef ) == kDNSServiceErr_NoError )
	{
		err = SetupNotifications();
	}
	else
	{
		err = NS_ERROR_FAILURE;
	}
	
	return err;
}


void DNSSD_API
CDNSSDService::BrowseReply
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
	CDNSSDService * self = ( CDNSSDService* ) context;
	
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IDNSSDBrowseListener * listener = ( IDNSSDBrowseListener* ) self->m_listener;

		// Same for this

		if ( listener != NULL )
		{
			listener->OnBrowse( self, ( flags & kDNSServiceFlagsAdd ) ? PR_TRUE : PR_FALSE, interfaceIndex, errorCode, NS_ConvertUTF8toUTF16( serviceName ), NS_ConvertUTF8toUTF16( regtype ), NS_ConvertUTF8toUTF16( replyDomain ) );
		}
	}
}


void DNSSD_API
CDNSSDService::ResolveReply
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
	CDNSSDService * self = ( CDNSSDService* ) context;
	// This should never be NULL, but let's be defensive.
	
	if ( self != NULL )
	{
		IDNSSDResolveListener * listener = ( IDNSSDResolveListener* ) self->m_listener;
		
		// Same for this

		if ( listener != NULL )
		{
			std::string		path = "";
			const void	*	value = NULL;
			uint8_t			valueLen = 0;

			value = TXTRecordGetValuePtr( txtLen, txtRecord, "path", &valueLen );
			
			if ( value && valueLen )
			{
				char * temp;
				
				temp = new char[ valueLen + 1 ];
				
				if ( temp )
				{
					memset( temp, 0, valueLen + 1 );
					memcpy( temp, value, valueLen );
					path = temp;
					delete [] temp;
				}
			}

			listener->OnResolve( self, interfaceIndex, errorCode, NS_ConvertUTF8toUTF16( fullname ), NS_ConvertUTF8toUTF16( hosttarget ) , ntohs( port ), NS_ConvertUTF8toUTF16( path.c_str() ) );
		}
	}
}

