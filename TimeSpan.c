/**********************************************************************
* 版权所有 (C)2015, Zhou Zhaoxiong。
*
* 文件名称：TimeSpan.c
* 文件标识：无
* 内容摘要：判断当前时间是否在配置的时间的范围内
* 其它说明：无
* 当前版本：V1.0
* 作    者：Zhou Zhaoxiong
* 完成日期：20150617
*
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 重定义数据类型
typedef signed   int        INT32;
typedef signed   char       INT8;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
typedef unsigned int        UINT32;
typedef long     int        LONG;

// 时间结构体
typedef struct
{
    UINT8   second;     /* 0-59 */
    UINT8   minute;     /* 0-59 */
    UINT8   hour;       /* 0-23 */
    UINT8   day;        /* 1-31 */
    UINT8   month;      /* 1-12 */
    UINT16  year;       /* 1994-2099 */
    UINT8   week;       /* 1-7 */
    UINT8   Count10ms;  /* 0-99 */
} ClockStruc;

// 函数声明
INT32 GetTimeFromStr(ClockStruc *ptTime, INT8 *pszTimeBuf);
void CurrentTime(ClockStruc *ptTime);
INT32 IsInTimeSpan(ClockStruc *pTimeNow, ClockStruc *pBeginTime, ClockStruc *pEndTime);
void GetCompletePath(UINT8 *pszConfigFileName, UINT8 *pszWholePath);
void GetStringContentValue(FILE *fp, UINT8 *pszSectionName, UINT8 *pszKeyName, UINT8 *pszOutput, UINT32 iOutputLen);
void GetConfigFileStringValue(UINT8 *pszSectionName, UINT8 *pszKeyName, UINT8 *pDefaultVal, UINT8 *pszOutput, UINT32 iOutputLen, UINT8 *pszConfigFileName);
INT32 IsRightStr(UINT8 *pszStr);


/**********************************************************************
* 功能描述：主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期         版本号        修改人           修改内容
* -------------------------------------------------------------------
* 20150617        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 main()
{
    INT8  szTimeBuf[50]     = {0};
    INT32 iRetVal           = 0;

    ClockStruc tBeginTime  = {0};      // 开始时间
    ClockStruc tEndTime    = {0};      // 结束时间
    ClockStruc tTimeNow    = {0};      // 当前时间

    // 获取开始时间, 输入格式: HH:MM
    GetConfigFileStringValue("TIMEINFO", "BeginTime", "", szTimeBuf, sizeof(szTimeBuf), "Config.ini");
    if (strlen(szTimeBuf) != 5)   // 时间串的长度必须为5位
    {
        printf("The length of BeginTime is not 5, please check!\n");
        return -1;
    }
    if (IsRightStr(szTimeBuf) != 0)   // 必须是包含:的数字串, 如果含有其它字符, 则不再执行后续流程
    {
        printf("BeginTime is not a right format string, please check!\n");
        return -1;
    }

    iRetVal = GetTimeFromStr(&tBeginTime, szTimeBuf);
    if (iRetVal == -1)
    {
        return -1;
    }

    // 获取结束时间, 输入格式: HH:MM
    GetConfigFileStringValue("TIMEINFO", "EndTime", "", szTimeBuf, sizeof(szTimeBuf), "Config.ini");
    if (strlen(szTimeBuf) != 5)   // 时间串的长度必须为5位
    {
        printf("The length of EndTime is not 5, please check!\n");
        return -1;
    }
    if (IsRightStr(szTimeBuf) != 0)   // 必须是包含:的数字串, 如果含有其它字符, 则不再执行后续流程
    {
        printf("EndTime is not a right format string, please check!\n");
        return -1;
    }

    iRetVal = GetTimeFromStr(&tEndTime, szTimeBuf);
    if (iRetVal == -1)
    {
        return -1;
    }

    printf("BeginTime: hour=%d, minute=%d; EndTime: hour=%d, minute=%d\n", tBeginTime.hour, tBeginTime.minute,
               tEndTime.hour, tEndTime.minute);

    // 如果开始时间和结束时间一样, 则不再执行后续流程
    if (tBeginTime.hour == tEndTime.hour && tBeginTime.minute == tEndTime.minute)
    {
        printf("BeginTime is the same as EndTime, please check!\n");
        return -1;
    }

    // 判断当前时间是否在配置的时间范围内
    CurrentTime(&tTimeNow);
    iRetVal = IsInTimeSpan(&tTimeNow, &tBeginTime, &tEndTime);
    if (iRetVal == 0)
    {
        printf("当前时间%d点%d分 在 %d点%d分和%d点%d分范围内\n", tTimeNow.hour, tTimeNow.minute, tBeginTime.hour, 
            tBeginTime.minute, tEndTime.hour, tEndTime.minute);
    }
    else
    {
        printf("当前时间%d点%d分 不在 %d点%d分和%d点%d分范围内\n", tTimeNow.hour, tTimeNow.minute, tBeginTime.hour, 
            tBeginTime.minute, tEndTime.hour, tEndTime.minute);
    }

    return 0;                  // main函数执行成功返回0
}


/**********************************************************************
* 功能描述：从时间字符串中获取小时和分
* 输入参数：ptTime-时间结构体
            pszTimeBuf-时间字符串
* 输出参数：无
* 返 回 值：0-成功 -1-失败
* 其它说明：无
* 修改日期        版本号         修改人           修改内容
* -------------------------------------------------------------------
* 20150617        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 GetTimeFromStr(ClockStruc *ptTime, INT8 *pszTimeBuf)
{
    INT8 *pFlag         = NULL;
    INT8  szTimeBuf[10] = {0};

    if (pszTimeBuf == NULL)    // 判断输入参数是否为空
    {
        printf("GetTimeFromStr: input parameter is NULL!\n");
        return -1;
    }

    strncpy(szTimeBuf, pszTimeBuf, sizeof(szTimeBuf)-1);

    pFlag = strchr(szTimeBuf, ':');        // 获取:所在的位置
    if (pFlag == NULL)
    {
        printf("GetTimeFromStr: the format of time in config file is wrong, please check!\n");
        return -1;
    }
    else
    {
        pFlag ++;
        ptTime->hour   = atoi(szTimeBuf);
        ptTime->minute = atoi(pFlag);
    }

    // 小时的范围是0到23, 分的范围是0到59
    if (ptTime->hour > 23 || ptTime->minute > 59)
    {
        printf("GetTimeFromStr: hour or minute in config file is out of range, please check!\n");
        return -1;
    }

    return 0;
}


/**********************************************************************
* 功能描述：当前时间
* 输入参数：ptTime-时间结构体
* 输出参数：ptTime-时间结构体
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号         修改人          修改内容
* -------------------------------------------------------------------
* 20150617        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
void CurrentTime(ClockStruc *ptTime)
{
    LONG    dt           = 0;
    struct  tm      *tm1 = NULL;
    struct  timeval  tp  = {0};

    // get real clock from system
    gettimeofday(&tp, NULL);
    dt  = tp.tv_sec;
    tm1 = localtime(&dt);
    ptTime->Count10ms = tp.tv_usec / 10000;
    ptTime->year      = (UINT16)(tm1->tm_year + 1900);
    ptTime->month     = (UINT8)tm1->tm_mon + 1;
    ptTime->day       = (UINT8)tm1->tm_mday;
    ptTime->hour      = (UINT8)tm1->tm_hour;
    ptTime->minute    = (UINT8)tm1->tm_min;
    ptTime->second    = (UINT8)tm1->tm_sec;
    ptTime->week      = (UINT8)tm1->tm_wday;
    if (ptTime->week == 0)   // Sunday
    {
        ptTime->week = 7;
    }
}


/**********************************************************************
* 功能描述：判断当前时间是否在配置的时间范围内
* 输入参数：pTime-时间结构体
* 输出参数：pTime-时间结构体
* 返 回 值：0-在范围内   -1-不在范围内
* 其它说明：无
* 修改日期         版本号        修改人           修改内容
* -------------------------------------------------------------------
* 20150617        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 IsInTimeSpan(ClockStruc *pTimeNow, ClockStruc *pBeginTime, ClockStruc *pEndTime)
{
    UINT8 iBegLessThanEnd = 0;       // 1-配置的开始时间小于结束时间  0-配置的开始时间大于结束时间

    if (pBeginTime->hour < pEndTime->hour || (pBeginTime->hour == pEndTime->hour && pBeginTime->minute <= pEndTime->minute))
    {
        iBegLessThanEnd = 1;
    }
    else
    {
        iBegLessThanEnd = 0;
    }

    if (iBegLessThanEnd)   // 开始时间小于结束时间
    {
         if ((pTimeNow->hour > pBeginTime->hour || (pTimeNow->hour == pBeginTime->hour && pTimeNow->minute >= pBeginTime->minute))
            && (pTimeNow->hour < pEndTime->hour || (pTimeNow->hour == pEndTime->hour && pTimeNow->minute <= pEndTime->minute)))
         {
             return 0;
         }
         else
         {
             return -1;
         }
    }
    else   // 开始时间大于结束时间, 跨天的情况
    {
         if ((pTimeNow->hour > pBeginTime->hour || (pTimeNow->hour == pBeginTime->hour && pTimeNow->minute >= pBeginTime->minute))
            || (pTimeNow->hour < pEndTime->hour || (pTimeNow->hour == pEndTime->hour && pTimeNow->minute <= pEndTime->minute)))
         {
             return 0;
         }
         else
         {
             return -1;
         }
    }
}


/**********************************************************************
* 功能描述： 获取配置文件完整路径(包含文件名)
* 输入参数： pszConfigFileName-配置文件名
             pszWholePath-配置文件完整路径(包含文件名)
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 无
* 修改日期       版本号       修改人         修改内容
* ------------------------------------------------------------------
* 20150617      V1.0     Zhou Zhaoxiong     创建
********************************************************************/  
void GetCompletePath(UINT8 *pszConfigFileName, UINT8 *pszWholePath)
{
    UINT8 *pszHomePath      = NULL;
    UINT8  szWholePath[256] = {0};

    // 先对输入参数进行异常判断
    if (pszConfigFileName == NULL || pszWholePath == NULL)
    {
        printf("GetCompletePath: input parameter(s) is NULL!\n");
        return;
    }

    pszHomePath = (UINT8 *)getenv("HOME");     // 获取当前用户所在的路径
    if (pszHomePath == NULL)
    {
        printf("GetCompletePath: Can't find home path!\n");
        return;
    }

    // 拼装配置文件路径
    snprintf(szWholePath, sizeof(szWholePath)-1, "%s/zhouzx/TimeSpan/%s", pszHomePath, pszConfigFileName);

    strncpy(pszWholePath, szWholePath, strlen(szWholePath));
}


/**********************************************************************
* 功能描述： 获取具体的字符串值
* 输入参数： fp-配置文件指针
             pszSectionName-段名, 如: GENERAL
             pszKeyName-配置项名, 如: EmployeeName
             iOutputLen-输出缓存长度
* 输出参数： pszOutput-输出缓存
* 返 回 值： 无
* 其它说明： 无
* 修改日期      版本号         修改人        修改内容
* ------------------------------------------------------------------
* 20150617      V1.0     Zhou Zhaoxiong     创建
********************************************************************/
void GetStringContentValue(FILE *fp, UINT8 *pszSectionName, UINT8 *pszKeyName, UINT8 *pszOutput, UINT32 iOutputLen)
{
    UINT8  szSectionName[100]    = {0};
    UINT8  szKeyName[100]        = {0};
    UINT8  szContentLine[256]    = {0};
    UINT8  szContentLineBak[256] = {0};
    UINT32 iContentLineLen       = 0;
    UINT32 iPositionFlag         = 0;

    // 先对输入参数进行异常判断
    if (fp == NULL || pszSectionName == NULL || pszKeyName == NULL || pszOutput == NULL)
    {
        printf("GetStringContentValue: input parameter(s) is NULL!\n");
        return;
    }

    sprintf(szSectionName, "[%s]", pszSectionName);
    strcpy(szKeyName, pszKeyName);

    while (feof(fp) == 0)
    {
        memset(szContentLine, 0x00, sizeof(szContentLine));
        fgets(szContentLine, sizeof(szContentLine), fp);      // 获取段名

        // 判断是否是注释行(以;开头的行就是注释行)或以其他特殊字符开头的行
        if (szContentLine[0] == ';' || szContentLine[0] == '\r' || szContentLine[0] == '\n' || szContentLine[0] == '\0')
        {
            continue;
        }

        // 匹配段名
        if (strncasecmp(szSectionName, szContentLine, strlen(szSectionName)) == 0)     
        {
            while (feof(fp) == 0)
            {
                memset(szContentLine,    0x00, sizeof(szContentLine));
                memset(szContentLineBak, 0x00, sizeof(szContentLineBak));
                fgets(szContentLine, sizeof(szContentLine), fp);     // 获取字段值

                // 判断是否是注释行(以;开头的行就是注释行)
                if (szContentLine[0] == ';')
                {
                    continue;
                }

                memcpy(szContentLineBak, szContentLine, strlen(szContentLine));

                // 匹配配置项名
                if (strncasecmp(szKeyName, szContentLineBak, strlen(szKeyName)) == 0)     
                {
                    iContentLineLen = strlen(szContentLine);
                    for (iPositionFlag = strlen(szKeyName); iPositionFlag <= iContentLineLen; iPositionFlag ++)
                    {
                        if (szContentLine[iPositionFlag] == ' ')
                        {
                            continue;
                        }
                        if (szContentLine[iPositionFlag] == '=')
                        {
                            break;
                        }

                        iPositionFlag = iContentLineLen + 1;
                        break;
                    }

                    iPositionFlag = iPositionFlag + 1;    // 跳过=的位置

                    if (iPositionFlag > iContentLineLen)
                    {
                        continue;
                    }

                    memset(szContentLine, 0x00, sizeof(szContentLine));
                    strcpy(szContentLine, szContentLineBak + iPositionFlag);

                    // 去掉内容中的无关字符
                    for (iPositionFlag = 0; iPositionFlag < strlen(szContentLine); iPositionFlag ++)
                    {
                        if (szContentLine[iPositionFlag] == '\r' || szContentLine[iPositionFlag] == '\n' || szContentLine[iPositionFlag] == '\0')
                        {
                            szContentLine[iPositionFlag] = '\0';
                            break;
                        }
                    }

                    // 将配置项内容拷贝到输出缓存中
                    strncpy(pszOutput, szContentLine, iOutputLen-1);
                    break;
                }
                else if (szContentLine[0] == '[')
                {
                    break;
                }
            }
            break;
        }
    }
}


/**********************************************************************
* 功能描述： 从配置文件中获取字符串
* 输入参数： pszSectionName-段名, 如: GENERAL
             pszKeyName-配置项名, 如: EmployeeName
             pDefaultVal-默认值
             iOutputLen-输出缓存长度
             pszConfigFileName-配置文件名
* 输出参数： pszOutput-输出缓存
* 返 回 值： 无
* 其它说明： 无
* 修改日期      版本号       修改人          修改内容
* ------------------------------------------------------------------
* 20150617      V1.0     Zhou Zhaoxiong     创建
********************************************************************/  
void GetConfigFileStringValue(UINT8 *pszSectionName, UINT8 *pszKeyName, UINT8 *pDefaultVal, UINT8 *pszOutput, UINT32 iOutputLen, UINT8 *pszConfigFileName)
{
    FILE  *fp                    = NULL;
    UINT8  szWholePath[256]      = {0};

    // 先对输入参数进行异常判断
    if (pszSectionName == NULL || pszKeyName == NULL || pszOutput == NULL || pszConfigFileName == NULL)
    {
        printf("GetConfigFileStringValue: input parameter(s) is NULL!\n");
        return;
    }

    // 获取默认值
    if (pDefaultVal == NULL)
    {
        strcpy(pszOutput, "");
    }
    else
    {
        strcpy(pszOutput, pDefaultVal);
    }

    // 打开配置文件
    GetCompletePath(pszConfigFileName, szWholePath);
    fp = fopen(szWholePath, "r");
    if (fp == NULL)
    {
        printf("GetConfigFileStringValue: open %s failed!\n", szWholePath);
        return;
    }

    // 调用函数用于获取具体配置项的值
    GetStringContentValue(fp, pszSectionName, pszKeyName, pszOutput, iOutputLen);

    // 关闭文件
    fclose(fp);
    fp = NULL;
}


/**********************************************************************
* 功能描述： 参数解析函数, 判断是否是包含:的数字串
* 输入参数： *pszStr-输入的字符串
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 无
* 修改日期         版本号         修改人          修改内容
* ------------------------------------------------------
* 20150617        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 IsRightStr(UINT8 *pszStr)
{
    UINT32 iLoopFlag = 0;    
    UINT32 iStrlen   = 0;

    iStrlen = strlen(pszStr);

    for (iLoopFlag = 0; iLoopFlag < iStrlen; iLoopFlag ++)
    {
        if (pszStr[iLoopFlag] < '0' || pszStr[iLoopFlag] > '9')
        {
            if (pszStr[iLoopFlag] != ':')
            {
                return -1;
            }
        }
    }

    return 0;
}
