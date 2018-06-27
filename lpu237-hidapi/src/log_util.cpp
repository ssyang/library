#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log_util.h"
#include "inner_log.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <string>

using namespace std;

static char log_file_prefix[64];
static char log_folder[1024] = ".";
static FILE *fp_log_file;
static int  log_level = LOG_LVL_INFO;

static int _mkdir_p(const char *dir, const mode_t mode);

/* recursive mkdir */
int _mkdir_p(const char *dir, const mode_t mode)
{
    char tmp[1024];
    char *p = NULL;
    struct stat sb;
    size_t len;

    /* copy path */
    strncpy(tmp, dir, sizeof(tmp));
    len = strlen(dir);
    if (len >= sizeof(tmp)) {
        return -1;
    }

    /* remove trailing slash */
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    /* recursive mkdir */
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            /* test path */
            if (stat(tmp, &sb) != 0) {
                /* path does not exist - create directory */
                if (mkdir(tmp, mode) < 0) {
                    return -1;
                }
            } else if (!S_ISDIR(sb.st_mode)) {
                /* not a directory */
                return -1;
            }
            *p = '/';
        }
    }
    /* test path */
    if (stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        if (mkdir(tmp, mode) < 0) {
            return -1;
        }
    } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
    }
    return 0;
}

/**
* log level을 조정합니다.
*/
int LOGsetLevel(int log_lvl)
{
    int tmp = LOGgetLevel();

    log_level = log_lvl;

    return tmp;
}
int LOGsetLevelByIni(int log_lvl)
{
    int tmp = LOGgetLevelByIni();

    log_level = log_lvl;

    return tmp;
}

/*
* log level을 최초로 요청할 때에는 환경변수의 설정을 읽습니다.
*/
int LOGgetLevel(void)
{
    char *log_env;
    static int is_env_check = 0;

    if(is_env_check == 0) {
        if((log_env = getenv("LOG_LEVEL")) == NULL) {
            log_level = LOG_LVL_INFO;
        } else {
            if(strcmp(log_env, "TRACE") == 0) {
                log_level = LOG_LVL_TRACE;
            } else if(strcmp(log_env, "DEBUG") == 0) {
                log_level = LOG_LVL_DEBUG;
            } else if(strcmp(log_env, "INFO") == 0) {
                log_level = LOG_LVL_INFO;
            } else if(strcmp(log_env, "WARNING") == 0) {
                log_level = LOG_LVL_WARNING;
            } else if(strcmp(log_env, "ERROR") == 0) {
                log_level = LOG_LVL_ERROR;
            } else if(strcmp(log_env, "FATAL") == 0) {
                log_level = LOG_LVL_FATAL;
            } else {
                log_level = LOG_LVL_INFO;
            }
        }
        is_env_check = 1;
    }
    return log_level;
}
int LOGgetLevelByIni(void)
{
    static bool b_first = true;

    do{
    	if( !b_first )
    		continue;
    	//
    	b_first = false;

		string s_full_path = inner_util::get_current_module_fullpath();
		string s_new_path = inner_util::get_changed_fullpath( s_full_path, string("liblpu237-hidapi.ini") );

		//default disable log
		log_level = LOG_LVL_DISABLE;

		if( s_new_path.empty() )
			continue;
		//
		INIReader reader(s_new_path);

		if( reader.ParseError() < 0 )
			continue;

		long n_log_enable = reader.GetInteger("LogSetting","logenable", 0 );
		if( n_log_enable != 1 )
			continue;
		long n_log_level = reader.GetInteger("LogSetting", "loglevel", 0 );
		switch( n_log_level ){
		case 1:
			log_level = LOG_LVL_ERROR;
			break;
		case 2:
			log_level = LOG_LVL_WARNING;
			break;
		case 3:
			log_level = LOG_LVL_INFO;
			break;
		case 4:
			log_level = LOG_LVL_DEBUG;
			break;
		case 5:
			log_level = LOG_LVL_TRACE;
			break;
		default:
			log_level = LOG_LVL_FATAL;
			break;
		}//end switch
    }while(0);

    return log_level;
}
/*
* Log file을 생성할 위치와 로그파일의 prefix를 설정함
* LOGset_log_info("/tmp", "mypgm");으로 설정하면
* 로그파일은 /tmp/mypgm-20160101.log 의 형태로 파일을 생성함
*/
int LOGsetInfo(const char *dir, const char *prefix)
{
    if(dir == NULL || dir[0] == 0x00) {
        fprintf(stderr, "log folder set error.\n");
        return -1;
    }
    if(prefix == NULL || prefix[0] == 0x00) {
        fprintf(stderr, "log file prefix set error.\n");
        return -1;
    }

    if(strcmp(dir, log_folder) == 0 && strcmp(prefix, log_file_prefix) == 0) {
        return 0;
    }

    strncpy(log_file_prefix, prefix, 64);
    strncpy(log_folder,      dir,    1024);

    if(fp_log_file != NULL) {
        fclose(fp_log_file);
        fp_log_file = NULL;
    }

    return 0;
}


/*
* LOGcreateFile 날짜가 변경되면 자동으로 새로운 파일이 생성됨
*/
static int LOGcreateFile(struct tm *tm1, const char *src_file)
{
    char filename[1024];
    char *ext;

    if(log_folder[0] == 0x00) {
        strcpy(log_folder, ".");
    }
    if(log_file_prefix[0] == 0x00) {
        strncpy(log_file_prefix, src_file, sizeof(log_file_prefix));
        if((ext = strchr(log_file_prefix, '.')) != NULL) {
            *ext = 0x00;
        }
    }
    snprintf(filename, 1024, "%s/%s-%04d%02d%02d%02d%02d%02d.log",
    		log_folder
			, log_file_prefix
			, 1900 + tm1->tm_year
			, tm1->tm_mon + 1
			, tm1->tm_mday
			, tm1->tm_hour
			, tm1->tm_min
			, tm1->tm_sec);

    if(fp_log_file != NULL) {
        fclose(fp_log_file);
        fp_log_file = NULL;
    }

    if( _mkdir_p(log_folder,0777) != 0 ){
    	return -1;
    }
    if((fp_log_file = fopen(filename, "a")) == NULL) {
        return -1;
    }
    setvbuf(fp_log_file, NULL, _IOLBF, 0); 
    return 0;
}

/*
* LOGlogging(...) 로그 파일을 생성함.
* log_type, 로그생성일시분초microseconds, process id, 소스파일, 함수명, 라인수, 오류 내용
* 의 format으로 로그를 생성함.
*/
int LOGlogging(char log_type,
		const char *src_file, const char *func, int line_no,
		const char *fmt,va_list arg)
{
    //va_list ap;
    int  sz = 0;
    struct timeval tv;
    struct tm *tm1;
    static int   day = -1;
    static pid_t pid = -1;
    char   src_info[128];

    gettimeofday(&tv, NULL);
    tm1 = localtime(&tv.tv_sec);
    //va_start(ap, fmt);

    if(pid == -1) {
        pid = getpid();
    }
    /* 날짜가 변경되었으면 또는 최초 실행시에  */
    if(day != tm1->tm_mday) {
        if(LOGcreateFile(tm1, src_file) != 0) {
            return -1;
        }
        day = tm1->tm_mday;
    }

    sz += fprintf(fp_log_file, "(%c) ", log_type);
    sz += fprintf(fp_log_file, "%04d%02d%02d:%02d%02d%02d:%05d",
                                1900 + tm1->tm_year, tm1->tm_mon + 1, tm1->tm_mday,
                                tm1->tm_hour, tm1->tm_min, tm1->tm_sec,pid);
    snprintf(src_info, 128, "%s:%s(%d)", src_file, func, line_no);
    //sz += fprintf(fp_log_file, ":%-50.50s: ", src_info);
    sz += fprintf(fp_log_file, ":%s: ", src_info);
    sz += vfprintf(fp_log_file, fmt, arg);
    sz += fprintf(fp_log_file, "\n");
    //va_end(ap);

    return sz;
}
