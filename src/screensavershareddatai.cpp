/*
* Copyright (c) 2003-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation for screensaver shared data connections.
*
*/



#include <msvids.h>
#include <MProfileName.h>  // Profile API, not yet included in .h
#include <MProfileTones.h>
#include <TProfileToneSettings.h>
#include <centralrepository.h>
#include <coreapplicationuisdomainpskeys.h>  // KCoreAppUIsUipInd, KLightsVTForcedLightsOn
#include <activeidle2domainpskeys.h>         // KPSUidAiInformation
#include <settingsinternalcrkeys.h>          // KSettingsScreenSaverPeriod
#include <LogsDomainCRKeys.h>                // KLogsNewMissedCalls
#include <ScreensaverInternalCRKeys.h>
#include <AvkonInternalCRKeys.h>             // KAknNewContactsNoteActive
#include <avkondomainpskeys.h>               // KAknKeyguardStatus
#include <ctsydomaincrkeys.h>                // Message (voice) waiting status
#include <ctsydomainpskeys.h>                // KCTsyCallState
#include <startupdomainpskeys.h>             // KPSGlobalSystemState
#include <AknSkinsInternalCRKeys.h>
#include <MProfileEngine.h> // Profile engine API
#include <MProfile.h>
#include <NcnListDomainCRKeys.h>

#include "screensaverappui.h"
#include "screensaverctrlmovingtext.h"
#include "screensaverengine.h"
#include "screensavershareddatai.h"
#include "ScreensaverInternalPSKeys.h"
#include "screensaverutility.h"
#include "screensaverrepositorywatcher.h"
#include "screensaversubscriber.h"

const TInt KTimeoutDefault = 15;        // seconds (also in GS)
const TInt KTimeoutMin = 5;             // minimum timeout in seconds (also in GS)

// Screensaver "On/Off" status values
const TInt KScreensaverOff = 0;
const TInt KScreensaverOn = 1;

// Screensaver activated from idle status values
const TInt KScreensaverNotFromIdle = 0;
const TInt KScreensaverStartedFromIdle = 1;

// Screen saver text type.
// _LIT( KScreenSaverTypeText, "Text" );
// Screensaver "None" (Sleep mode)
// _LIT( KScreenSaverTypeNone, "None" );

// Define security policies for Screensavers property keys
_LIT_SECURITY_POLICY_PASS(KSSPolicyPass);
_LIT_SECURITY_POLICY_C1(KSSPolicyWriteUserData, ECapabilityWriteUserData);
_LIT_SECURITY_POLICY_S0(KSSPolicyWriteSSOnly, 0x100056cf);

//
// CScreensaverSharedDataI
//
// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::NewL
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataI* CScreensaverSharedDataI::NewL()
    {
    CScreensaverSharedDataI* self = new (ELeave) CScreensaverSharedDataI();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(); //self
    return self;
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::~CScreensaverSharedDataI
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataI::~CScreensaverSharedDataI()
    {
    // Release the active profile resources
    InvalidateActiveProfile();

    // Release the profile engine
    if (iProfileEngine)
        {
        // No need to delete pointer, just release resources
        iProfileEngine->Release();
        }
    
    delete iScreensaverRepositoryWatcher;
    delete iScreensaverRepository;
    delete iSettingsRepositoryWatcher;
    delete iSettingsRepository;
    delete iCTsyRepository;
    delete iInboxFolder;
    delete iMsvSession;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::DisplayObjectType
// -----------------------------------------------------------------------------
//
TDisplayObjectType CScreensaverSharedDataI::DisplayObjectType() const
    {
    TInt type = 0;
    
    if (iScreensaverRepository)
        {
        iScreensaverRepository->Get( KScreenSaverObject, type );
        }
    
    return ( TDisplayObjectType )type;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetDisplayObjectType
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetDisplayObjectType( const TDisplayObjectType aType )
    {
    if( iScreensaverRepository )
        {
        iScreensaverRepository->Set(KScreenSaverObject, aType);
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::GetDisplayObjectText
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::GetDisplayObjectText( TDes& aDes ) const
    {
    CEikonEnv::Static()->ReadResource(aDes, R_SCREEN_SAVER_TEXT);
    
    if ( iScreensaverRepository )
        {
        iScreensaverRepository->Get( KScreenSaverText, aDes );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::PluginName
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::GetPluginName( TFileName& aFileName ) const
    {
    if ( iScreensaverRepository )
        {
        iScreensaverRepository->Get( KScreenSaverPluginName, aFileName );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ColorInversionUsed
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::IsInvertedColors() const
    {
    TInt colorInversionUsed(0);
    
    if( iScreensaverRepository )
        {
        iScreensaverRepository->Get( KScreenSaverInvertedColors, colorInversionUsed );
        }

    return ( colorInversionUsed != 0 );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ScreensaverTimeout
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::ScreensaverTimeout() const
    {
    TInt timeout = KTimeoutDefault;

    if (iSettingsRepository)
        {
        iSettingsRepository->Get(KSettingsAutomaticKeyguardTime, timeout);
        }
    // No less than minimum timeout
    // Old backed up minutes 1-4 will cause default timeout after
    // being restored to new sw using seconds
    if (timeout < KTimeoutMin)
        {
        return KTimeoutDefault;
        }

    return timeout;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsNCNActive
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::IsNCNActive() const
    {
    // Setting the flag here is also needed, because Screensaver should not
    // display new calls when Ncn note has been cancelled (even if the missed
    // calls count is still nonzero).
    // Also, when the phone is (re)started, and there are missed calls,
    // Ncn will pop a note, This is the only way Screensaver knows about that.
    
    CRepository* avkonRepository = NULL;
    
    TRAP_IGNORE( avkonRepository = CRepository::NewL( KCRUidAvkon ) );

    TInt value = 0;

    if ( avkonRepository )
        {
        avkonRepository->Get( KAknNewContactsNoteActive, value );
        }

    delete avkonRepository;
    
    return ( value != 0 );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::NewMissedCalls
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::NewMissedCalls() const
    {
    CRepository* logsRepository = NULL;
    
    TRAP_IGNORE( logsRepository = CRepository::NewL( KCRUidLogs ) );
    
    TInt value = 0;
    
    if ( logsRepository )
        {
        logsRepository->Get( KLogsNewMissedCalls, value );
        }
    
    delete logsRepository;
    
    return value;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetSSForcedLightsOn
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetSSForcedLightsOn(TInt aSecs)
    {
    SCRLOGGER_WRITEF(_L("SCR: RequestLights(%d)"), aSecs);

    RProperty::Set( KPSUidCoreApplicationUIs, KLightsSSForcedLightsOn, aSecs );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetScreensaverStatus
// Indicate whether Screensaver is active or not
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetScreensaverStatus( TBool aOn /* = ETrue */ )
    {
    //Check if the value is already updated. If so, setting the value again
    //will cause unnecessary call backs to any subscribers of this property
    if ( aOn )
        {
        RProperty::Set( KPSUidScreenSaver, KScreenSaverOn, KScreensaverOn );
        //else - no need to re-apply the same value
        }
    else //aOn is false
        {
        RProperty::Set( KPSUidScreenSaver, KScreenSaverOn, KScreensaverOff );
        //else - no need to re-apply the same value
        
        //if stop from idle, change the status
        RProperty::Set( KPSUidScreenSaver, KScreenSaverOn, KScreensaverNotFromIdle);
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetSSStartedFromIdleStatus
// Set "Screensaver started from Idle" status
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetSSStartedFromIdleStatus()
    {
    TInt idleState = EPSAiBackground;
    RProperty::Get(KPSUidAiInformation, KActiveIdleState, idleState);

    if (idleState == EPSAiBackground)
        {
        RProperty::Set( KPSUidScreenSaver, KScreenSaverActivatedFromIdle, KScreensaverNotFromIdle );
        }
    else
        {
        RProperty::Set(  KPSUidScreenSaver, KScreenSaverActivatedFromIdle, KScreensaverStartedFromIdle );
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsScreenSaverAllowed
// Set "Screensaver started from Idle" status
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::IsScreenSaverAllowed() const
    {
    TInt value = 0;
    
    RProperty::Get( KPSUidScreenSaver, KScreenSaverAllowScreenSaver, value );
    
    return value;
    }

TInt CScreensaverSharedDataI::ScreensaverPreviewMode() const
    {
    TInt value = 0;
    
    RProperty::Get( KPSUidScreenSaver, KScreenSaverPreviewMode, value );
    
    return value;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetScreensaverPreviewMode
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetScreensaverPreviewMode( const TInt aMode )
    {
    RProperty::Set( KPSUidScreenSaver, KScreenSaverPreviewMode, aMode );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ScreensaverPreviewState
// Get preview state
// -----------------------------------------------------------------------------
//
TScreenSaverPreviewState CScreensaverSharedDataI::ScreensaverPreviewState() const
    {
    TInt state = EScreenSaverPreviewNone;
    RProperty::Get( KPSUidScreenSaver, KScreenSaverPreviewState, state );

    return ( TScreenSaverPreviewState ) state;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SetScreensaverPreviewState
// Report preview state
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::SetScreensaverPreviewState(
    const TScreenSaverPreviewState aState )
    {
    RProperty::Set( KPSUidScreenSaver, KScreenSaverPreviewState, aState );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsKeyguardOn
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::IsKeyguardOn() const
    {
    // Report preview state
    TInt state = 0;
    
    RProperty::Get( KPSUidAvkonDomain, KAknKeyguardStatus, state );
    
    return ( state != 0 );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsUsePowerSaveMode
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsUsePowerSaveMode() const
    {
    TInt state = 0;

    // Check if power save mode has been denied
    RProperty::Get( KPSUidScreenSaver, KScreenSaverNoPowerSaveMode, state );

    return ( state != 1 );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::SystemStartupComplete
// Returns whether system startup has been completed
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::IsSystemStartupComplete()
    {
    if ( iStartupComplete == EFalse )
        {
        TInt startupState = -1;
        RProperty::Get( KPSUidStartup, KPSGlobalSystemState, startupState );

        switch (startupState)
            {
        case ESwStateNormalRfOn:
        case ESwStateNormalRfOff:
        case ESwStateNormalBTSap:
            iStartupComplete = ETrue;
            break;
        default:
            break;
            }
        }
    return iStartupComplete;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::DefaultScreensaverType
// -----------------------------------------------------------------------------
//
TDisplayObjectType CScreensaverSharedDataI::DefaultScreensaverType() const
    {
    return iDefaultType;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::UnreadMessagesNumber
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::UnreadMessagesNumber()
    {
    TInt numberOfUnreadMessages = 0;
    // Check how many unread messages there are in the In-box.
    if( !iInboxFolder )
        {
        TRAPD( err, ConnectToInboxL() );
        if (err != KErrNone)
            {
            return numberOfUnreadMessages;
            }
        }

    const TInt itemCount = iInboxFolder->Count();
    for ( TInt index = 0; index < itemCount; index++ )
        {
        const TMsvEntry entry = ( *iInboxFolder )[index];
        if ( entry.iType.iUid == KUidMsvMessageEntryValue && entry.Unread())
            {
            if( entry.Visible() )
                {
                numberOfUnreadMessages++;
                }
            }
        }
    return numberOfUnreadMessages;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ProfileName
// -----------------------------------------------------------------------------
const TDesC& CScreensaverSharedDataI::ProfileName()
    {
    // Get the active profile name from profile engine, not from
    // shareddata ( may not be up-to-date after phone restart )

    if ( ConnectToActiveProfile() )
        {
        // Ectract profile name
        const MProfileName& profileName = iActiveProfile->ProfileName();

        return ( profileName.Name() );
        }

    return KNullDesC;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ActiveProfile
// -----------------------------------------------------------------------------
TInt CScreensaverSharedDataI::ActiveProfile()
    {
    // Return the current profile Id
    TInt profileId = 0;

    // Connect to profile engine
    if ( ConnectToProfileEngine() )
        {
        profileId = iProfileEngine->ActiveProfileId();
        }

    return profileId;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsHaveNewEmail
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsHaveNewEmail() const
    {
    TInt state = 0;
    // Check the new email state from PubSub
    TInt error = RProperty::Get(
        KPSUidCoreApplicationUIs, KCoreAppUIsNewEmailStatus, state);

    SCRLOGGER_WRITEF(_L("SCR: QueryNewEmail state=%d error=%d"), state, error);

    if ( ( error == KErrNone ) && ( state == ECoreAppUIsNewEmail ) )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsHaveNewVoicemail
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsHaveNewVoicemail() const
    {
    // Show the indicator, if either line has messages waiting
    if ( ( VoiceMailCount( KCtsyMessageWaitingVoiceMailCount ) > 0 ) 
        || ( VoiceMailCount( KCtsyMessageWaitingAuxLineCount ) > 0 ) )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsHaveNewInstantMessages
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsHaveNewInstantMessages() const
    {
    TInt showValue = 0;
    TInt error = RProperty::Get(
        KPSUidCoreApplicationUIs, KCoreAppUIsUipInd, showValue);

    SCRLOGGER_WRITEF(_L("SCR: QueryNewInstantMessages state=%d error=%d"),
                     showValue, error);

    return ( ( error == KErrNone ) && ( showValue == ECoreAppUIsShow ) );
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsSilentMode
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsSilentMode()
    {
    // Connect to active profile, and return silent mode status
    if (ConnectToActiveProfile())
        {
        return iActiveProfile->IsSilent();
        }

    return EFalse;
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::IsVibraMode
// -----------------------------------------------------------------------------
TBool CScreensaverSharedDataI::IsVibraMode()
    {
    // Get the active profile name from profile engine, not from
    // shareddata (may not be up-to-date after phone restart)
    if (ConnectToActiveProfile())
        {
        // Extract profile vibra setting
         return iActiveProfile->ProfileTones().ToneSettings().iVibratingAlert;
        }

    return EFalse;
    }

// ---------------------------------------------------------------------------
// CScreensaverSharedDataI::IsOngoingCall()
// ---------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::IsOngoingCall()
    {
    TInt state = EPSCTsyCallStateUninitialized;
    // Check the call state from PubSub
    RProperty::Get( KPSUidCtsyCallInformation, KCTsyCallState, state );
    
    return ( state != EPSCTsyCallStateNone && 
             state != EPSCTsyCallStateUninitialized );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::HandleSessionEventL
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/)
    {
    switch (aEvent)
        {
        case EMsvEntriesChanged:
            AppUi()->ScreensaverView()->UpdateAndRefresh();
            break;
        case EMsvCloseSession:
        case EMsvServerTerminated:
            delete iInboxFolder;
            iInboxFolder = NULL;
            delete iMsvSession;
            iMsvSession = NULL;
            break;
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::HandleEntryEventL
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::HandleEntryEventL(TMsvEntryEvent /*aEvent */, TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/)
    {
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ScreensaverRepository
// -----------------------------------------------------------------------------
//
CRepository* CScreensaverSharedDataI::ScreensaverRepository() const
    {
    return iScreensaverRepository;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::CScreensaverSharedDataI
// -----------------------------------------------------------------------------
//
CScreensaverSharedDataI::CScreensaverSharedDataI()
    {
    iStartupComplete = EFalse;
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConstructL
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ConstructL()
    {
    GetDefaultType();
    ConnectToSSCRL();
    ConnectToPslnSettingCRL();
    ConnectToCommonTsyCRL();

    // Define and publish "Screensaver is on" property
    // Read: pass, Write: Screensaver only
    DefineScreensaverProperties( KScreenSaverOn, KSSPolicyWriteSSOnly );
    
    // Define and publish "Screensaver activated from idle" property
    // Read: pass, Write: Screensaver only
    DefineScreensaverProperties( KScreenSaverActivatedFromIdle, KSSPolicyWriteSSOnly );
    
    // Define and attach "allow screensaver property"
    // Read: Readuserdata, Write: writeuserdata
    DefineScreensaverProperties( KScreenSaverAllowScreenSaver, KSSPolicyWriteUserData );

    // Define and attach "screensaver preview property"
    // Read: Readuserdata, Write: writeuserdata
    DefineScreensaverProperties( KScreenSaverPreviewMode, KSSPolicyWriteUserData );
    
    // Define and attach "screensaver preview state property"
    // Read: Readuserdata, Write: writeuserdata
    DefineScreensaverProperties( KScreenSaverPreviewState, KSSPolicyWriteUserData );

    // Define and initialize "current plugin settings changed" property
    // Read: pass, Write: pass
    DefineScreensaverProperties( KScreenSaverPluginSettingsChanged, KSSPolicyPass );

    // Define and initialize "no power save mode" property
    // Read: pass, Write: pass
    DefineScreensaverProperties( KScreenSaverNoPowerSaveMode, KSSPolicyPass );

    DefineScreensaverProperties( KScreenSaverActivate, KSSPolicyWriteUserData );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConnectToSSCR
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ConnectToSSCRL()
    {
    iScreensaverRepository = CRepository::NewL( KCRUidScreenSaver );
    
    if( iScreensaverRepository )
    iScreensaverRepositoryWatcher = CScreensaverRepositoryWatcher::NewL(
        KCRUidScreenSaver,
        TCallBack( HandleDisplayObjChanged, this ),
        iScreensaverRepository );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConnectToPslnSettingCRL
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ConnectToPslnSettingCRL()
    {
    TRAPD(ret, iSettingsRepository = CRepository::NewL(KCRUidSecuritySettings));
    
    if( ret == KErrNone )
        {
        iSettingsRepositoryWatcher = CScreensaverRepositoryWatcher::NewL(
            KCRUidPersonalizationSettings,
            KSettingsAutomaticKeyguardTime,
            CCenRepNotifyHandler::EIntKey,
            TCallBack(HandleTimeoutChanged, this),
            iSettingsRepository);
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConnectToCommonTsyCRL
// Connects to common tsy central repository
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ConnectToCommonTsyCRL()
    {
    iCTsyRepository = CRepository::NewL( KCRUidCtsyMessageWaitingIndicator );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConnectToInboxL
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ConnectToInboxL()
    {
    if (!iMsvSession)
        {
        iMsvSession = CMsvSession::OpenSyncL( *this );
        }
    iInboxFolder = iMsvSession->GetEntryL( KMsvGlobalInBoxIndexEntryId );

    TMsvSelectionOrdering ordering = iInboxFolder->SortType();
    ordering.SetShowInvisibleEntries( EFalse );
    iInboxFolder->SetSortTypeL( ordering );

    iInboxFolder->AddObserverL( *this );
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ConnectToProfileEngine
// Connect to Profile engine. Returns ETrue if successfully connected
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::ConnectToProfileEngine()
    {
    // Check if already connected
    if ( iProfileEngine )
        {
        // Done
        return ETrue;
        }
    // The member variable is also used to indicate successful
    // connection, hence a pointer instead of just RSystemAgent
    TRAPD( err, ( iProfileEngine = CreateProfileEngineL() ) );

    if ( err != KErrNone )
        {
        // Something went wrong on allocation
        iProfileEngine = NULL;
        return EFalse;
        }

    // Successfully connected
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ClearPreviewFlag
// Connect to current active Profile. Returns ETrue if successfully connected
// -----------------------------------------------------------------------------
//
TBool CScreensaverSharedDataI::ConnectToActiveProfile()
    {
    InvalidateActiveProfile();
    
    if ( ConnectToProfileEngine() )
        {
        TRAPD( err, ( iActiveProfile = iProfileEngine->ActiveProfileL() ) );
        if ( ( err == KErrNone ) && ( iActiveProfile ) )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::DefineScreensaverProperties
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::DefineScreensaverProperties( 
    TUint32 aKey, const TSecurityPolicy& aWritePolicy )
    {
    RProperty::Define(
            KPSUidScreenSaver,
            aKey,
            RProperty::EInt,
            KSSPolicyPass,
            aWritePolicy );
    
    RProperty::Set( KPSUidScreenSaver, aKey, 0);
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::HandleTimeoutChanged
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::HandleTimeoutChanged(TAny* aPtr)
    {
    STATIC_CAST(CScreensaverSharedDataI*, aPtr)->ResetInactivityTimeout();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::ResetInactivityTimeout
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::ResetInactivityTimeout()
    {
    AppUi()->Model().ResetInactivityTimeout();
    }


// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::HandleDisplayObjChanged
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::HandleDisplayObjChanged(TAny* aPtr)
    {
    CScreensaverSharedDataI* self = STATIC_CAST(CScreensaverSharedDataI*, aPtr);
    
    self->AppUi()->ScreensaverView()->SetDisplayObject( self->DisplayObjectType() );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::GetDefaultType
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::GetDefaultType()
    {
/*
default type is hardcoded, so commented out below code.
Remove the comment when default type is read from CenRep.
    //Store the default screensaver for this device
    TBuf<16> screensaverName;
    
    TInt error = GetDefaultTypeString( screensaverName );
    
    if ( KErrNone == error )
        {
        if ( screensaverName.CompareF( KScreenSaverTypeNone ) == 0 )
            {
            iDefaultType = EDisplayNone;
            }
        else if ( screensaverName.CompareF( KScreenSaverTypeText ) == 0 )
            {
            iDefaultType = EDisplayText;
            }
        else //go back to the "old" option, ie - Date type
            {
            iDefaultType = EDisplayTime;
            }
        }
*/  
      
#ifdef USE_DATE_AND_TEXT
    iDefaultType = EDisplayTime;
#else
    iDefaultType = EDisplayNone;    
#endif
    
    
    
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::GetDefaultTypeString
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::GetDefaultTypeString( TDes& DefaultScreenSaver )
    {
    CRepository* skinsRepository = NULL;
    TRAP_IGNORE( skinsRepository = CRepository::NewL( KCRUidPersonalisation ) );
    
    TInt error = skinsRepository->Get( KPslnSystemDefaultScreenSaver,
        DefaultScreenSaver );
    
    delete skinsRepository;
    
    return error;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::VoiceMailCount
// -----------------------------------------------------------------------------
//
TInt CScreensaverSharedDataI::VoiceMailCount( const TUint32 aKey ) const
    {
    TInt line(0);
    TInt error = KErrNone;
    
    error = iCTsyRepository->Get( aKey, line );
    
    if( error != KErrNone )
        {
        line = 0;
        }
    
    return line;
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::InvalidateActiveProfile
// Invalidate the cached active profile, so that next call to ConnectToActiveProfile
// will use the newest profile data (in case it has changed)
// -----------------------------------------------------------------------------
//
void CScreensaverSharedDataI::InvalidateActiveProfile()
    {
    if (iActiveProfile)
        {
        iActiveProfile->Release();
        iActiveProfile = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CScreensaverSharedDataI::AppUi
// -----------------------------------------------------------------------------
//
CScreensaverAppUi *CScreensaverSharedDataI::AppUi() const
    {
    return STATIC_CAST( CScreensaverAppUi*, CCoeEnv::Static()->AppUi() );
    }
// End of file.
