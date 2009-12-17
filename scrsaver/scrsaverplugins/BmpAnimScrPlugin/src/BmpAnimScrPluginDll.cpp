/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:     Plugin DLL export that creates the plugin object itself
*
*/





#include "CBmpAnimScrPlugin.h"


#ifndef EKA2
// Dll entry point or something
GLDEF_C TInt E32Dll(TDllReason)
	{
	return KErrNone;
	}
#endif
    

#define KCScreensaverPluginInterfaceDefinitionImpUid 0x101F87F9

#include <implementationproxy.h>

// __________________________________________________________________________
// Exported proxy for instantiation method resolution
// Use the plugin UID and Implementation factory method
// as a pair for ECom instantiation.
const TImplementationProxy ImplementationTable[] =
	{
	IMPLEMENTATION_PROXY_ENTRY(KCScreensaverPluginInterfaceDefinitionImpUid, CBmpAnimScrPlugin::NewL)
	};

// Give the table to the caller
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
	{
	aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
	return ImplementationTable;
	}
//  End of File
