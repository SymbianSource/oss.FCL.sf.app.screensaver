/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:     This file defines the functions 
*                _NPNetscapeFuncs of Browser's Netscape API
*
*/





#include "NpnImplementation.h"


// -----------------------------------------------------------------------------
// NpnGetUrl
//
// Requests the associated pluginInst to load the given url.
// -----------------------------------------------------------------------------
//
 NPError NpnGetUrl( NPP /*aInstance*/, const TDesC& /*aUrl*/,
                   const TDesC* /*aWindowType*/ )
    {
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnPostUrl
//
// Requests the associated pluginInst to POST to the given url.
// -----------------------------------------------------------------------------
//
 NPError NpnPostUrl( NPP /*aInstance*/, const TDesC& /*aUrl*/, 
                    const TDesC* /*aWindowType*/,
                    const TDesC& /*aBuf*/, NPBool /*aFile*/ )
    {
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnRequestRead
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 NPError NpnRequestRead( NPStream* /*aStream*/, NPByteRange* /*aRangeList*/ )
    {
    // This function is not supported
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnNewStream
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 NPError NpnNewStream( NPP /*aInstance*/, NPMIMEType /*aType*/,
                      const TDesC* /*aWindowType*/, NPStream** /*aStream*/ )
    {
    // This function is not supported.
    // It is specifically intended to create a new Browser window
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnWrite
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 TInt32 NpnWrite( NPP /*aInstance*/, NPStream* /*aStream*/,
                 TInt32 /*aLen*/, void* /*aBuffer*/ )
    {
    // This function is not supported.
    // It is specifically intended to create a new Browser window
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnDestroyStream
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 NPError NpnDestroyStream( NPP /*aInstance*/, NPStream* /*aStream*/,
                          NPReason /*aReason*/ )
    {
    // This function is not supported.
    // It is specifically intended to create a new Browser window
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnStatus
//
// Allows a plugin to report its current status to the associated PluginInst.
// -----------------------------------------------------------------------------
//
 void NpnStatus( NPP /*aInstance*/, const TDesC& /*aMessage*/ )
    {   
    }

// -----------------------------------------------------------------------------
// NpnUAgent
//
// Returns the user agent of the Series 60 Browser.
// -----------------------------------------------------------------------------
//
 const TDesC* NpnUAgent( NPP /*aInstance*/ )
    {
    return NULL;
    }

// -----------------------------------------------------------------------------
// NpnMemAlloc
//
// Allocates a chunk of memory on the behalf of the plugin.
// -----------------------------------------------------------------------------
//
 void* NpnMemAlloc( uint32 aSize )
    {
    if ( aSize )
        {
        return User::Alloc( aSize );
        }
    return NULL;
    }

// -----------------------------------------------------------------------------
// NpnMemFree
//
// Deallocates a chunk of memory on the behalf of the plugin.
// -----------------------------------------------------------------------------
//
 void NpnMemFree( void* aPtr )
    {
    User::Free( aPtr );
    }

// -----------------------------------------------------------------------------
// NpnMemFlush
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 uint32 NpnMemFlush( uint32 /*aSize*/ )
    {
    // This function is not supported
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnReloadPlugins
//
// Causes the PluginHandler to scan for new plugins and reloads the current
// page if aReloadPages is true.
// -----------------------------------------------------------------------------
//
 void NpnReloadPlugins( NPBool /*aReloadPages*/ )
    {
    //This is currently not supported since CPluginHandler is non static
    }

// -----------------------------------------------------------------------------
// NpnGetJavaEnv
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 JRIEnv* NpnGetJavaEnv()
    {
    // This function is not supported
    return NULL;
    }

// -----------------------------------------------------------------------------
// NpnGetJavaPeer
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 jref NpnGetJavaPeer( NPP /*aInstance*/ )
    {
    // This function is not supported
    return NULL;
    }

// -----------------------------------------------------------------------------
// NpnGetUrlNotify
//
// Requests the associated PluginInst to load the given url.  The plugin is
// notified when the request completes.
// -----------------------------------------------------------------------------
//
 NPError NpnGetUrlNotify( NPP /*aInstance*/, const TDesC& /*aUrl*/,
                         const TDesC* /*aWindowType*/, void* /*aNotifyData*/ )
    {
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnPostUrlNotify
//
// Requests the associated PluginInst to POST to the given url.  The plugin is
// notified when the request completes.
// -----------------------------------------------------------------------------
//
 NPError NpnPostUrlNotify( NPP /*aInstance*/, const TDesC& /*aUrl*/,
                          const TDesC* /*aWindowType*/, const TDesC& /*aBuf*/,
                          NPBool /*aFile*/, void* /*aNotifyData*/ )
    {
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnGetValue
//
// Query the associated PluginInst for information.
// -----------------------------------------------------------------------------
//
 NPError NpnGetValue( NPP /*aInstance*/, NPNVariable /*aVariable*/, 
                     void* /*aRetValue*/ )
    {   
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnSetValue
//
// Set a value on the associated PluginInst
// -----------------------------------------------------------------------------
//
 NPError NpnSetValue( NPP /*aInstance*/, NPPVariable /*aVariable*/, 
                                        void* /*aSetValue*/ )
    {   
    return NPERR_GENERIC_ERROR;
    }

// -----------------------------------------------------------------------------
// NpnInvalidateRect
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 void NpnInvalidateRect( NPP /*aInstance*/, NPRect * /*aRect*/ )
    {
    // This function is not supported
    }

// -----------------------------------------------------------------------------
// NpnInvalidateRegion
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 void NpnInvalidateRegion( NPP /*aInstance*/, NPRegion /*aRegion*/ )
    {
    // This function is not supported
    }

// -----------------------------------------------------------------------------
// NpnForceRedraw
//
// Unsupported.
// -----------------------------------------------------------------------------
//
 void NpnForceRedraw( NPP /*aInstance*/ )
    {
    // This function is not supported
    }


// End Of File

