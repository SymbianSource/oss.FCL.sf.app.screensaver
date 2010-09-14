/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This file implements the Slideshow Screensaver plugin.
*
*/



#include <eikenv.h>
#include <bautils.h>
#include <data_caging_path_literals.hrh>
#include <centralrepository.h>
#include <DRMHelper.h>

// #include <aknnotewrappers.h> AppendSlideL
// #include <aknnavi.h>
// #include <aknnavide.h>
// #include <mmfcontrollerpluginresolver.h>
#include <DRMHelperServerInternalCRKeys.h>
#include <mmf/common/mmfcontrollerpluginresolver.h> // For CleanupResetAndDestroyPushL
#include <pslninternalcrkeys.h>
#include <pslnslidesetdialoginterface.h>

// #include <NPNExtensions.h>
#include <coecntrl.h>
#include <ecom/ecom.h>

#include <ContentListingFactory.h>
#include <mdeobjectdef.h>
#include <mdelogiccondition.h>
#include <mdenamespacedef.h>
#include <mdeconstants.h>
#include <mdeobjectquery.h>

#include <slideshowplugin.rsg>

#include "SlideshowPlugin.h"
#include "SlideshowSlide.h"
#include "SlideshowPluginUtils.h"

// Constants
const TInt KSecsToMicros = 1000000;
const TUint KDefaultRandomLoadingNumber = 100;

// Slideshow duration times (secs)
const TInt KMinSlideshowTime = 1;
const TInt KMaxSlideshowTime = 60;
const TInt KDefaultSlideshowTime = 5;

// Backlight times (secs)
const TInt KMinLightsTime = 0;
const TInt KMaxLightsTime = 30;
const TInt KDefaultLightsTime = 0;

// Slide times 
// const TInt KMinSlideTime = 1;
// const TInt KMaxSlideTime = 30;
const TInt KDefaultSlideTime = 5;

// Refresh interval (Draw() call frequency) in secs.
// For better resolution kept smaller than slide time
const TInt KRefreshInterval = 1;

// Slideshow type
const TInt KSlideshowTypeContinuous = 0;
const TInt KSlideshowTypeRandom = 1;

// const TInt KScreensaverAnimPluginInterfaceUid = 0x102750CB;

_LIT(KResourceFileName, "Z:SlideshowPlugin.rsc");


CSlideshowSettings::CSlideshowSettings()
    {
    iSlideshowTime = KDefaultSlideshowTime;
    iLightsTime = KDefaultLightsTime;
    iSlideTime = KDefaultSlideTime;
    iSlideshowType = KSlideshowTypeContinuous;
    }

// ============================ MEMBER FUNCTIONS =============================


// ---------------------------------------------------------------------------
// CSlideshowPlugin::NewL
// ---------------------------------------------------------------------------
//
CSlideshowPlugin* CSlideshowPlugin::NewL()
    {
    CSlideshowPlugin* self = new (ELeave) CSlideshowPlugin;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ----------------------------------------------------------------------------
// CSlideshowPlugin::~CSlideshowPlugin
// ----------------------------------------------------------------------------
//   
CSlideshowPlugin::~CSlideshowPlugin()
    {    
    if (iResourceOffset != 0)
        {
        iEikEnv->DeleteResourceFile(iResourceOffset);
        }

    // Close and delete CR handlers
    NotifyCenrepChangeCancel();
    UnInitializeCenRep();

    // Close and delete P&S handlers
    if (iSettingsChangedSubscriber)
        {
        iSettingsChangedSubscriber->StopSubscribe();
        }
    iSettingsChangedProperty.Close();
    delete iSettingsChangedSubscriber;
    
    delete iScreensaverName;
    delete iSettings;
    delete iDrmHelper;
    delete iModel;

    // Close and delete mds query
    if ( iQuery )
        {
        iQuery->Cancel();
        delete iQuery;
        iQuery = NULL;
        }

	if ( iMdESession )
        {
        delete iMdESession;
        iMdESession = NULL;
        }
    
    // Logging done
    SSPLOGGER_DELETE;
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::ConstructL
// -----------------------------------------------------------------------------
//    
void CSlideshowPlugin::ConstructL()
    {
    // Start logging
    SSPLOGGER_CREATE;
    
    iModel = CSlideshowModel::NewL();
    iSettings = new (ELeave) CSlideshowSettings();

    // Get nearest language resource file and add it to the
    // eikon environment for the duration of the plugin life
    TFileName fileName;
    TParse lParse;
    
    // Add the resource dir
    lParse.Set(KResourceFileName, &KDC_RESOURCE_FILES_DIR, NULL);
    
    // Get the filename with full path
    fileName = lParse.FullName();

    iEikEnv = CEikonEnv::Static();

    BaflUtils::NearestLanguageFile(iEikEnv->FsSession(), fileName);
        
    iResourceOffset = iEikEnv->AddResourceFileL(fileName);

    // Load localised name of screensaver to be returned 
    // to Themes app in Name() query
    iScreensaverName = iEikEnv->AllocReadResourceL(R_SLIDESHOW_SCREENSAVER_NAME);

    InitializeCenRepL();

    // Read settings
    ReadSettings();
 
    iTimerUpdated = EFalse;
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::CSlideshowPlugin
// -----------------------------------------------------------------------------
CSlideshowPlugin::CSlideshowPlugin()
    {
    }


// --- MScreensaverPlugin ---


// -----------------------------------------------------------------------------
// CSlideshowPlugin::InitializeL
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::InitializeL(MScreensaverPluginHost *aHost)
    {
    SSPLOGGER_ENTERFN("InitializeL()");

    iWaitActive = EFalse;
    iDrawCount = 0;

    if (!aHost)
        {
        return KErrArgument;
        }

    iHost = aHost;   
    // iHost->UseRefreshTimer( EFalse );

    NotifyCenrepChangeL();

    // No indicators while plugin running
    iHost->OverrideStandardIndicators();    
    
    // Get display info
    UpdateDisplayInfo();

    // Attach and start watching settings changed key
    User::LeaveIfError(iSettingsChangedProperty.Attach(
        KPSUidScreenSaver, KScreenSaverPluginSettingsChanged));

    iSettingsChangedSubscriber = new (ELeave) CPSSubscriber(
        TCallBack(HandleSettingsChanged, this), iSettingsChangedProperty);
    iSettingsChangedSubscriber->SubscribeL();
    
    iDrmHelper = CDRMHelper::NewL(*iEikEnv);
    
    // Load slides on start rather than here
    // LoadSlidesL();

    SSPLOGGER_LEAVEFN("InitializeL()");
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::Draw
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::Draw(CWindowGc& aGc)
    {
    SSPLOGGER_ENTERFN("Draw()");
    
    // Clear the screen on the very first draw call after starting
    if (iDrawCount < 0)
        {
        iDrawCount = 0;
        aGc.SetBrushColor(KRgbBlack);
        aGc.Clear();
        }
    
    // If still waiting for CLF, bail out
    if (iWaitActive)
        {
        SSPLOGGER_WRITE("Draw(): Waiting for CLF, bail out");
        SSPLOGGER_LEAVEFN("Draw()");
        return KErrNone;
        }
    // Make sure we have something to display
    if (iModel->NumberOfSlides() == 0 && iIsLoadFinished ) 
    	{
    	iHost->RevertToDefaultSaver();
        SSPLOGGER_WRITE("Draw(): Nothing to display, suspending");
        SSPLOGGER_LEAVEFN("Draw()");
//        iHost->Suspend( -1 ); // This place will be crashed when transform to default type
        return KErrNone;
        }
    
    CSlideshowSlide* pSlide = iModel->NextSlide(EFalse);
   // get the status of the slide
    TInt status = iModel->SlideStatus(pSlide); // TInt   decoded(1), decoding(2), not decoding(3),no slide (0)
    											// 4 could not decode the slide
    if(KStatusDecodingInProgress == status) 
    	{
    	// decoding in progress... return
    	// when ever decoding is going on.. call draw method every 1 second to check the status of the decoding
    	// and draw the image as soon as it is available ( otherwise Draw is called every 5 secods, very bad user responce)
    	if(iTimerUpdated)
    		{
    		iHost->SetRefreshTimerValue(KSecsToMicros);
    		iTimerUpdated = EFalse;
    		}
    	return KErrNone;
      	}
    else if (KStatusSlideDecoded == status)
    	{
    	// File was decoded properly and available
    	SSPLOGGER_WRITEF(_L("SSP: Draw(): Got slide (%x), drawing"), pSlide);

    	// Consume slide's DRM rights
    	SlideshowUtil::DRMConsume(iDrmHelper, pSlide);
          
    	// Make sure the window is empty in case the bitmap doesn't
    	// fill the whole screen. Draw background black
    	aGc.SetBrushColor(KRgbBlack);
    	// aGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    	aGc.Clear();
    	// Draw image centered
    	DrawCentered(aGc, pSlide);
    	if(iHost && !iTimerUpdated)
        	{
        	iHost->SetRefreshTimerValue(iSettings->iSlideTime * KSecsToMicros);
        	iTimerUpdated = ETrue;
        	}
    	// Flush draw buffer so that the new image is displayed immediately
    	FlushDrawBuffer();
    	SSPLOGGER_WRITE("Draw(): Prepare next slide");

    	SSPLOGGER_LEAVEFN("Draw()");
    	}
    
    // Prefetch image for the next slide
    iModel->PrepareNextSlide(
            (iSettings->iSlideshowType == KSlideshowTypeRandom), iDi.iRect.Size());
    
    SSPLOGGER_LEAVEFN("Draw()");

    return KErrNone;
    	
  }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::Name
// -----------------------------------------------------------------------------
const TDesC16& CSlideshowPlugin::Name() const
    {
    if (iScreensaverName)
        {
        return *iScreensaverName;
        }
    else
        {
        return KNullDesC;
        }
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::Capabilities
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::Capabilities()
    {
    return (EScpCapsConfigure | EScpCapsSelectionNotification);
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::PluginFunction
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::PluginFunction(
    TScPluginCaps aFunction,
    TAny* aParam)
    {
    TInt ret = KErrNone;
    
    switch (aFunction)
        {
        case EScpCapsSelectionNotification:
        case EScpCapsPreviewNotification:
        // Peek into the slideset
        TRAP(ret, LoadSlidesL());
	    if (ret!=KErrNone || iModel->NumberOfSlides() == 0)
            {
            SSPLOGGER_WRITE("No slides selected - launching settings");
            TRAP(ret, ConfigureL(aParam));
            // Check if at least one file is selected    
            ReadSettings();// update the key value of KThemesScreenSaverSlideSetType
            TRAP(ret, LoadSlidesL());
            if ( ret == KErrNone && iModel->NumberOfSlides() == 0)
                {
                // Don't Pop an error note, just return error
                return KErrCancel;          
                }   
            }
            break;      
        case EScpCapsConfigure:
            // Configure command
            TRAP(ret, ConfigureL(aParam));
            break;
        default:
            break;
        }
    
    return ret;
    }  


// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleScreensaverEventL
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::HandleScreensaverEventL(
    TScreensaverEvent aEvent,
    TAny* /*aData*/ )
    {
    switch (aEvent)
        {
        case EScreensaverEventStopping:
            StopPlugin();
            break;
            
        case EScreensaverEventStarting:
            StartPlugin();
            break;
            
        case EScreensaverEventDisplayChanged:
            UpdateDisplayInfo();
            break;

        case EScreensaverEventTimeout:
            StopPlugin();
            iHost->Suspend(-1);
            break;
        
        default:
            break;
        }
    return KErrNone;
    }  

// -----------------------------------------------------------------------------
// CSlideshowPlugin::ReadSettings
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::ReadSettings()
    {
    
    TInt err = KErrNone;
    
    err = iSlideshowRepository->Get(
        KThemesScreenSaverSlideSetDuration,
        iSettings->iSlideshowTime);
                          
    if ((iSettings->iSlideshowTime < KMinSlideshowTime) ||
        (iSettings->iSlideshowTime > KMaxSlideshowTime))
        {
        // Out of range, set to default
        iSettings->iSlideshowTime = KDefaultSlideshowTime;
        }
    
    err = iSlideshowRepository->Get(
        KThemesScreenSaverSlideSetBacklight,
        iSettings->iLightsTime);

    if ((iSettings->iLightsTime < KMinLightsTime) ||
        (iSettings->iLightsTime > KMaxLightsTime))
        {
        // Out of range, set to default
        iSettings->iLightsTime = KDefaultLightsTime;
        }

    err = iSlideshowRepository->Get(
        KThemesScreenSaverSlideSetType,
        iSettings->iSlideshowType);

    // No setting for slide timing
    iSettings->iSlideTime = KDefaultSlideTime;

    return err;             
    }

#if 0

// -----------------------------------------------------------------------------
// CSlideshowPlugin::UpdateFileName
// -----------------------------------------------------------------------------
//      
TBool CSlideshowPlugin::UpdateFileName()
    {
    if ( !BaflUtils::FileExists( iEikEnv->FsSession(), 
                                iSettings->iFileName  ) )
        {
        // File is deleted now, Show empty string
        iSettings->iFileName.Copy( KEmptyString );        
        return ETrue;
        }
    
    TBool canbeautomated(EFalse);   

    TInt res = 0;
    iDrmHelper->SetAutomatedType( 
                   CDRMHelper::EAutomatedTypeScreenSaver );
    res = iDrmHelper->CanSetAutomated( iSettings->iFileName, 
                                       canbeautomated );

    if ( res || !canbeautomated)
        {
        iSettings->iFileName.Copy( KEmptyString );
        return ETrue;
        }
    return EFalse;
    }

#endif      
    
// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleRepositoryCallBack
// -----------------------------------------------------------------------------
//                
TInt CSlideshowPlugin::HandleRepositoryCallBack(TAny* aPtr)    
    {
    STATIC_CAST(CSlideshowPlugin*, aPtr)->HandleCRSettingsChange(aPtr);
    return KErrNone;
    }
    
    
// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleCRSettingsChange
// -----------------------------------------------------------------------------
//                
void CSlideshowPlugin::HandleCRSettingsChange(TAny* /*aPtr*/)    
    {    
    if (iSlideshowCRWatcher)
        {
        TInt changedKey = iSlideshowCRWatcher->ChangedKey();
        
        switch (changedKey)
            {
            case KThemesScreenSaverSlideSetType:
            case KThemesScreenSaverSlideSetBacklight:
            case KThemesScreenSaverSlideSetDuration:
                // Update settings
                ReadSettings();
                break;
                
            default:
                // Not interested in other keys
                break;
            }
        }
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleSettingsChanged
// -----------------------------------------------------------------------------
//                
TInt CSlideshowPlugin::HandleSettingsChanged(TAny* aPtr)    
    {    
    return (STATIC_CAST(CSlideshowPlugin*, aPtr)->SettingsChanged());
    }
    
    
// --- private functions ---

    
// -----------------------------------------------------------------------------
// CSlideshowPlugin::InitializeCenRepL
// Connect Central Repository sessions
// -----------------------------------------------------------------------------
void CSlideshowPlugin::InitializeCenRepL()
    {
    
    TRAPD(err, iSlideshowRepository = CRepository::NewL(KCRUidThemes));
    User::LeaveIfError(err);
    }

    
// -----------------------------------------------------------------------------
// CSlideshowPlugin::NotifyCenrepChangeL
// -----------------------------------------------------------------------------
//  
void CSlideshowPlugin::NotifyCenrepChangeL()
    { 
    iSlideshowCRWatcher = CRepositoryWatcher::NewL(
            KCRUidThemes,
            TCallBack(HandleRepositoryCallBack, this),
            iSlideshowRepository);
    }
        

// -----------------------------------------------------------------------------
// CSlideshowPlugin::NotifyCenrepChangeCancel
// -----------------------------------------------------------------------------
void CSlideshowPlugin::NotifyCenrepChangeCancel()
    {
    delete iSlideshowCRWatcher;
    iSlideshowCRWatcher = NULL;
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::UnInitializeCenRep
// -----------------------------------------------------------------------------
void CSlideshowPlugin::UnInitializeCenRep()
    {       
    if (iSlideshowRepository)
        {
        delete iSlideshowRepository;
        iSlideshowRepository = NULL;  
        }
    }


// Starts the saver when screensaver starts
void CSlideshowPlugin::StartPlugin()
    {
    // Make sure we start from a clean slate
    iDrawCount = -1;
    
    // (Re-)load slideset. When done on every start this avoids
    // having to keep track of gallery changes and images on MMC.
    TRAP_IGNORE(LoadSlidesL());

    if (iModel->NumberOfSlides() == 0)
        {
        iHost->RevertToDefaultSaver();
        return;
        }

    iModel->PrepareNextSlide(
        (iSettings->iSlideshowType == KSlideshowTypeRandom),
        iDi.iRect.Size());

    if (iHost)
        {
        // Start animating as per settings
        iHost->RequestTimeout(iSettings->iSlideshowTime);
        Lights(iSettings->iLightsTime);
        // let the draw method be called every 1 second till first image is available
        iHost->SetRefreshTimerValue(KRefreshInterval * KSecsToMicros);
        SetDisplayMode();
        }
    }


// Stops the plugin
void CSlideshowPlugin::StopPlugin()
    {
    // Let go of pre-fetched images
    iModel->ReleaseNextSlide();
    iTimerUpdated = EFalse;
    }
    

// Draws centered slides
void CSlideshowPlugin::DrawCentered(CWindowGc& aGc, CSlideshowSlide* aSlide)
    {
    CFbsBitmap* bitmap = aSlide->Image();
    
    if (!bitmap)
        return;
    
    // Center the bitmap horizontally and vertically (crop off excess)    
    TPoint pos;
    TRect rectToDraw;
    TSize sizeBmp = bitmap->SizeInPixels();
    TInt screenWidth = iDi.iRect.Width();
    TInt screenHeight = iDi.iRect.Height();
    
    // Horizontally
    if (sizeBmp.iWidth <= screenWidth)
        {
        // Width fits on screen - center xpos
        pos.iX = (screenWidth - sizeBmp.iWidth) / 2;
        
        // Whole width of bmp can be drawn
        rectToDraw.SetWidth(sizeBmp.iWidth);
        }
    else
        {
        // Bmp wider than screen - xpos top left
        pos.iX = 0;
        
        // Adjust draw rect position and width
        rectToDraw.iTl.iX = (sizeBmp.iWidth - screenWidth) / 2;
        rectToDraw.SetWidth(screenWidth);
        }
    
    // Vertically
    if (sizeBmp.iHeight <= screenHeight)
        {
        // Height fits on screen - center ypos
        pos.iY = (screenHeight - sizeBmp.iHeight) / 2;
        
        // Whole height of bmp can be drawn
        rectToDraw.SetHeight(sizeBmp.iHeight);
        }
    else
        {
        // Bmp higher than screen - ypos top left
        pos.iY = 0;
        
        // Adjust draw rect position and height
        rectToDraw.iTl.iY = (sizeBmp.iHeight - screenHeight) / 2;
        rectToDraw.SetHeight(screenHeight);
        }
    
    // Do the drawing
    aGc.BitBlt(pos, bitmap, rectToDraw);
    }


// Loads the slides into the model
void CSlideshowPlugin::LoadSlidesL()
    {
    SSPLOGGER_ENTERFN("LoadSlidesL()");
    iIsLoadFinished = EFalse;
    // Based on settings, load predefined set or random slides
    if (iSettings->iSlideshowType == KSlideshowTypeRandom)
        {
        LoadRandomSlidesL();
        }
    else
        {
        LoadSlideSetL();
        }
    iIsLoadFinished = ETrue;
    SSPLOGGER_LEAVEFN("LoadSlidesL()");
    }
    

// Load filenames from settings file stored in skin server directory
void CSlideshowPlugin::LoadSlideSetL()
    {
    SSPLOGGER_ENTERFN("LoadSlideSetL()");
    
    // Start by getting rid of possibly loaded slides
    iModel->DeleteAll();

    // Connect to skin server.
    RAknsSrvSession skinSrv;
    TInt error = skinSrv.Connect();
    User::LeaveIfError(error);
    CleanupClosePushL(skinSrv);

    // Open images file
    TInt fileSrvHandle;
    TInt fileHandle;
    fileSrvHandle = skinSrv.OpenImageInifile(EAknsSrvInifileSSSS, fileHandle);

    // Adopt the handle to our process
    RFile imgFile;
    error = imgFile.AdoptFromServer(fileSrvHandle, fileHandle);
    User::LeaveIfError(error);
    CleanupClosePushL(imgFile);
    
    // Read image filenames from file (as text)
    TFileText textFile;
    textFile.Set(imgFile);
    textFile.Seek(ESeekStart);

    // Read until EOF
    TInt count = 0;
    TFileName fileName;
    while (textFile.Read(fileName) == KErrNone)
        {
        // Check that the file exists. If not, it is still OK, if it is
        // on the memory card - it may show up later. Omit files from other
        // drives that do not exist at the time of loading
        TBool exists = BaflUtils::FileExists(iEikEnv->FsSession(), fileName);
        TBool isOnMC = SlideshowUtil::IsOnMC(fileName);
        
        if (!exists) //  && (!isOnMC))
            {
            // Do not add nonexisting files // from other than memory card
            continue;
            }
#if 0
        // Check that the file's DRM rights allow it to be displayed (if not
        // missing because not on MMC
        if (!SlideshowUtil::DRMCheck(fileName))
            {
            // No point in adding files that cannot be displayed anyway
            continue;
            }
#endif        
        // Create a slide with the filename and store it in the model
        CSlideshowSlide* pSlide = CSlideshowSlide::NewL(fileName, isOnMC);
        CleanupStack::PushL(pSlide);
        iModel->AppendSlideL(pSlide);
        CleanupStack::Pop(pSlide);

        SSPLOGGER_WRITEF(_L("SSP: Slide %d added, file: %S"), count, &fileName);
        count++;
        }

    CleanupStack::PopAndDestroy(2); // imgFile, skinSrv
    
    SSPLOGGER_LEAVEFN("LoadSlideSetL()");    
    }


// Load filenames from CLF
void CSlideshowPlugin::LoadRandomSlidesL()
    {
    SSPLOGGER_ENTERFN("LoadRandomSlidesL()");
    // Start by getting rid of possibly loaded slides
    iModel->DeleteAll();
    // connect to MDS, load data or not
    ConnectToMDSSessionL();

    SSPLOGGER_LEAVEFN("LoadRandomSlidesL()");
    }


// Requests display mode from host
void CSlideshowPlugin::SetDisplayMode()
    {
    if (!iHost)
        {
        return;
        }
    
    // Exit partial mode
    iHost->ExitPartialMode();
    }


// Requests lights
void CSlideshowPlugin::Lights(TInt aSecs)
    {
    // Request nothing, if no time set (don't force lights off)
    if ((iHost) && (aSecs > 0))
        {
        SSPLOGGER_WRITEF(_L("SSP: Request lights for %d secs"), aSecs);
        iHost->RequestLights(aSecs);
        }
    }


// Configure the plugin
void CSlideshowPlugin::ConfigureL(TAny* /* aParam */)
    {
    // Look for screensaver slideset setting interface
    RImplInfoPtrArray array;            
    const TUid slidesetInterface = { 0x102823AD };
    const TUid slidesetImplUidScreensaver = { 0x102823AF };
    const TEComResolverParams emptyParams;
    
    REComSession::ListImplementationsL(
        slidesetInterface,
        emptyParams,
        KRomOnlyResolverUid,
        array);
    CleanupResetAndDestroyPushL(array);

    // Loop through implementations and look for screensaver
    // slideset implementation (this is not really completely
    // necessary, because we already know the implementation UID,
    // but we want to be sure it's there, and on ROM only
    TInt nCount = array.Count();
    TUid implUid;

    for (TInt i = 0; i < nCount; i++)
        {
        CImplementationInformation* info = array[i];
        
        if (info->ImplementationUid() == slidesetImplUidScreensaver)
            {
            // Found
            implUid = info->ImplementationUid();
            break;
            }
        }

    // Did we get it?
    if (implUid != slidesetImplUidScreensaver)
        {
        User::Leave(KErrNotFound);
        }
    
    // Instantiate the interface
    CPslnSlidesetDialogInterface* plugin = 
        CPslnSlidesetDialogInterface::NewL(implUid);

    TInt dlgRet = KErrNone;
    if (plugin)
        {
        dlgRet = plugin->ExecuteDialogLD();
        }

    // Cleanup
    array.ResetAndDestroy();
    CleanupStack::PopAndDestroy(); // array

    User::LeaveIfError(dlgRet);
    }

    
// Updates the saved information about display
void CSlideshowPlugin::UpdateDisplayInfo()
    {
    iDi.iSize = sizeof(TScreensaverDisplayInfo);
    iHost->DisplayInfo(&iDi);
    }

    
// Flushes pending draws
void CSlideshowPlugin::FlushDrawBuffer()
    {
    iEikEnv->WsSession().Flush();
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::SettingsChanged
// Called when Themes reports a settings change through P & S
// -----------------------------------------------------------------------------
TInt CSlideshowPlugin::SettingsChanged()
    {
    // If current key value is nonzero, it means the settings
    // have just changed, otherwise, no action necessary
    TInt value = 0;
    iSettingsChangedProperty.Get(value);

    if (value == EScreenSaverPluginSettingsChanged)
        {
#if 0
        // Slide file has changed - re-load images
        TRAP_IGNORE(LoadSlidesL());
        // Pre-fetch the fist image to be drawn
        iModel->PrepareNextSlide(
            (iSettings->iSlideshowType == KSlideshowTypeRandom),
            iDi.iRect.Size());
#endif
        iSettingsChangedProperty.Set(EScreenSaverPluginSettingsNoChange);
        }

    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CSlideshowPlugin::ConnectToMDSSessionL
// Connects to MDS Listing Framework. Can be called many times,
// connects only once
// -----------------------------------------------------------------------------
void CSlideshowPlugin::ConnectToMDSSessionL()
    {
    if (!iMdESession)
        {
        iMdESession = CMdESession::NewL( *this );
        // Wait for query of MDS to complete before continuing
        WaitForMDS();
        }
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::WaitForMDS
// Begins wait for MDS session connected 
// -----------------------------------------------------------------------------
void CSlideshowPlugin::WaitForMDS()
    {
    if (iWaitActive)
        {
        return;
        }
    else
        {
        iWaitActive = ETrue;
        iWaiter.Start();
        }
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::WaitForMDS
// Ends wait for MDS
// -----------------------------------------------------------------------------
void CSlideshowPlugin::EndWaitForMDS()
    {
    if (!iWaitActive)
        {
        return;
        }
    else
        {
        iWaiter.AsyncStop();
        iWaitActive = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleSessionOpened
// Session is open successfully, then start a query for images
// -----------------------------------------------------------------------------
void CSlideshowPlugin::HandleSessionOpened(CMdESession& /*aSession*/, TInt aError)
    {
    if ( KErrNone != aError )
        {
        // Error occurred when opening session. iMdeSession must be deleted and new
        // session opened if we wish to use MdE.
        delete iMdESession;
        iMdESession = NULL;
        return;
        }
    // The session was opened successfully.
    TRAP(aError, OpenQueryL() );
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::OpenQueryL
// -----------------------------------------------------------------------------
void CSlideshowPlugin::OpenQueryL()
    {
    CMdENamespaceDef& defaultNamespaceDef = iMdESession->GetDefaultNamespaceDefL();
    CMdEObjectDef& imageObjDef = defaultNamespaceDef.GetObjectDefL( MdeConstants::Image::KImageObject );

    // query objects with object definition "Image"
    iQuery = iMdESession->NewObjectQueryL( defaultNamespaceDef, imageObjDef, this );

    iQuery->FindL( KDefaultRandomLoadingNumber );
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleSessionError
// error happened when open the session, close session and end the waiting
// -----------------------------------------------------------------------------
void CSlideshowPlugin::HandleSessionError(CMdESession& /*aSession*/, TInt /*aError*/)
    {
    if ( iMdESession )
        {
        delete iMdESession;
        iMdESession = NULL;
        }
    // error happened when open the session, so end the waiting for MDS session.
    EndWaitForMDS();
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleQueryCompleted
// query completed, load the images
// -----------------------------------------------------------------------------
void CSlideshowPlugin::HandleQueryCompleted(CMdEQuery& aQuery, TInt aError)
    {
    if ( aError == KErrNone )
        {
        LoadImagesToModel( aQuery );
        }
    EndWaitForMDS();
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::HandleQueryCompleted
// part of query completed, load the completed images
// -----------------------------------------------------------------------------
void CSlideshowPlugin::HandleQueryNewResults( CMdEQuery& aQuery,
                                              TInt aFirstNewItemIndex,
                                              TInt aNewItemCount)
    {
    LoadImagesToModel( aQuery, aFirstNewItemIndex, aNewItemCount );
    EndWaitForMDS();
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::LoadImagesToModel
// load the images when query is successfully
// -----------------------------------------------------------------------------
void CSlideshowPlugin::LoadImagesToModel(const CMdEQuery& aQuery,
                                         TInt aFirstNewItemIndex,
                                         TInt aNewItemCount)
    {
    CMdEObjectQuery& query= ( CMdEObjectQuery& ) aQuery;
    // query is completed
    if( aQuery.Count() > 0 )
        {
        // some items were found!
        // Loop through, add filenames
        TInt startIndex = aFirstNewItemIndex;
        TInt nItem = aNewItemCount;
        if ( aFirstNewItemIndex == 0 && aFirstNewItemIndex == aNewItemCount )
            {
            startIndex = 0;
            nItem = aQuery.Count();
            }
        TInt count = 0;
        for ( ; startIndex < nItem; startIndex++)
            {
            // TFileName fileName;
            TBufC<256> fileName;

            const CMdEObject& obj = query.Result(startIndex);
            fileName = obj.Uri();

            // Check that the file exists. If not, it is still OK, if it is
            // on the memory card - it may show up later. Omit files from other
            // drives that do not exist at the time of loading
            // TODO: Can be removed, as slides are loaded on every start
            TBool exists = BaflUtils::FileExists(iEikEnv->FsSession(), fileName);
            TBool isOnMC = SlideshowUtil::IsOnMC(fileName);

            if (!exists)  // && (!isOnMC))
                {
                // Do not add nonexisting files from other than memory card
                continue;
                }
#if 0
            // Check that the file's DRM rights allow it to be displayed (if not
            // missing because not on MMC
            if (!SlideshowUtil::DRMCheck(fileName))
                {
                // No point in adding files that cannot be displayed anyway
                continue;
                }
#endif        
            // Create a slide with the filename and store it in the model
            TRAPD(err, AppendSlideToModelL( fileName, isOnMC ) );
            if ( KErrNone != err )
                {
                // appending error, go on to append next slide
                continue;
                }
            SSPLOGGER_WRITEF(_L("SSP: Slide %d added, file: %S"), count, &fileName);
            count++;
            }
        }
    }

// -----------------------------------------------------------------------------
// CSlideshowPlugin::AppendSlideToModelL
// Add slide to model
// -----------------------------------------------------------------------------
void CSlideshowPlugin::AppendSlideToModelL(TDesC& aFileName, TBool aIsOnMC)
    {
    CSlideshowSlide* pSlide = CSlideshowSlide::NewL(aFileName, aIsOnMC);
    CleanupStack::PushL(pSlide);
    iModel->AppendSlideL(pSlide);
    CleanupStack::Pop(pSlide);
    }
// End Of file.
