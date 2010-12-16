/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Document and application class file for screensaver app.
*
*/



#include <eikstart.h>
#include "screensaver.hrh"
#include "screensaver.h"
#include "screensaverappui.h"



// -----------------------------------------------------------------------------
// CScreensaverDocument::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverDocument::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverDocument::CScreensaverDocument
// -----------------------------------------------------------------------------
//
CScreensaverDocument::CScreensaverDocument(CEikApplication& aApp): CAknDocument(aApp) 
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverDocument::~CScreensaverDocument
// -----------------------------------------------------------------------------
//
CScreensaverDocument::~CScreensaverDocument()
    {
    
    }

//
// CScreensaverEikDocument
//

// -----------------------------------------------------------------------------
// CScreensaverDocument::CreateAppUiL
// -----------------------------------------------------------------------------
//
CEikAppUi* CScreensaverDocument::CreateAppUiL()
    {
    return ( new (ELeave) CScreensaverAppUi );
    }


//
// CScreensaverApplication
//
// -----------------------------------------------------------------------------
// CScreensaverApplication::CreateDocumentL
// -----------------------------------------------------------------------------
//
CApaDocument* CScreensaverApplication::CreateDocumentL()
    {
    CScreensaverDocument* document=new(ELeave) CScreensaverDocument(*this);
    // No need to do ConstructL for document since it is empty.
    return(document);
    }

// -----------------------------------------------------------------------------
// CScreensaverApplication::AppDllUid
// -----------------------------------------------------------------------------
//
TUid CScreensaverApplication::AppDllUid() const
    {
    return KUidScreensaverApp;
    }


//
// EXPORTed functions
//
LOCAL_C CApaApplication* NewApplication()
    {
    return new CScreensaverApplication;
    }

    
GLDEF_C TInt E32Main()
    {
    RProcess().SetPriority( EPriorityHigh );
    return EikStart::RunApplication(NewApplication);
    }


// End of file.
