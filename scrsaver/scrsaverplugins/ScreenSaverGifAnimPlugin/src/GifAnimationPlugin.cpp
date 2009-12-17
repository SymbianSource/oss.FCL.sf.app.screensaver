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
* Description:     Screensaver GifAnimation plug-in implementation source file
*
*/





#include <e32std.h>
#include <eikenv.h>
#include <barsread.h>
#include <fbs.h>
#include <gdi.h>
#include <f32file.h>
#include <CEcomBrowserPluginInterface.h>

#include "GifAnimationPlugin.h"
#include "GifAnimationUtils.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Instance factory.
// ---------------------------------------------------------------------------
//
CGifAnimationPlugin* CGifAnimationPlugin::NewL( NPP anInstance )
    {
    DBG_TRACE_FN_BEGIN;        
    CGifAnimationPlugin *self = new (ELeave) CGifAnimationPlugin;
    CleanupStack::PushL( self );
    self->Construct( anInstance );

    CleanupStack::Pop();
    DBG_TRACE_FN_END;
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CGifAnimationPlugin::~CGifAnimationPlugin()
    {
    DBG_TRACE_FN_BEGIN;        
    if ( iControl )
        {
        delete iControl;
        iControl=NULL;
        }
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Notifies host that plug-in has finished.
// ---------------------------------------------------------------------------
//
void CGifAnimationPlugin::PrepareToExitL()
    {
    DBG_TRACE_FN_BEGIN;        

    if ( iPluginAdapter )
        {
        CCoeControl* aParentControl = iPluginAdapter->GetParentControl();
        if ( iControl != NULL )
            {
            // will free resources, if file was loaded but was not stopped
            iControl->StopAnimation();
            }
        iPluginAdapter->PluginFinishedL();
        }
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor.
// ---------------------------------------------------------------------------
//
void CGifAnimationPlugin::Construct( NPP anInstance )
    {
    DBG_TRACE_FN_BEGIN;        
    iInstance=anInstance;
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Sets the parent window to plug-in. If already set, only updates size.
// ---------------------------------------------------------------------------
//
void CGifAnimationPlugin::SetWindowL( const NPWindow *aWindow, 
                                      const TRect& aRect )
    {
    DBG_TRACE_FN_BEGIN;        
    iPluginAdapter = static_cast<MPluginAdapter*>( aWindow->window );
    CCoeControl* aParentControl = iPluginAdapter->GetParentControl();
    if ( iControl == NULL )
        {
        iControl = CGifAnimationPluginControl::NewL( aParentControl, 
													 iPluginAdapter );
        iPluginAdapter->PluginConstructedL( iControl );
        }

    if ( iControl != NULL )
        {
        iControl->SetRect( aRect );
        }
    DBG_TRACE_FN_END;
    }

// ---------------------------------------------------------------------------
// Returns pointer on instance of CGifAnimationPluginControl
// ---------------------------------------------------------------------------
//
CGifAnimationPluginControl* CGifAnimationPlugin::Control()
    {
    DBG_TRACE_FN_BEGIN;        
    ASSERT( iControl );
    DBG_TRACE_FN_END;
    return iControl;
    }

// ---------------------------------------------------------------------------
// Returns pointer on NPP instance
// ---------------------------------------------------------------------------
//
NPP CGifAnimationPlugin::Instance()
    {
    DBG_TRACE_FN_BEGIN;        
    ASSERT( iInstance );
    DBG_TRACE_FN_END;
    return iInstance;
    }
