/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:     ECOM implementation array.
*
*/






// INCLUDE FILES
#include    <e32std.h>
#include    <ecom/implementationinformation.h>    
#include    <ecom/implementationproxy.h>
#include    <coecntrl.h>
#include    "SlideshowPluginImpUID.hrh"
#include    "SlideshowPlugin.h"

// CONSTANTS
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KSlideshowPluginImpUID, 
                    CSlideshowPlugin::NewL)
    };

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// ImplementationProxy
// ECOM implementation table entry.
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

    return ImplementationTable;
    }

// End of File

