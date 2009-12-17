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
* Description:   Screensaver view class defination.
*
*/



#ifndef C_SCREENSAVERVIEW_H
#define C_SCREENSAVERVIEW_H

#include <aknview.h>
#include <babitflags.h>

#include "screensaver.hrh"

//CLASS DECLARATION
class CScreensaverBase;
class CScreensaverEngine;

const TUid KUidScreensaverView ={ 1 };
//
// class CScreensaverView
// an empty place holder for when we are in the background
class CScreensaverView : public CAknView
    {
public:
    
    /**
    * Two-phased constructor
    */
    static CScreensaverView* NewL();

    /**
    * C++ default constructor
    */
    CScreensaverView();
    
    /**
    * Destructor
    */
    ~CScreensaverView();
    
    /**
    * Two-phased constructor
    */
    void ConstructL();
    
    /**
    * Creates the display object by the parameter
    * 
    * @param the object factory object, default type is EDisplayTime
    */
    void CreateDisplayObjectL( TDisplayObjectType aType );
    
    /**
    * Resets the display object
    */
    void SetDisplayObject( TDisplayObjectType aType );

    /**
    * Creates the preview display object
    */
    void CreatePreviewDisplayObjectL();
    

public://From CAknView
    
    /**
    * Returns views id, intended for overriding by sub classes.
    * @return id for this view.
    */
    TUid Id() const;

    /** 
    * Handles a view activation and passes the message of type 
    * @c aCustomMessageId. This function is intended for overriding by 
    * sub classes. This function is called by @c AknViewActivatedL().
    * Views should not launch waiting or modal notes or dialogs in
    * @c DoActivateL.
    * @param aPrevViewId Specifies the view previously active.
    * @param aCustomMessageId Specifies the message type.
    * @param aCustomMessage The activation message.
    */
    void DoActivateL( const TVwsViewId& aPrevViewId, TUid aCustomMessageId,
        const TDesC8& aCustomMessage );

    /** 
    * View deactivation function intended for overriding by sub classes. 
    * This function is called by @c AknViewDeactivated().
    */
    void DoDeactivate();
    
    /**
    * Shows the display object
    */
    void ShowDisplayObject();
    
    /**
    * Hides the display object
    */
    void HideDisplayObject();

    /**
    * Update indicator attributes and refresh display
    */
    void UpdateAndRefresh();

private:
    /**
    * Returns the default type of screensaver
    */
    TDisplayObjectType DefaultScreensaverType();
    
    /**
    * Returns the psln setting type of screensaver
    */
    TDisplayObjectType PslnSettingScreensaverType();
    
    /**
    * Get the model of appui
    */
    CScreensaverEngine& Model() const;

    /**
    * Delete the display object
    */
    void DestroyDisplayObject();
    
private:
    /**
    * The type of current control
    */
    TDisplayObjectType iCurrentControlType;

    /**
    * The display object of screensaver
    * 
    * Owned
    */
    CScreensaverBase* iControl;

    /**
    * The plugin refresh rate
    */
    TInt iPluginRefreshRate;

    };

#endif // C_SCREENSAVERVIEW_H
