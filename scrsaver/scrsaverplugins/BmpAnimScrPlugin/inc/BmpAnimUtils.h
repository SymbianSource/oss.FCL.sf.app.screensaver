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
* Description:   BmpAnimScrPlugin utilities collection
*
*/



#ifndef BMPANIMUTILS_H
#define BMPANIMUTILS_H

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
BMALOGGER_CREATE;

// entering to function...
BMALOGGER_ENTERFN("ConstructL()");

TInt number = 42;
TFileName file;
file = "something" ....

// print variable number of parameters...
// Note: use _L macro for format string!
BMALOGGER_WRITEF(_L("BMA: number = %d, file = %S"), number, &file);

// print just a string (no _L macro here!)
BMALOGGER_WRITE("Hello world!");

// leave from function...
BMALOGGER_LEAVEFN("ConstructL()");
}

CTestAppUi::~CTestAppUi()
{
// print app exit banner when application exits.
BMALOGGER_DELETE;
}

*/

#ifdef USE_RDEBUG

_LIT(KBMALogBanner, "BmpAnim log start");
_LIT(KBMALogEnterFn, "BMA: enter --> %S");
_LIT(KBMALogLeaveFn, "BMA: leave <-- %S");
_LIT(KBMALogWrite, "BMA: %S");
_LIT(KBMALogTag, "BMA: ");
_LIT(KBMALogExit, "BmpAnim log end");
_LIT(KBMALogTimeFormatString, "%H:%T:%S:%*C2");

#define BMALOGGER_CREATE             {RDebug::Print(KBMALogBanner);}
#define BMALOGGER_DELETE             {RDebug::Print(KBMALogExit);}
#define BMALOGGER_ENTERFN(a)         {_LIT(temp, a); RDebug::Print(KBMALogEnterFn, &temp);}
#define BMALOGGER_LEAVEFN(a)         {_LIT(temp, a); RDebug::Print(KBMALogLeaveFn, &temp);}
#define BMALOGGER_WRITE(a)           {_LIT(temp, a); RDebug::Print(KBMALogWrite, &temp);}
#define BMALOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KBMALogTimeFormatString); buffer.Insert(0, temp); buffer.Insert(0, KBMALogTag); RDebug::Print(buffer); }
#define BMALOGGER_WRITEF             RDebug::Print

#else // !USE_RDEBUG

_LIT(KBMALogDir, "BMA");
_LIT(KBMALogFile, "BMA.txt");
_LIT8(KBMALogBanner, "BmpAnim log start");
_LIT8(KBMALogEnterFn, "BMA: -> %S");
_LIT8(KBMALogLeaveFn, "BMA: <- %S");
_LIT8(KBMALogExit, "BmpAnim log end");
_LIT(KBMALogTimeFormatString, "%H:%T:%S:%*C2");

#define BMALOGGER_CREATE             {FCreate();}
#define BMALOGGER_DELETE             {RFileLogger::Write(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, KBMALogExit);}
#define BMALOGGER_ENTERFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, KBMALogEnterFn, &temp);}
#define BMALOGGER_LEAVEFN(a)         {_LIT8(temp, a); RFileLogger::WriteFormat(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, KBMALogLeaveFn, &temp);}
#define BMALOGGER_WRITE(a)           {_LIT(temp, a); RFileLogger::Write(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, temp);}
#define BMALOGGER_WRITE_TIMESTAMP(a) {_LIT(temp, a); TTime time; time.HomeTime(); TBuf<256> buffer; time.FormatL(buffer, KBMALogTimeFormatString); buffer.Insert(0, temp); RFileLogger::Write(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, buffer); }
#define BMALOGGER_WRITEF             FPrint

inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    RFileLogger::WriteFormat(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, aFmt, list);
    }

inline void FPrint(const TDesC& aDes)
    {
    RFileLogger::WriteFormat(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, aDes);
    }

inline void FHex(const TUint8* aPtr, TInt aLen)
    {
    RFileLogger::HexDump(KBMALogDir, KBMALogFile, EFileLoggingModeAppend, 0, 0, aPtr, aLen);
    }

inline void FHex(const TDesC8& aDes)
    {
    FHex(aDes.Ptr(), aDes.Length());
    }

inline void FCreate()
    {
#ifdef RD_PF_SEC_APPARC
    // Use screensaver private dir
    TFileName path(_L("c:\\private\\100056cf\\"));
#else
    TFileName path(_L("c:\\logs\\"));
#endif
    path.Append(KBMALogDir);
    path.Append(_L("\\"));
    RFs& fs = CEikonEnv::Static()->FsSession();
    fs.MkDirAll(path);
    RFileLogger::WriteFormat(KBMALogDir, KBMALogFile, EFileLoggingModeOverwrite, KBMALogBanner);
    }

#endif // !USE_RDEBUG

#else // !USE_LOGGER

inline void FPrint(const TRefByValue<const TDesC> /*aFmt*/, ...) { };

#define BMALOGGER_CREATE
#define BMALOGGER_DELETE
#define BMALOGGER_ENTERFN(a)
#define BMALOGGER_LEAVEFN(a)
#define BMALOGGER_WRITE(a)
#define BMALOGGER_WRITEF 1 ? ((void)0) : FPrint
#define BMALOGGER_WRITE_TIMESTAMP(a)

#endif // USE_LOGGER

#endif // BMPANIMUTILS_H

