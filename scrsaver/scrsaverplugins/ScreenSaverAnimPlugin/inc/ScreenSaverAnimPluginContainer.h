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
* Description:   This header defines the container for the settings list
*
*/



#ifndef C_SCREENSAVERANIMPLUGINCONTAINER_H
#define C_SCREENSAVERANIMPLUGINCONTAINER_H

// INCLUDES
#include    <coeccntx.h>
#include    <eikclb.h>
#include    <eiklbo.h>

// CLASS DECLARATION

/**
* CScreenSaverAnimPluginContainer container class
*
*/
class CScreenSaverAnimPluginContainer : public CCoeControl,
                                        public MEikListBoxObserver
    {
    public: // Constructors and destructor

        static CScreenSaverAnimPluginContainer* NewLC(
                           CScreenSaverAnimPlugin* aCaller, 
                           const CCoeControl* aParent,
                           CScreenSaverAnimSettingObject* aSettingObject );

        static CScreenSaverAnimPluginContainer* NewL( 
                              CScreenSaverAnimPlugin* aCaller,
                              const CCoeControl* aParent,
                              CScreenSaverAnimSettingObject* aSettingObject );

        CScreenSaverAnimPluginContainer(
                          CScreenSaverAnimPlugin* aCaller,
                          CScreenSaverAnimSettingObject* aSettingObject );

        virtual void ConstructL( const CCoeControl* aParent,
                                const TRect& aRect );

        /**
        * Constructs all common parts of the container.
        * @param aRect Listbox control's rect.
        * @param aResTitleId Resource id for title.
        * @param aResLbxId Resource id for listbox.
        * @return void
        */
        void BaseConstructL( const TRect& aRect, TInt aResTitleId, 
                    TInt aResLbxId = 0 );

        /**
        * Destructor.
        */
        ~CScreenSaverAnimPluginContainer();

    public: //new

        /**
        * Updates listbox's item's value.
        * @param aItemId An item which is updated.
        * @return void.
        */
        void UpdateListBoxL( TInt aItemId );

        void HandleListBoxSelectionL();

        /**
        * Sets feature support - see screensaveranimpluginconst.h for feature list.
        * @param aFeatureBitFlags feature support bitflags.
        */
        void GetFeatureSupportL( TBitFlags& aFeatureBitFlags ) const;

        /**
        * Checks if feature is supported.
        * @param aFeatureID feature ID.
        * @return ETrue if feature is supported, EFalse otherwise.
        */
        TBool IsFeatureSupported( TInt aFeatureID );

    public: //MEikListBoxObserver 

        virtual void HandleListBoxEventL( CEikListBox* aListBox, 
                TListBoxEvent aEventType );

    public: //From CCoeControl

        virtual void HandleResourceChange( TInt aType );

        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                TEventCode aType ); // this processes key events

        virtual void SetContainerWindowL( const CCoeControl& aContainer );

        // Focus change handler
        void FocusChanged( TDrawNow aDrawNow );

    protected: // New methods

        /**
        * Constructs listbox.
        * @param aResLbxId Resource id for listbox.
        * @return void
        */
        virtual void ConstructListBoxL( TInt aResLbxId );

    private: // new
        void CreateListBoxItemsL();
        void MakeAnimTimeOutItemL();
        void MakeLightsTimeOutItemL();
        void MakeFileNameItem();
        void ShowFileNameItemL();
        void ShowAnimTimeOutItemL();
        void ShowLightsTimeOutItemL();

    private: // from CCoeControl

        void SizeChanged();

        TInt CountComponentControls() const;

        CCoeControl* ComponentControl( TInt /*aIndex*/ ) const;
        
        void GetNormalScreenLayoutRect( TRect& aRect )  const;

    protected: // Data
        CEikTextListBox* iListBox;
        
    private: // new
        CDesCArrayFlat* iItems;
        CDesCArray* iItemArray;
        TBuf<128> iItem;
        CScreenSaverAnimSettingObject* iSettingObject;
        CScreenSaverAnimPlugin* iCaller;
        TBool iInShowFileItem;

        /**
        * Defines which features are supported.
        */
        TBitFlags iFeaturesSupport;

    };

#endif // C_SCREENSAVERANIMPLUGINCONTAINER_H

// End of File
