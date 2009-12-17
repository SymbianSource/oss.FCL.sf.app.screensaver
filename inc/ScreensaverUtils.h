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
* Description:   Screensaver utilities collection
*
*/


#ifndef C_SCREENSAVERUTILS_H
#define C_SCREENSAVERUTILS_H

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

_LIT(KSCRLogBanner, "Screensaver log start");
_LIT(KSCRLogEnterFn, "SCR: enter --> %S");
_LIT(KSCRLogLeaveFn, "SCR: leave <-- %S");
_LIT(KSCRLogWrite, "SCR: %S");
_LIT(KSCRLogTag, "SCR: ");
_LIT(KSCRLogExit, "Screensaver log end");
_LIT(KSCRLogTimeFormatString, "%H:%T:%S:%*C2");

#define SCRLOGGER_CREATE             {RDebug::Print(KSCRLogBanner);}
#define SCRLOGGER_DELETE             {RDebug::Print(KSCRLogExit);}
#define SCRLOGGER_ENTERFN(a)         {_LIT(temp, a); RDebug::Print(KSCRLogEnterFn, &temp);}
#define SCRLOGGER_LEAVEFN(a)         {_LIT(temp, a); RDebug::Print(KSCRLogLeaveFn, &temp);}
#define SCRLOGGER_WRITE(a)           {_LIT(temp, a); RDebug::Print(KSCRLogWrite, &temp);}
#define SCRLOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KSCRLogTimeFormatString); buffer.Insert(0, temp); buffer.Insert(0, KSCRLogTag); RDebug::Print(buffer); }
#define SCRLOGGER_WRITEF             RDebug::Print

#else

_LIT(KSCRLogDir, "SCR");
_LIT(KSCRLogFile, "SCR.txt");
_LIT8(KSCRLogBanner, "Screensaver log start");
_LIT8(KSCRLogEnterFn, "SCR: -> %S");
_LIT8(KSCRLogLeaveFn, "SCR: <- %S");
_LIT8(KSCRLogExit, "Screensaver log end");
_LIT(KSCRLogTimeFormatString, "%H:%T:%S:%*C2");

#define SCRLOGGER_CREATE             {FCreate();}
#define SCRLOGGER_DELETE             {RFileLogger::Write(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, KSCRLogExit);}
#define SCRLOGGER_ENTERFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, KSCRLogEnterFn, &temp);}
#define SCRLOGGER_LEAVEFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, KSCRLogLeaveFn, &temp);}
#define SCRLOGGER_WRITE(a)           {_LIT(temp, a); RFileLogger::Write(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, temp);}
#define SCRLOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KSCRLogTimeFormatString); buffer.Insert(0, temp); RFileLogger::Write(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, buffer); }
#define SCRLOGGER_WRITEF             FPrint

inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    RFileLogger::WriteFormat(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, aFmt, list);
    }

inline void FPrint(const TDesC& aDes)
    {
    RFileLogger::WriteFormat(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, aDes);
    }

inline void FHex(const TUint8* aPtr, TInt aLen)
    {
    RFileLogger::HexDump(KSCRLogDir, KSCRLogFile, EFileLoggingModeAppend, 0, 0, aPtr, aLen);
    }

inline void FHex(const TDesC8& aDes)
    {
    FHex(aDes.Ptr(), aDes.Length());
    }

inline void FCreate()
    {
    TFileName path(_L("c:\\logs\\"));
    path.Append(KSCRLogDir);
    path.Append(_L("\\"));
    RFs& fs = CEikonEnv::Static()->FsSession();
    fs.MkDirAll(path);
    RFileLogger::WriteFormat(KSCRLogDir, KSCRLogFile, EFileLoggingModeOverwrite, KSCRLogBanner);
    }

#endif // USE_RDEBUG

#else // USE_LOGGER

inline void FPrint(const TRefByValue<const TDesC> /*aFmt*/, ...) { };

#define SCRLOGGER_CREATE
#define SCRLOGGER_DELETE
#define SCRLOGGER_ENTERFN(a)
#define SCRLOGGER_LEAVEFN(a)
#define SCRLOGGER_WRITE(a)
#define SCRLOGGER_WRITEF 1 ? ((void)0) : FPrint
#define SCRLOGGER_WRITE_TIMESTAMP(a)

#endif // USE_LOGGER

#endif // C_SCREENSAVERUTILS_H
