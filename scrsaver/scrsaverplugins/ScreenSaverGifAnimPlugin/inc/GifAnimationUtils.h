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
* Description:     Screensaver GifAnimation plug-in utility file header
*
*/





#ifndef GIFANIMPLUGINUTIL_H
#define GIFANIMPLUGINUTIL_H

// INCLUDES
#include <e32base.h>
#include <aknnotewrappers.h>

#ifdef _DEBUG

// MACROS
#define DBG_TRACE( str ) \
    RDebug::Printf( "GifAnimation plug-in: %s:%s:%d %s", \
                    __FILE__, __FUNCTION__, __LINE__, (str) );
#define DBG_TRACE_FN_BEGIN \
    RDebug::Printf("GifAnimation plug-in: %s:%s begin", \
                   __FILE__, __FUNCTION__);
#define DBG_TRACE_FN_END \
    RDebug::Printf("GifAnimation plug-in: %s:%s end", \
                   __FILE__, __FUNCTION__);

#define DBG_ERR_DLG( strDesC, code ) \
        CAknErrorNote* informationNote = \
            new ( ELeave ) CAknErrorNote(ETrue); \
        TBuf<256> message; \
        _LIT(KErrorFormat, "Error: %S, code: %d"); \
        message.Format(KErrorFormat, &strDesC, code); \
        informationNote->ExecuteLD( message );
#else

#define DBG_TRACE( str )
#define DBG_TRACE_FN_BEGIN
#define DBG_TRACE_FN_END
#define DBG_ERR_DLG( strDesC, code )

#endif // _DEBUG

#endif // GIFANIMPLUGINUTIL_H
