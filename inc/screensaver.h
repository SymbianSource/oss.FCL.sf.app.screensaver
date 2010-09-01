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
* Description:     Main header file for screensaver application.
*
*/




#ifndef C_SCREENSAVER_H
#define C_SCREENSAVER_H

#include <e32base.h>

//For turning display off
#include <hal.h>

#include <centralrepository.h>

#include <aknapp.h>
#include <AknDoc.h>
#include <apgwgnam.h>

//
// class CScreensaverEikDocument
//
class CScreensaverDocument : public CAknDocument
	{
public:
    /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CScreensaverDocument object.
    */
    void ConstructL();
    /**
    * CScreensaverDocument.
    * C++ default constructor.
    */
    CScreensaverDocument(CEikApplication& aApp);

    /**
    * ~CScreensaverDocument 
    * Destructor.
    */
    ~CScreensaverDocument();
private:
    // from CEikDocument
    CEikAppUi* CreateAppUiL();
    };

//
// CScreensaverApplication
//

class CScreensaverApplication : public CAknApplication
{
private:
    // from CApaApplication
    /**
    * From CApaApplication, CreateDocumentL.
    * Creates CScreensaverDocument document object. The returned
    * pointer in not owned by the CScreensaverApplication object.
    * @return A pointer to the created document object.
    */
    CApaDocument* CreateDocumentL();
    /**
    * From CApaApplication, AppDllUid.
    * @return Application's UID (KUidScreensaverApp).
    */
    TUid AppDllUid() const;
    };

#endif // C_SCREENSAVER_H
