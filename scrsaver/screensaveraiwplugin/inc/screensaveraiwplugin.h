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
* Description:     Header file for class CScreenSaverAIWPlugin
*
*/




#ifndef C_CSCREENSAVERAIWPLUGIN_H
#define C_CSCREENSAVERAIWPLUGIN_H

#include <AiwServiceIfMenu.h>
#include <ConeResLoader.h>

#define CSCREENSAVERAIWPLUGIN_TRACES


class CRepository;

/**
 *  CScreenSaverAIWPlugin plugin implementation.
 *
 *  This plugin implements an AIW provider for setting image files as slide set
 *  screen saver or setting a single animated image file as animated screen saver.
 *  
 *
 *  @since S60 v3.2
 */ 
class CScreenSaverAIWPlugin : public CAiwServiceIfMenu 
    {
    public: 
        static CScreenSaverAIWPlugin* NewL();
        virtual ~CScreenSaverAIWPlugin();

    private: 
    // from base class CAiwServiceIfMenu
    
        void InitialiseL(
            MAiwNotifyCallback& aFrameworkCallback,
            const RCriteriaArray& aInterest);

        void HandleServiceCmdL(
            const TInt& aCmdId,
            const CAiwGenericParamList& aInParamList,
            CAiwGenericParamList& aOutParamList,
            TUint aCmdOptions = 0,
            const MAiwNotifyCallback* aCallback = NULL);
            
        void InitializeMenuPaneL(
            CAiwMenuPane& aMenuPane,
            TInt aIndex,
            TInt aCascadeId,
            const CAiwGenericParamList& aInParamList);

        void HandleMenuCmdL(
            TInt aMenuCmdId,
            const CAiwGenericParamList& aInParamList,
            CAiwGenericParamList& aOutParamList,
            TUint aCmdOptions = 0,
            const MAiwNotifyCallback* aCallback = NULL);

    private:
        CScreenSaverAIWPlugin();
        void ConstructL();

    private: // New
    
        struct TImageFile
            {
                TPtrC iFileName;
                TPtrC iMimeType;
            };    
            
        void DisplayInfoNoteL(TInt aTextResourceId);
        void ValidateParamListL(const CAiwGenericParamList& aParamList);
        TBool AnyMimeTypeSupportedL(const CAiwGenericParamList& aParamList);
        TBool IsMimeTypeSupportedL(const TDesC& aMimeTypeString);
        TBool IsAnimatedMimeTypeL( const TDesC& aMimeTypeString, TBool& aUseImageDecoder );
        TPtrC GetAiwParamAsDescriptor(TInt& aIndex,
                                      const CAiwGenericParamList& aParamList,
									  TGenericParamId aParamType);
        void StoreImageListToFileL( const RArray<TImageFile>& aSupportedImageFiles  );
        void OpenImageFileL( RFile& aImageFile, const TInt aSlideSetType );
        TBool IsAnimatedImageL( const TDesC& aFileName, const TDesC& aMimeTypeString );
        TBool IsReallyAnimatedImageL( const TDesC& aFileName );
									  
#ifdef CSCREENSAVERAIWPLUGIN_TRACES	
								  
        void DumpParamList(const TDesC& aMethod, const CAiwGenericParamList& aParamList);
        void DumpParamList(const TDesC& aMethod, 
                           TGenericParamId aParamType,
                           const CAiwGenericParamList& aParamList);	
        void DumpSupportedImageFileList(const RArray<TImageFile>& aSupportedImageFiles);                       								  
#endif    

                                  
        void GetSupportedImageFilesL(const CAiwGenericParamList& aParamList,
                             RArray<TImageFile>& aSupportedImageFiles);
        void SetAnimatedScreenSaverL( const RArray<TImageFile>& aSupportedImageFiles );                             
        void SetSlideShowScreenSaverL( const RArray<TImageFile>& aSupportedImageFiles );
        
    private:
        RConeResourceLoader iConeResLoader;        
        RArray<TImageFile> iSupportedImageFiles;
        CRepository* iScreensaverRepository;
        CRepository* iAnimationRepository;
        CRepository* iSlideshowRepository;         
    };
    
#endif // C_CSCREENSAVERAIWPLUGIN_H
