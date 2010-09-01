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
* Description:   Implementation file for class CScreenSaverAIWPlugin
*
*/



#include <eikmenup.h>
#include <f32file.h>
#include <e32property.h>
#include <AiwMenu.h>
#include <AiwCommon.h>
#include <AiwCommon.hrh>
#include <barsread.h>
#include <eikenv.h>
#include <implementationproxy.h>
#include <StringLoader.h>
#include <aknnotewrappers.h>
#include <data_caging_path_literals.hrh>
#include <screensaveraiwplugin.rsg>
#include <AknsSrvClient.h>
#include <imageconversion.h>
#include <centralrepository.h>
#include <ScreensaverInternalCRKeys.h>
#include <ScreensaverInternalPSKeys.h>
#include <pslninternalcrkeys.h> 
#include "ScreenSaverAnimPluginInternalCRKeys.h"
#include "screensaveraiwplugin.h"
#include "screensaveraiwplugin.rh"

_LIT(KResourceFileName, "screensaveraiwplugin.rsc");
_LIT(KDriveZ, "z:");
_LIT(KPluginUidAnimation, "[1020744D]25");  // '25' is the drive number (Z = ROM), not 
_LIT(KPluginUidSlideshow, "[102823ED]25");  // mandatory


_LIT(KScreenSaverAIWPluginMimeTypeImage, "image");
_LIT(KScreenSaverAIWPluginMimeTypeOTABitmap, "image/x-ota-bitmap");
_LIT(KScreenSaverAIWPluginSeparator, "/");

_LIT(KScreenSaverAIWPluginMimeTypeGIF, "image/gif");
_LIT(KScreenSaverAIWPluginMimeTypeSVG, "image/svg+xml");
_LIT(KScreenSaverAIWPluginMimeTypeM3G, "application/m3g");
_LIT(KScreenSaverAIWPluginMimeTypeSWF, "application/x-shockwave-flash");


// ======== LOCAL FUNCTIONS ========

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CScreenSaverAIWPlugin::NewL
// -----------------------------------------------------------------------------
//
CScreenSaverAIWPlugin* CScreenSaverAIWPlugin::NewL()
    {
    CScreenSaverAIWPlugin* self = new( ELeave ) CScreenSaverAIWPlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CScreenSaverAIWPlugin::CScreenSaverAIWPlugin
// -----------------------------------------------------------------------------
//
CScreenSaverAIWPlugin::CScreenSaverAIWPlugin():
    iConeResLoader(*CCoeEnv::Static())
    {
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAIWPlugin::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::ConstructL()
    {
    TParse parse;
    parse.Set(KResourceFileName, &KDC_RESOURCE_FILES_DIR, &KDriveZ);
    TFileName resourceFileName;
    resourceFileName.Append(parse.FullName());
    iConeResLoader.OpenL(resourceFileName);  
    }    

// -----------------------------------------------------------------------------
// CScreenSaverAIWPlugin::~CScreenSaverAIWPlugin
// -----------------------------------------------------------------------------
//
CScreenSaverAIWPlugin::~CScreenSaverAIWPlugin()
    {
    iConeResLoader.Close();
    iSupportedImageFiles.Close();
    delete iScreensaverRepository;
    delete iAnimationRepository;
    }

// ---------------------------------------------------------------------------
// From class CAiwServiceIfMenu.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::InitialiseL(MAiwNotifyCallback& /*aFrameworkCallback*/,
    const RCriteriaArray& /*aInterest*/)
    {
    // Not needed.
    }

// ---------------------------------------------------------------------------
// From class CAiwServiceIfMenu.
//
// Implements setting the passed image(s) as screensaver.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::HandleServiceCmdL(const TInt& aCmdId,
    const CAiwGenericParamList& aInParamList,
    CAiwGenericParamList& /*aOutParamList*/,
    TUint /*aCmdOptions*/,
    const MAiwNotifyCallback* aCallback)
    {
     // Handle only KAiwCmdAssignScreenSaver command.
    if ( aCmdId == KAiwCmdAssignScreenSaver )
        {
#ifdef CSCREENSAVERAIWPLUGIN_TRACES	
        DumpParamList(_L("HandleServiceCmdL"),aInParamList);
#endif

        // Leave, if there were not given all the required parameters
        ValidateParamListL(aInParamList);
        
        // Get the supported image files to iSupportedImageFiles
        GetSupportedImageFilesL(aInParamList,iSupportedImageFiles);

#ifdef CSCREENSAVERAIWPLUGIN_TRACES	
        DumpSupportedImageFileList(iSupportedImageFiles);
#endif
        
        TInt textResourceId = 0;        
        if (iSupportedImageFiles.Count() == 1 && IsAnimatedImageL(iSupportedImageFiles[0].iFileName,
                                                                  iSupportedImageFiles[0].iMimeType))
            {
            textResourceId = R_SCREEN_SAVER_AIW_PLUGIN_TEXT_IMAGE_AS_ANIMATION;
            SetAnimatedScreenSaverL(iSupportedImageFiles);
            }
        else
            {
            textResourceId = ((iSupportedImageFiles.Count() > 1)?R_SCREEN_SAVER_AIW_PLUGIN_TEXT_IMAGES_ADDED:
                                                               R_SCREEN_SAVER_AIW_PLUGIN_TEXT_IMAGE_ADDED);           
            SetSlideShowScreenSaverL(iSupportedImageFiles);
            }            
            
        //  display confirmation note
        DisplayInfoNoteL(textResourceId);              

        // If aCallback defined inform consumers that we have done with
        // the operation.
        if (aCallback)
            {
            // Cope with the design problems of AIW framework
            MAiwNotifyCallback* nonConstCallback =
            const_cast<MAiwNotifyCallback*> (aCallback);
        
            CAiwGenericParamList* eventParamList = CAiwGenericParamList::NewL();
            CleanupStack::PushL(eventParamList);
            nonConstCallback->HandleNotifyL(
        	    KAiwCmdAssignScreenSaver,
        	    KAiwEventCompleted,
        	    *eventParamList,
        	    aInParamList);
            CleanupStack::PopAndDestroy(eventParamList);
            }
        }
    }
    
// ---------------------------------------------------------------------------
// From class CAiwServiceIfMenu.
//
// Inserts plugin's menu items to aMenuPane.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::InitializeMenuPaneL(CAiwMenuPane& aMenuPane,
    TInt aIndex,
    TInt /*aCascadeId*/,
    const CAiwGenericParamList& aInParamList)
    {        
#ifdef CSCREENSAVERAIWPLUGIN_TRACES	
        DumpParamList(_L("InitializeMenuPaneL"),aInParamList);        
#endif
    			
	// Insert menu only if there is at least a single supported MIME type is given
	if ( AnyMimeTypeSupportedL(aInParamList) )   
		{
	    TResourceReader reader;
	    CCoeEnv::Static()->CreateResourceReaderLC(reader, R_SCREEN_SAVER_AIW_PLUGIN_MENU);
	    aMenuPane.AddMenuItemsL(reader, KAiwCmdAssignScreenSaver, aIndex);
	    CleanupStack::PopAndDestroy(); // reader
		}
    }
    
// ---------------------------------------------------------------------------
// From class CAiwServiceIfMenu.
//
// Implements menu command handling for EScreenSaverAIWPluginCmdSetScreenSaver.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::HandleMenuCmdL(
	TInt aMenuCmdId,
    const CAiwGenericParamList& aInParamList,
    CAiwGenericParamList& aOutParamList,
    TUint aCmdOptions,
    const MAiwNotifyCallback* aCallback)
    {
    if (aMenuCmdId == EScreenSaverAIWPluginCmdSetScreenSaver)
        {
        // Menu commands are handled as service commands.
        HandleServiceCmdL(
            KAiwCmdAssignScreenSaver,
            aInParamList,
            aOutParamList,
            aCmdOptions,
            aCallback );
        }
    }

// -----------------------------------------------------------------------------
// Implements showing information note 
// -----------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::DisplayInfoNoteL(TInt aTextResourceId)
    {
    HBufC* text = StringLoader::LoadLC(aTextResourceId);
    CAknInformationNote* dlg = new (ELeave) CAknInformationNote(EFalse);
    dlg->ExecuteLD(*text);
    CleanupStack::PopAndDestroy(text);
    }


// -----------------------------------------------------------------------------
// Validates aParamList if it is not valid
// the method leaves with KErrArgument
// -----------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::ValidateParamListL(const CAiwGenericParamList& aParamList)
    {
    TBool valid = EFalse;
    
    // The parameter list must be dividable by 2 because the number of file name
    // items must be equal with the number of MIME type items.
    valid = !(aParamList.Count()%2); 
    
    valid = valid && AnyMimeTypeSupportedL(aParamList);
    
    if (valid)
        {
        TInt index = 0;
        TPtrC fileName = GetAiwParamAsDescriptor(index, aParamList, EGenericParamFile);
        
        // At least one file name parameter should be given
        valid = !(fileName == KNullDesC);
        }
    
    if (!valid)
        {
		User::Leave(KErrArgument);
        }    
    }
    
// -----------------------------------------------------------------------------
// Gets supported image file list from generic param list
// -----------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::GetSupportedImageFilesL(const CAiwGenericParamList& aParamList,
                                                    RArray<TImageFile>& aSupportedImageFiles)
    {    
    aSupportedImageFiles.Reset();
    for (TInt i = 0;; i++)
        {        
        TImageFile imageFile;
        TPtrC fileName = GetAiwParamAsDescriptor(i, aParamList, EGenericParamFile);
        
        if (fileName == KNullDesC)
            {
            // last file item is reached
            break;    
            }
            
        TPtrC mimeTypeString = GetAiwParamAsDescriptor(i, aParamList, EGenericParamMIMEType);
        if (mimeTypeString == KNullDesC)
            {
            // missing MIME type for file
            User::Leave(KErrArgument);    
            break;
            }
        else if (IsMimeTypeSupportedL(mimeTypeString))
            {
            imageFile.iFileName.Set(fileName);
            imageFile.iMimeType.Set(mimeTypeString);
            aSupportedImageFiles.Append(imageFile);
            }            
        }
    }
    

// -----------------------------------------------------------------------------
// Returns ETrue if any of the MIME types is supported
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAIWPlugin::AnyMimeTypeSupportedL(const CAiwGenericParamList& aParamList)
    {
    TBool ret = EFalse;
    for (TInt i = 0;; i++)
        {
        TPtrC mimeTypeString = GetAiwParamAsDescriptor(i, aParamList, EGenericParamMIMEType);
        if (mimeTypeString == KNullDesC)
            {
            // If no MIME type parameters are passed just leave
            if (i <= 0)
                {
                User::Leave(KErrArgument);    
                }            
            break;
            }
        else if (IsMimeTypeSupportedL(mimeTypeString))
            {
            ret = ETrue;
            break;
            }            
        }
    return ret;
    }
// -----------------------------------------------------------------------------
// Implements checking if a given MIME type is supported or not
// -----------------------------------------------------------------------------
//
TBool CScreenSaverAIWPlugin::IsMimeTypeSupportedL(const TDesC& aMimeTypeString)
    {   
    // Check for a type separator in the string
    TInt pos = aMimeTypeString.Find(KScreenSaverAIWPluginSeparator);

    // Leave if no separator was found.. the MIME
    // standard requires it   
    if (pos == KErrNotFound)
        {
        User::Leave(KErrArgument);
        }
    
	// Construct the compare string    
    TPtrC compareString(aMimeTypeString.Left(pos));

	// Perform the comparison
    TBool ret = EFalse;
    
    // Mime type case:  IMAGE/* except IMAGE/X-OTA-BITMAP
    if (!compareString.CompareF(KScreenSaverAIWPluginMimeTypeImage) &&
         aMimeTypeString.CompareF(KScreenSaverAIWPluginMimeTypeOTABitmap))
        {
        ret = ETrue;
        }   
        
    if (!ret)         	   		    	
        {
        TBool dummy;
        ret = IsAnimatedMimeTypeL(aMimeTypeString,dummy);
        }
    
	return ret;    
    }    
    
// ---------------------------------------------------------------------------
// Determines if the MIME type is animated or not and in addition returns
// in aUseImageDecoder if image decoder should be used to determine if
// the image is really animated
// ---------------------------------------------------------------------------
//
TBool CScreenSaverAIWPlugin::IsAnimatedMimeTypeL( const TDesC& aMimeTypeString, TBool& aUseImageDecoder )
    {    
    TBool ret = EFalse;     
    aUseImageDecoder = EFalse; 
    
    // check if animated an needs image decoder
    if (!aMimeTypeString.CompareF(KScreenSaverAIWPluginMimeTypeGIF))
        {
        aUseImageDecoder = ETrue;
        ret = ETrue; 
        }
    else if (!aMimeTypeString.CompareF(KScreenSaverAIWPluginMimeTypeSVG) ||
             !aMimeTypeString.CompareF(KScreenSaverAIWPluginMimeTypeM3G) ||
             !aMimeTypeString.CompareF(KScreenSaverAIWPluginMimeTypeSWF))
        {
        ret = ETrue; 
        }
    
    return ret;
    }
    
    
// -----------------------------------------------------------------------------
// Implements getting a AIW parameter as descriptor
// -----------------------------------------------------------------------------
//    
TPtrC CScreenSaverAIWPlugin::GetAiwParamAsDescriptor(
        TInt& aIndex,
		const CAiwGenericParamList& aParamList,
		TGenericParamId aParamType)
	{	
	const TAiwGenericParam* genericParam = NULL;
    genericParam = aParamList.FindFirst( 
        aIndex,
        aParamType,
        EVariantTypeDesC);
        
    if (aIndex != KErrNotFound && genericParam)
        {
        // Get the data
        return genericParam->Value().AsDes();
        }
	else
    	{
		return KNullDesC();
    	}
	}
	
#ifdef CSCREENSAVERAIWPLUGIN_TRACES	

// -----------------------------------------------------------------------------
// Dump parameters in aParamList
// -----------------------------------------------------------------------------
//   
void CScreenSaverAIWPlugin::DumpParamList(const TDesC& aMethod, const CAiwGenericParamList& aParamList)
    {
    DumpParamList(aMethod,EGenericParamFile,aParamList);
    DumpParamList(aMethod,EGenericParamMIMEType,aParamList);
    }
    
// -----------------------------------------------------------------------------
// Dump parameters in aParamList for a given aParamType
// -----------------------------------------------------------------------------
//   
void CScreenSaverAIWPlugin::DumpParamList(const TDesC& aMethod,
                                          TGenericParamId aParamType,
                                          const CAiwGenericParamList& aParamList)
	{	
    for (TInt i = 0 ;; i++)
        {
        TPtrC paramString = GetAiwParamAsDescriptor(i, aParamList, aParamType);
        if (paramString == KNullDesC)
            {
            break;
            }
        else 
            {
            RDebug::Print( _L("ScreenSaverAIWPlugin: %x CScreenSaverAIWPlugin::%S: Index=%d, paramType=%d, paramString=%S"), this, &aMethod, i, aParamType, &paramString);
            }            
        }
	}    
	
// -----------------------------------------------------------------------------
// Dump supported image file list
// -----------------------------------------------------------------------------
//   
void CScreenSaverAIWPlugin::DumpSupportedImageFileList(const RArray<TImageFile>& aSupportedImageFiles)
	{	
    for (TInt i = 0 ; i < aSupportedImageFiles.Count() ; i++)
        {
        RDebug::Print( _L("ScreenSaverAIWPlugin: %x DumpSupportedImageFileList: Index=%d, file=%S, MIME type=%S"), this, i, &aSupportedImageFiles[i].iFileName, &aSupportedImageFiles[i].iMimeType);
        }
	}    
	
#endif	    

// ---------------------------------------------------------------------------
// Determines if an image is animated or not
// ---------------------------------------------------------------------------
//
TBool CScreenSaverAIWPlugin::IsAnimatedImageL( const TDesC& aFileName,
                                               const TDesC& aMimeTypeString )
    { 
    TBool useImageDecoder = EFalse;
    TBool animated = IsAnimatedMimeTypeL( aMimeTypeString, useImageDecoder );
    
    if (useImageDecoder)
        {
        animated = IsReallyAnimatedImageL(aFileName);
        }
        
    return animated;
    }

// ---------------------------------------------------------------------------
// Determines if an image file is really animated or not
// ---------------------------------------------------------------------------
//
TBool CScreenSaverAIWPlugin::IsReallyAnimatedImageL( const TDesC& aFileName )
    {    
    CImageDecoder* decoder = CImageDecoder::FileNewL(CCoeEnv::Static()->FsSession(),aFileName);
    CleanupStack::PushL( decoder );
    TBool animated = (decoder->FrameCount() > 1);
    CleanupStack::PopAndDestroy(); // decoder
    return animated;
    }
    
// ---------------------------------------------------------------------------
// Set and activate slide-show screen saver.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::SetSlideShowScreenSaverL( const RArray<TImageFile>& aSupportedImageFiles  )
    {
    StoreImageListToFileL(aSupportedImageFiles);
    
    if (iScreensaverRepository == NULL)
        {
        // Setting the screensaver type & plugin name 
        iScreensaverRepository = CRepository::NewL(KCRUidScreenSaver);
        }
    if (iSlideshowRepository == NULL)
        {
        iSlideshowRepository = CRepository::NewL(KCRUidThemes);        
        }
        
    // set psln to "not random"
    iSlideshowRepository->Set(KThemesScreenSaverSlideSetType, 0);        
        
    // Slideshow plugin used
    iScreensaverRepository->Set(KScreenSaverPluginName, KPluginUidSlideshow);
    
    // Type 3 = plugin
    iScreensaverRepository->Set(KScreenSaverObject, EScreensaverTypePlugin);
    
    // Inform screensaver that slide set has changed
    User::LeaveIfError(
            RProperty::Set(
                KPSUidScreenSaver,
                KScreenSaverPluginSettingsChanged,
                EScreenSaverPluginSettingsChanged ) );
    }
    
// ---------------------------------------------------------------------------
// Set and activate animated screen saver.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::SetAnimatedScreenSaverL( const RArray<TImageFile>& aSupportedImageFiles  )
    {
    if (iScreensaverRepository == NULL)
        {
        // Setting the screensaver type & plugin name 
        iScreensaverRepository = CRepository::NewL(KCRUidScreenSaver);
        }    
        
    if (iAnimationRepository == NULL)
        {
        // Setting the screensaver type & plugin name 
        iAnimationRepository = CRepository::NewL(KCRUidScreenSaverAnimPlugin);
        }
        
    iAnimationRepository->Set(KScreenSaverAnimatedFileName, aSupportedImageFiles[0].iFileName);
    
    // Animation plugin used
    iScreensaverRepository->Set(KScreenSaverPluginName, KPluginUidAnimation);
    
    // Type 3 = plugin
    iScreensaverRepository->Set(KScreenSaverObject, EScreensaverTypePlugin);
    }
    
// ---------------------------------------------------------------------------
// Stores slide set image file names to file.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::StoreImageListToFileL( const RArray<TImageFile>& aSupportedImageFiles  )
    {
    // Get file from server.
    RFile imgFile;
    OpenImageFileL( imgFile, EAknsSrvInifileSSSS );
    CleanupClosePushL( imgFile );    

    // Finally, write image filenames to the file.
    TFileText textFile;
    textFile.Set( imgFile );
    textFile.Seek( ESeekEnd );

    for (TInt i = 0 ; i < aSupportedImageFiles.Count(); i++)
        {
        User::LeaveIfError( textFile.Write( aSupportedImageFiles[i].iFileName ) );
        }
                
    imgFile.Flush();

    CleanupStack::PopAndDestroy(); // imgFile
    }

// ---------------------------------------------------------------------------
// Open image list file for operations.
// ---------------------------------------------------------------------------
//
void CScreenSaverAIWPlugin::OpenImageFileL( RFile& aImageFile, const TInt aSlideSetType )
    {
    // First, connect to skin server.
    RAknsSrvSession skinsrv;
    User::LeaveIfError( skinsrv.Connect() );
    CleanupClosePushL( skinsrv );

    // Then get file handle.
    TInt fileserverhandle = 0;
    TInt filehandle = 0;
    // Validate type and open image file.
    if ( aSlideSetType == EAknsSrvInifileSSWP )
        {
        fileserverhandle = 
            skinsrv.OpenImageInifile( EAknsSrvInifileSSWP, filehandle );
        }
    else
        {
        fileserverhandle = 
            skinsrv.OpenImageInifile( EAknsSrvInifileSSSS, filehandle );

        }
    if ( fileserverhandle <= 0 || filehandle == 0 )
        {
        User::Leave( fileserverhandle );
        }

    // Finally adopt file from server.
    User::LeaveIfError( aImageFile.AdoptFromServer( fileserverhandle, filehandle ) );
    CleanupStack::PopAndDestroy(); // skinsrv
    }


    
// ======== ECOM INITIALIZATION ========

// Map the interface UIDs to implementation factory functions
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KScreenSaverAIWPluginImplementationUid, CScreenSaverAIWPlugin::NewL)
    };

// ---------------------------------------------------------
// Exported proxy for instantiation method resolution
// ---------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }

