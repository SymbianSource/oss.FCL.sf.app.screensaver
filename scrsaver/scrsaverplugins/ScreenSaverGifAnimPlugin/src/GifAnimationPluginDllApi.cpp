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
* Description:     Screensaver GifAnimation plug-in API file
*
*/





#include <IHLImageFactory.h>        // IHLImageFactory

#include "GifAnimationPlugin.h"
#include "NPNExtensions.h"
#include "GifAnimationUtils.h"

// ======== Ecom interface functions implementation ========

NPError GifAnimationPluginNewp( NPMIMEType /*pluginType*/, 
                                NPP instance, 
                                uint16 /*mode*/, 
                                CDesCArray* /*argn*/, 
                                CDesCArray* /*argv*/, 
                                NPSavedData* /*saved*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin* lGifAnimationPlugin=NULL;
    TRAPD(err,lGifAnimationPlugin=CGifAnimationPlugin::NewL( instance ) );

    if ( err == KErrNoMemory )
        {
        DBG_TRACE_FN_END;
        return NPERR_OUT_OF_MEMORY_ERROR;
        }
    if ( err != KErrNone )
        {
        DBG_TRACE_FN_END;
        return NPERR_MODULE_LOAD_FAILED_ERROR;
        }
    instance->pdata = (void *) lGifAnimationPlugin;
    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

NPError GifAnimationPluginDestroy( NPP instance, NPSavedData** /*save*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin* lGifAnimationPlugin = 
                ( CGifAnimationPlugin * )instance->pdata;
    if ( lGifAnimationPlugin )
        {
        TRAPD( err, lGifAnimationPlugin->PrepareToExitL() );
        if ( err != KErrNone )
            {
            DBG_TRACE( 
                "GifAnimationPluginDestroy Error when PrepareToExitL()" );
            }
        /**Ignoring the error*/
        delete lGifAnimationPlugin;
        lGifAnimationPlugin = NULL;
        }
    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

NPError GifAnimationPluginSetWindow( NPP instance, NPWindow *window )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin* lGifAnimationPlugin = 
                (CGifAnimationPlugin *) instance->pdata;
    TUint lWidth = window->width;
    TUint lHeight = window->height;
    TRAPD( err, 
          lGifAnimationPlugin->SetWindowL( window, 
                                      TRect( TSize( lWidth, lHeight ) ) ) );

    if ( err == KErrNoMemory )
        {
        DBG_TRACE_FN_END;
        return NPERR_OUT_OF_MEMORY_ERROR;
        }
    if ( err != KErrNone )
        {
        DBG_TRACE_FN_END;
        return NPERR_GENERIC_ERROR;
        }
    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

NPError GifAnimationPluginNewStream( NPP /*instance*/, 
                                     NPMIMEType /*type*/, 
                                     NPStream* /*stream*/, 
                                     NPBool /*seekable*/, 
                                     uint16* stype )
    {
    DBG_TRACE_FN_BEGIN;        
    *stype = NP_ASFILEONLY;
    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

NPError GifAnimationPluginDestroyStream( NPP /*instance*/, 
                                         NPStream* /*stream*/, 
                                         NPReason /*reason*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

void GifAnimationPluginAsFile( NPP instance, 
                               NPStream* /*stream*/, 
                               const TDesC& fname )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin* lGifAnimationPlugin = 
                ( CGifAnimationPlugin* )instance->pdata;
    TRAP_IGNORE( lGifAnimationPlugin->Control()->LoadImageL( fname ) );
    DBG_TRACE_FN_END;
    }

int32 GifAnimationPluginWriteReady( NPP /*instance*/, NPStream* /*stream*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    return 65536;
    }

int32 GifAnimationPluginWrite( NPP /*instance*/, 
                               NPStream* /*stream*/, 
                               int32 /*offset*/, 
                               int32 len, 
                               void* /*buffer*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    return len;
    }

int16 GifAnimationPluginEvent( NPP /*instance*/, void* /*event*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    return 0;
    }


NPError GifAnimationPluginGetValue( NPP instance, 
                                    NPPVariable variable, 
                                    void* ret_value )
    {
    DBG_TRACE_FN_BEGIN;        
    if ( variable == NPPCheckFileIsValid )
        {
        NPCheckFile* checkFile = (NPCheckFile*)ret_value;
        TRAPD( err, 
               CGifAnimationPluginControl::CheckFileIsValidL(
                                             checkFile->fileName->Des() ) );

        checkFile->fileValidityCheckError = err;
        DBG_TRACE_FN_END;
        return NPERR_NO_ERROR;
        }
        
    if ( variable == NPPScreenSaverGetLastError )
        {
        DBG_TRACE( "GetLastError" );
        TInt* retValue = (TInt*)ret_value;
        CGifAnimationPlugin* lGifAnimationPlugin = 
                    ( CGifAnimationPlugin* )instance->pdata;
        *retValue = lGifAnimationPlugin->Control()->GetLastError();
        DBG_TRACE_FN_END;
        return NPERR_NO_ERROR;
        }
        
    // new command, panic 
    _LIT( KErrGetMsg, 
          "CGifAnimationPluginDllApi::GifAnimationPluginGetValue" );
    User::Panic( KErrGetMsg, KErrNotSupported );
    return NPERR_GENERIC_ERROR;
    }

NPError GifAnimationPluginSetValue( NPP instance, 
                                    NPNVariable variable, 
                                    void* value )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin* lGifAnimationPlugin = 
                ( CGifAnimationPlugin* )instance->pdata;

    if ( variable == NPNScreenSaverAnimationEvent )
        {
        NPAnimationEvent* event = (NPAnimationEvent*)value;
        switch ( *event )
            {
            case NPStartAnimation:
                DBG_TRACE( "StartAnimationL" );
                TRAPD( errStart, 
                       lGifAnimationPlugin->Control()->StartAnimationL() ); 
                if ( errStart != KErrNone )     
                    {
                    DBG_TRACE( "StartAnimation failed!" );
                    return NPERR_GENERIC_ERROR;
                    }
                return NPERR_NO_ERROR;
            case NPEndAnimation:
                DBG_TRACE( "StopAnimation" );
                TRAPD( errEnd, 
                       lGifAnimationPlugin->Control()->StopAnimation() );
                if (errEnd != KErrNone)     
                    {
                    DBG_TRACE( "StopAnimation failed!" );
                    return NPERR_GENERIC_ERROR;
                    }
                DBG_TRACE_FN_END;
                return NPERR_NO_ERROR;
            default: 
                // new command, panic 
                _LIT( KErrSet1Msg, 
                  "CGifAnimationPluginDllApi::GifAnimationPluginSetValue1" );
                User::Panic( KErrSet1Msg, KErrNotSupported );
                return NPERR_GENERIC_ERROR;
            }
        }

    if ( variable == NPScreenSaverMode )
        {
        // we do not need the event
        DBG_TRACE_FN_END;
        return NPERR_NO_ERROR;
        }

    if ( variable == NPNInteractionMode )
        {
        // we do not need the event
        DBG_TRACE_FN_END;
        return NPERR_NO_ERROR;
        }

    // new command, panic 
    _LIT( KErrSet2Msg, 
          "CGifAnimationPluginDllApi::GifAnimationPluginSetValue2" );
    User::Panic( KErrSet2Msg, KErrNotSupported );
    return NPERR_GENERIC_ERROR;
    }

void GifAnimationPluginURLNotify( NPP /*instance*/, 
                                  const TDesC& /*url*/, 
                                  NPReason /*reason*/, 
                                  void* /*notifyData*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    }

void GifAnimationPluginPrint( NPP /*instance*/, NPPrint* /*platformPrint*/ )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    }
