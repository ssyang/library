

#ifndef	__LOG__UTIL__
#define	__LOG__UTIL__

#include <stdio.h>

#define LOG_LVL_TRACE	50
#define LOG_LVL_DEBUG	40
#define LOG_LVL_INFO		30
#define LOG_LVL_WARNING	20
#define LOG_LVL_ERROR	10
#define LOG_LVL_FATAL	0

/*
* Log file을 생성할 위치와 로그파일의 prefix를 설정함
* LOGset_log_info("/tmp", "mypgm");으로 설정하면
* 로그파일은 /tmp/mypgm-20160101.log 의 형태로 파일을 생성함
*/
int LOGsetInfo(const char *dir, const char *prefix);

/*
* LOGlogging(...) 로그 파일을 생성함.
* log_type, 로그생성일시분초microseconds, process id, 소스파일, 함수명, 라인수, 오류 내용
* 의 format으로 로그를 생성함.
*/
int LOGlogging(char log_type
		, const char *src_file, const char *func, int line_no
		, const char *fmt,va_list arg);

/**
* log level을 조정합니다.
*/
int LOGsetLevel(int log_lvl);

/*
* log level을 최초로 요청할 때에는 환경변수의 설정을 읽습니다.
*/
int LOGgetLevel(void);

/*
 * 함수의 실행 시작과 종료룰 로그로 생성하는 마크로 함수
 */
#define LOG_CALL(func)\
	LOG_TRACE("%s ### starting...",#func);\
	func;\
	LOG_TRACE("%s ### end.",#func)

#endif		//__LOG__UTIL__

