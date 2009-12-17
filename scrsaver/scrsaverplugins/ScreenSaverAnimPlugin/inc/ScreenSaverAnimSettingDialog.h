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
* Description:   ScreenSaverAnimPlugin Settings Dialog
*
*/



#ifndef C_SCREENSAVERANIMSETTINGDIALOH_H
#define C_SCREENSAVERANIMSETTINGDIALOH_H


// FORWARD DECLARATIONS
class CAknDialog;
class CScreenSaverAnimPluginContainer;
class CScreenSaverAnimSettingObject;

/**
*  Provides the dialog functionality for viewing Settings.
*  @lib ScreenSaverAnimPlugin.dll
*  @since 3.1
*/
 
class CScreenSaverAnimSettingDialog : public CAknDialog
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CScreenSaverAnimSettingDialog* NewL(
                              CScreenSaverAnimPlugin* aCaller,
                              CScreenSaverAnimSettingObject* aSettingObject );

        /**
        * Two-phased constructor.
        */
        static CScreenSaverAnimSettingDialog* NewLC(
                              CScreenSaverAnimPlugin* aCaller,
                              CScreenSaverAnimSettingObject* aSettingObject );

        // Destructor
        virtual ~CScreenSaverAnimSettingDialog();

    public: // New functions
    public: // Functions from base classes

        /**
        * From CAknDialog Executes the dialog and deletes the dialog on exit.
        * @since 3.1
        * @param aFileHandle File handle to the SVGT content.
        * @param aAppObs Implementation Class provided by application that
        *   uses this dialog
        * Note: The ownership of the file handle is not transferred. 
        *       It is the responsibility of the caller to close this file 
        *       after deleting this dialog.        
        * @return Integer - Indicates Exit Reason.
        */
        virtual TInt ExecuteLD();
        /**
        * From CAknDialog - Called by framework to process commands invoked by
        *  the user.
        */
        virtual void ProcessCommandL( TInt aCommandId );

#ifdef __SERIES60_HELP
        /**
        * From CCoeControl Context Sensitive Help ID retrieving function.
        * @since 3.0
        * @see CCoeControl
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;
#endif

    public:

        /**
        * Function used by the application to handle background/foreground
        * events. When the application goes to background the presentation
        * needs to be paused. When application comes back to foreground,
        * the user would need to start the paused presentation. 
        * @param aForeground: ETrue: Foreground event, 
        *                     EFalse: Background event.
        * @since 3.1
        */
        virtual void HandleApplicationForegroundEvent( TBool aForeground );

    protected: // Functions from base classes
        // From CEikDialog
        // From CAknDialog

        /**
        * From CAknDialog - Called by framework to dynamically manipulate
        *  the menu for the dialog.
        */
        void  DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

        /**
        * From CEikDialog. Prelayouts the dialog.
        */
        void PreLayoutDynInitL();

        /**
        * From CAknDialog - Framework method to determine if it OK
        *  to exit the dialog.
        */
        TBool OkToExitL( TInt aButtonId );

        /**
        * From CAknDialog - Called by framework to add custom control to the
        *  dialog when specified in resource.
        */
        SEikControlInfo CreateCustomControlL( TInt aControlType );

        /**
        * From CAknDialog - Called by framework to notify resize of dialog.
        */
        void SizeChanged();

        /**
        * From CCoeControl Key Handling Method for control.
        * @since 3.1
        * @param aKeyEvent Key Event.
        * @param aType Type of Key Event.
        * @return TKeyResponse - EKeyWasConsumed/EKeyWasNotConsumed.
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                     TEventCode aType );

        /**
        * From CCoeControl 
        * @see CCoeControl::HandleResourceChange
        */
        void HandleResourceChange( TInt aType );

        /**
        * From CCoeControl 
        * @see CCoeControl::FocusChanged
        */
        void FocusChanged( TDrawNow aDrawNow );

        /**
         * Sets middle softkey label.
         * @param aResourceId resource ID of label text to use.
         * @param aCommandId command to place to middle softkey.
         */
        void SetMiddleSoftKeyLabelL( const TInt aResourceId,
                                     const TInt aCommandId );

    private:
        /**
        * C++ default constructor.
        */
        CScreenSaverAnimSettingDialog( 
                 CScreenSaverAnimPlugin* aCaller,
                 CScreenSaverAnimSettingObject* aSettingObject );
        /**
        * Second Phase constructor.
        */
        void ConstructL();

    private:    // Data

        // Pointer to Custom CCoeControl.
        CScreenSaverAnimPluginContainer* iCustControl;

        // Application Title Text
        HBufC* iAppTitleText;

        // Text to be displayed in the title pane
        HBufC* iTitlePaneString;

        // Setting Object
        CScreenSaverAnimSettingObject* iSettingObject ;

        // Caller's reference
        CScreenSaverAnimPlugin* iCaller;

    };

#endif // C_SCREENSAVERANIMSETTINGDIALOH_H
// End of File
