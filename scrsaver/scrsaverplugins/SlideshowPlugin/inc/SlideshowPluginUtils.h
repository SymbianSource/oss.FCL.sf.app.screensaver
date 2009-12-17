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
* Description:   SlideshowPlugin utilities collection
*
*/



#ifndef C_SLIDESHOWPLUGINUTILS_H
#define C_SLIDESHOWPLUGINUTILS_H

//  INCLUDES
#include <e32base.h>
#include <cenrepnotifyhandler.h>
#include <e32property.h>

#include "SlideshowSlide.h"

// FORWARD DECLARATIONS
class CDRMHelper;
class CCoeEnv;
class CRepository;

// CLASS DECLARATION

/**
* Central Repository Change watcher
* @since 3.2
* @internal
*/    
class CRepositoryWatcher : public CBase, public MCenRepNotifyHandlerCallback
    {
public:
    static CRepositoryWatcher* NewL(
        const TUid aUid,
        const TUint32 aKey,
        CCenRepNotifyHandler::TCenRepKeyType aKeyType,
        TCallBack aCallBack,
        CRepository* aRepository);

    static CRepositoryWatcher* NewL(
        const TUid aUid,
        TCallBack aCallBack,
        CRepository* aRepository);

    ~CRepositoryWatcher();

    TUint32 ChangedKey();

public: // from MCenRepNotifyHandlerCallback
    void HandleNotifyInt(TUint32 aKey, TInt aNewValue);
    void HandleNotifyString(TUint32 aKey, const TDesC16& aNewValue);
    void HandleNotifyGeneric(TUint32 aKey);
    void HandleNotifyError(
        TUint32 aKey, TInt aError, CCenRepNotifyHandler* aHandler);

private:
    CRepositoryWatcher(
        const TUid aUid,
        const TUint32 aKey,
        TCallBack aCallBack,
        CRepository* aRepository);

    void ConstructL(CCenRepNotifyHandler::TCenRepKeyType aKeyType);

    void ConstructL();

private:
    TUid iUid;
    TUint32 iKey;
    TUint32 iChangedKey;
    TCallBack iCallBack;
    CRepository* iRepository;
    CCenRepNotifyHandler* iNotifyHandler;
};    


/**
* Publish & Subscribe change subscriber
* @since 3.2
* @internal
*/    
class CPSSubscriber : public CActive
    {
public:
    CPSSubscriber(TCallBack aCallBack, RProperty& aProperty);
    ~CPSSubscriber();
    
public: // New functions
    void SubscribeL();
    void StopSubscribe();

private: // from CActive
    void RunL();
    void DoCancel();

private:
    TCallBack   iCallBack;
    RProperty&  iProperty;
};


// ========== UTILITIES CLASS =============================

class SlideshowUtil
    {
public:
    // Checks if memory card is present
    static TBool IsMCPresent();

    // Checks if the given file is on memory card
    static TBool IsOnMC(TFileName aFile);

    // Checks that the file has enough DRM rights to be displayed
    static TBool DRMCheck(CDRMHelper *aDRMHelper, TDesC& aFileName);

    // Checks that the slide has enough DRM rights to be displayed
    static TBool DRMCheck(CDRMHelper *aDRMHelper, CSlideshowSlide* aSlide);

    // Consumes the slide's DRM rights
    static void DRMConsume(CDRMHelper* aDRMHelper, CSlideshowSlide* aSlide);
    
private:
    /**
     * Static class can not be instantiated.
     */
    SlideshowUtil();
    //lint -esym(1526, SlideshowUtil::SlideshowUtil) Intentionally undefined

    /**
     * Static class can not be instantiated.
     */
    ~SlideshowUtil();
    };


// ========== LOGGING MACROS ===============================

#ifdef USE_LOGGER

#include <e32std.h>
#include <e32def.h>
#include <eikenv.h>
#include <flogger.h>


/* logging macros usage:

// set logging on in mmp file...
MACRO USE_LOGGER
#ifdef WINS
MACRO USE_RDEBUG
#endif


void CTestAppUi::ConstructL()
{
// creates log directory and log file on app startup.
// prints starting banner...
SCRLOGGER_CREATE;

// entering to function...
SCRLOGGER_ENTERFN("ConstructL()");

TInt number = 42;
TFileName file;
file = "something" ....

// print variable number of parameters...
// Note: use _L macro for format string!
SCRLOGGER_WRITEF(_L("SCR: number = %d, file = %S"), number, &file);

// print just a string (no _L macro here!)
SCRLOGGER_WRITE("Hello world!");

// leave from function...
SCRLOGGER_LEAVEFN("ConstructL()");
}

CTestAppUi::~CTestAppUi()
{
// print app exit banner when application exits.
SCRLOGGER_DELETE;
}

*/

#ifdef USE_RDEBUG

_LIT(KSSPLogBanner, "SlideshowPlugin log start");
_LIT(KSSPLogEnterFn, "SSP: enter --> %S");
_LIT(KSSPLogLeaveFn, "SSP: leave <-- %S");
_LIT(KSSPLogWrite, "SSP: %S");
_LIT(KSSPLogTag, "SSP: ");
_LIT(KSSPLogExit, "SlideshowPlugin log end");
_LIT(KSSPLogTimeFormatString, "%H:%T:%S:%*C2");

#define SSPLOGGER_CREATE             {RDebug::Print(KSSPLogBanner);}
#define SSPLOGGER_DELETE             {RDebug::Print(KSSPLogExit);}
#define SSPLOGGER_ENTERFN(a)         {_LIT(temp, a); RDebug::Print(KSSPLogEnterFn, &temp);}
#define SSPLOGGER_LEAVEFN(a)         {_LIT(temp, a); RDebug::Print(KSSPLogLeaveFn, &temp);}
#define SSPLOGGER_WRITE(a)           {_LIT(temp, a); RDebug::Print(KSSPLogWrite, &temp);}
#define SSPLOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KSSPLogTimeFormatString); buffer.Insert(0, temp); buffer.Insert(0, KSSPLogTag); RDebug::Print(buffer); }
#define SSPLOGGER_WRITEF             RDebug::Print

#else

_LIT(KSSPLogDir, "SSP");
_LIT(KSSPLogFile, "SSP.txt");
_LIT8(KSSPLogBanner, "SlideshowPlugin log start");
_LIT8(KSSPLogEnterFn, "SSP: -> %S");
_LIT8(KSSPLogLeaveFn, "SSP: <- %S");
_LIT8(KSSPLogExit, "SlideshowPlugin log end");
_LIT(KSSPLogTimeFormatString, "%H:%T:%S:%*C2");

#define SSPLOGGER_CREATE             {FCreate();}
#define SSPLOGGER_DELETE             {RFileLogger::Write(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, KSSPLogExit);}
#define SSPLOGGER_ENTERFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, KSSPLogEnterFn, &temp);}
#define SSPLOGGER_LEAVEFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, KSSPLogLeaveFn, &temp);}
#define SSPLOGGER_WRITE(a)           {_LIT(temp, a); RFileLogger::Write(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, temp);}
#define SSPLOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KSSPLogTimeFormatString); buffer.Insert(0, temp); RFileLogger::Write(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, buffer); }
#define SSPLOGGER_WRITEF             FPrint

inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    RFileLogger::WriteFormat(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, aFmt, list);
    }

inline void FPrint(const TDesC& aDes)
    {
    RFileLogger::WriteFormat(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, aDes);
    }

inline void FHex(const TUint8* aPtr, TInt aLen)
    {
    RFileLogger::HexDump(KSSPLogDir, KSSPLogFile, EFileLoggingModeAppend, 0, 0, aPtr, aLen);
    }

inline void FHex(const TDesC8& aDes)
    {
    FHex(aDes.Ptr(), aDes.Length());
    }

inline void FCreate()
    {
    TFileName path(_L("c:\\logs\\"));
    path.Append(KSSPLogDir);
    path.Append(_L("\\"));
    RFs& fs = CEikonEnv::Static()->FsSession();
    fs.MkDirAll(path);
    RFileLogger::WriteFormat(KSSPLogDir, KSSPLogFile, EFileLoggingModeOverwrite, KSSPLogBanner);
    }

#endif // USE_RDEBUG

#else // USE_LOGGER

inline void FPrint(const TRefByValue<const TDesC> /*aFmt*/, ...) { };

#define SSPLOGGER_CREATE
#define SSPLOGGER_DELETE
#define SSPLOGGER_ENTERFN(a)
#define SSPLOGGER_LEAVEFN(a)
#define SSPLOGGER_WRITE(a)
#define SSPLOGGER_WRITEF 1 ? ((void)0) : FPrint
#define SSPLOGGER_WRITE_TIMESTAMP(a)

#endif // USE_LOGGER

#endif // C_SLIDESHOWPLUGINUTILS_H
