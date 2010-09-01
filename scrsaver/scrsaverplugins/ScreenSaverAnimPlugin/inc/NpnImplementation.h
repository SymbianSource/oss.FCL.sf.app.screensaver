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
* Description:     Thin wrapper around plugin's function table.
*
*/





#ifndef NPNIMPLEMENTATION_H
#define NPNIMPLEMENTATION_H

 // INCLUDE FILES
#include <e32std.h>
#include <e32base.h>
#include <eikenv.h>
#include <npupp.h>
#include <sysutil.h>


NPError NpnGetUrl( NPP aInstance, const TDesC& aUrl, 
        const TDesC* aWindowType );


NPError NpnPostUrl( NPP aInstance, const TDesC& aUrl, 
        const TDesC* aWindowType, const TDesC& aBuf, NPBool aFile );


NPError NpnRequestRead( NPStream* /*aStream*/, 
        NPByteRange* /*aRangeList*/ );


NPError NpnNewStream( NPP /*aInstance*/, NPMIMEType /*aType*/, 
        const TDesC* /*aWindowType*/, NPStream** /*aStream*/ );


TInt32 NpnWrite( NPP /*aInstance*/, NPStream* /*aStream*/, 
        TInt32 /*aLen*/, void* /*aBuffer*/ );


NPError NpnDestroyStream( NPP /*aInstance*/, 
        NPStream* /*aStream*/, NPReason /*aReason*/ );


void NpnStatus( NPP /*aInstance*/, const TDesC& aMessage );


const TDesC* NpnUAgent( NPP /*aInstance*/ );


void* NpnMemAlloc( uint32 aSize );


void NpnMemFree( void* aPtr );


uint32 NpnMemFlush( uint32 /*aSize*/ );


void NpnReloadPlugins( NPBool aReloadPages );


JRIEnv* NpnGetJavaEnv();


jref NpnGetJavaPeer( NPP /*aInstance*/ );


NPError NpnGetUrlNotify( NPP aInstance, const TDesC& aUrl, 
        const TDesC* aWindowType, void* aNotifyData );


NPError NpnPostUrlNotify( NPP aInstance, const TDesC& aUrl, 
        const TDesC* aWindowType, const TDesC& aBuf, NPBool aFile, void* aNotifyData );


NPError NpnGetValue( NPP aInstance, NPNVariable aVariable, void *aRetValue );


NPError NpnSetValue( NPP aInstance, NPPVariable aVariable, void *aSetValue );


void NpnInvalidateRect( NPP /*aInstance*/, NPRect * /*aRect*/ );


void NpnInvalidateRegion( NPP /*aInstance*/, NPRegion /*aRegion*/ );


void NpnForceRedraw( NPP /*aInstance*/ );


   
// -----------------------------------------------------------------------------
// PluginNpnFuncs
//
// A const global variable used to hold the browser callback functions.
// -----------------------------------------------------------------------------
//
const NPNetscapeFuncs NpnImplementationFuncs =
    {
    sizeof( NPNetscapeFuncs ),
    1, // Version
    NpnGetUrl,
    NpnPostUrl,
    NpnRequestRead,
    NpnNewStream,
    NpnWrite,
    NpnDestroyStream,
    NpnStatus,
    NpnUAgent,
    NpnMemAlloc,
    NpnMemFree,
    NpnMemFlush,
    NpnReloadPlugins,
    NpnGetJavaEnv,
    NpnGetJavaPeer,
    NpnGetUrlNotify,
    NpnPostUrlNotify,
    NpnGetValue,
    NpnSetValue,
    NpnInvalidateRect,
    NpnInvalidateRegion,
    NpnForceRedraw
    };


#endif // NPNIMPLEMENTATION_H
// End Of file
