/* Coding: ANSI */
#ifndef _TIGER_LOG_H
#define _TIGER_LOG_H

#include <stdarg.h>
#include "tiger_type.h"

namespace tiger{

class LoggerBase{
public:
    /* log type */
    enum{
        kLogger_Stdio,
        kLogger_File,
        kLogger_Socket,
        kLogger_Invalid
    };
    /* log level */
    enum{
        kLogger_Level_Error, /* highest  */
        kLogger_Level_Debug,
        kLogger_Level_Warn,
        kLogger_Level_Info, /* lowest */
        kLogger_Level_Invalid,
    };
    /* default log level */
    enum{
        kLogger_Level_Default = kLogger_Level_Warn,
    };
    /* log something */
    virtual bool Log(s32 level,char* fmt,...);
    virtual bool D(char* fmt,...){return true;}
    virtual bool I(char* fmt,...){return true;}
    virtual bool W(char* fmt,...){return true;}
    virtual bool E(char* fmt,...){return true;}
    s32 GetLevel(){return m_level;}
    void SetLevel(s32 level){m_level = level;}
    LoggerBase(){m_kind = kLogger_Invalid;m_level = kLogger_Level_Invalid;}
    LoggerBase(s32 kind){m_kind=kind;m_level=kLogger_Level_Default;}
    void SetKind(s32 kind){m_kind = kind;}
    static char* GetLevelString(s32 level){
        return LevelStrings[level];
    }
private:
    s32 m_kind;
    s32 m_level;
    
    static char* LevelStrings[];
};

class LoggerStdio:public LoggerBase{
public:
    LoggerStdio():LoggerBase(kLogger_Stdio){}
    bool Log(s32 level, char* fmt,...);
    bool D(char* fmt,...);
    bool I(char* fmt,...);
    bool W(char* fmt,...);
    bool E(char* fmt,...);
private:
    void LogHeader(s32 level);
    void LogEnder();
    bool vlog(s32 level,char* fmt,va_list args);
};

class LoggerFile:public LoggerBase{
public:
    LoggerFile();
    LoggerFile(char* filename);
    bool Log(s32 level, char* fmt,...);
    bool D(char* fmt,...);
    bool I(char* fmt,...);
    bool W(char* fmt,...);
    bool E(char* fmt,...);
    ~LoggerFile();
private:
    FILE* m_file;
    void LogHeader(s32 level);
    void LogEnder();
    bool vlog(s32 level,char* fmt,va_list args);
};

}/// namespace tiger

#endif
