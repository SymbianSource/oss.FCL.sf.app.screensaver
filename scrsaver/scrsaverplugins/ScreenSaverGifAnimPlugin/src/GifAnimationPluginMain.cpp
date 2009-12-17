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





#include <ecom/implementationproxy.h>
#include <CEcomBrowserPluginInterface.h>


#include "GifAnimationPlugin.h"
#include "GifAnimationUtils.h"

// ---------------------------------------------------------------------------
// Table of implementation UID / implementation class factory
// ---------------------------------------------------------------------------
//
const TImplementationProxy KImplementationTable[] =
    {
        { { KGifAnimationPluginImplementationValue }, 
          (TProxyNewLPtr) CGifAnimationPluginEcomMain::NewL
        }
    };


/************************************************************************/
/**********ECOM STYLE Screensaver GifAnimation plug-in ENTRY POINTS******/
/************************************************************************/

// ---------------------------------------------------------------------------
// Instance factory.
// ---------------------------------------------------------------------------
//
CGifAnimationPluginEcomMain* CGifAnimationPluginEcomMain::NewL( 
                                                    TAny* aInitParam )
    {
    DBG_TRACE_FN_BEGIN;        
    TFuncs* funcs = REINTERPRET_CAST( TFuncs*, aInitParam);
    CGifAnimationPluginEcomMain* self = 
           new( ELeave )CGifAnimationPluginEcomMain( funcs->iNetscapeFuncs );
    CleanupStack::PushL(self);
    self->Construct( funcs->iPluginFuncs );
    CleanupStack::Pop();
    DBG_TRACE_FN_END;
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CGifAnimationPluginEcomMain::~CGifAnimationPluginEcomMain()
    {
    DBG_TRACE_FN_BEGIN;        
    // nothing here
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor.
// ---------------------------------------------------------------------------
//
void CGifAnimationPluginEcomMain::Construct( NPPluginFuncs* aPluginFuncs )
    {
    DBG_TRACE_FN_BEGIN;        
    InitializeFuncs( aPluginFuncs );
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Initializes NPP with pointers to implementing functions.
// ---------------------------------------------------------------------------
//
NPError CGifAnimationPluginEcomMain::InitializeFuncs( NPPluginFuncs* aPpf )
    {
    DBG_TRACE_FN_BEGIN;        
    aPpf->size          = sizeof(NPPluginFuncs);
    aPpf->version       = 1;
    aPpf->newp          = NewNPP_NewProc( GifAnimationPluginNewp );
    aPpf->destroy       = NewNPP_DestroyProc( GifAnimationPluginDestroy );
    aPpf->setwindow     = NewNPP_SetWindowProc( GifAnimationPluginSetWindow );
    aPpf->newstream     = NewNPP_NewStreamProc( GifAnimationPluginNewStream );
    aPpf->destroystream = NewNPP_DestroyStreamProc( 
                                    GifAnimationPluginDestroyStream );
    aPpf->asfile        = NewNPP_StreamAsFileProc( GifAnimationPluginAsFile );
    aPpf->writeready    = NewNPP_WriteReadyProc( 
                                    GifAnimationPluginWriteReady );
    aPpf->write         = NewNPP_WriteProc( GifAnimationPluginWrite );
    aPpf->print         = NewNPP_PrintProc( GifAnimationPluginPrint );
    aPpf->event         = NewNPP_HandleEventProc( GifAnimationPluginEvent );
    aPpf->urlnotify     = NewNPP_URLNotifyProc( GifAnimationPluginURLNotify );
    aPpf->javaClass     = NULL;
    aPpf->getvalue      = NewNPP_GetValueProc( GifAnimationPluginGetValue );
    aPpf->setvalue      = NewNPP_SetValueProc( GifAnimationPluginSetValue );

    DBG_TRACE_FN_END;
    return NPERR_NO_ERROR;
    }

// ---------------------------------------------------------------------------
// Constructor.
// ---------------------------------------------------------------------------
//
CGifAnimationPluginEcomMain::CGifAnimationPluginEcomMain( 
                                    NPNetscapeFuncs* aNpf ) 
    : CEcomBrowserPluginInterface(), iNpf( aNpf )
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Returns the filters implemented in this DLL
// Returns: The filters implemented in this DLL
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( 
                                                    TInt& aTableCount )
    {
    DBG_TRACE_FN_BEGIN;        
    aTableCount = sizeof( KImplementationTable ) 
                        / sizeof( TImplementationProxy );
    DBG_TRACE_FN_END;
    return KImplementationTable;
    }

#ifndef EKA2
// ---------------------------------------------------------------------------
// The E32Dll() entry point function. Deprecated.
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
    {
    DBG_TRACE_FN_BEGIN;        
    //
    DBG_TRACE_FN_END;
    return KErrNone;
    }
#endif // EKA2
