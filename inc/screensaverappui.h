/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   The define of appui for screensaver
*
*/




#ifndef C_SCREENSAVERAPPUI_H
#define C_SCREENSAVERAPPUI_H

#include <e32base.h>
#include <aknViewAppUi.h>

#include "screensaverview.h"


class CScreensaverEngine;

// UID of Screensaver application
const TUid KUidScreensaverApp  = { 0x100056CF };

/**
 *  'AppUi' class.
 */
class CScreensaverAppUi : public CAknViewAppUi
    {
public:	
	/**
    * ConstructL.
    * 2nd phase constructor.
    */
    void ConstructL();

    /**
    * ~CScreensaverAppUi.
    * Destructor.
    */
    ~CScreensaverAppUi();
    
    /**
    * Returns the pointer to screensaver model which is owned by AppUi
    * @return Screensaver Model
    */
    CScreensaverEngine& Model() const;

    /**
    * From CEikAppUi.
    * Takes care of command handling.
    * @param aCommand command to be handled
    */
    void HandleCommandL(TInt aCommand);

    /**
    * Returns the pointer to CScreensaverView which is derived from AknView
    * @return CScreensaverView
    */
    CScreensaverView* ScreensaverView();

private:
    
    /**
    * Creates the CScreensaverView object and the Display object   
    */
    void CreateViewL();

    /**
    * From CEikAppUi.
    * Key event handler.
    */
    TKeyResponse HandleKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/);
    
    /**
    * From @c CCoeAppUiBase.   
    * Calls CAknViewAppUi::HandleScreenDeviceChangedL().
    */
    void HandleScreenDeviceChangedL();

    /**
    * From AknAppUi. Handles pointer-initiated view switch. Currently this
    * function does nothing but calls base class function.
    * @since Series 60 3.0
    * @param aEvent Window server event.
    * @param aDestination Pointer to the control which the event is targeted to.
    */
    void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);

private:
    
    /**
    * Screensaver Model.
    * Owned.
    */
    CScreensaverEngine* iModel;
    
    /**
    * Screensaver view
    * 
    * Owned
    */
    CScreensaverView* iView;
    };

#endif // C_SCREENSAVERAPPUI_H

