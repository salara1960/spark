#include "functions.h"
#include "names.c"

//const char *vers = "0.2";//22.09.2019
//const char *vers = "0.3";//23.09.2019
//const char *vers = "0.4";//24.09.2019
//const char *vers = "0.4.1";//24.09.2019
//const char *vers = "0.5";//25.09.2019
//const char *vers = "0.6";//26.09.2019
//const char *vers = "0.6.1";//26.09.2019
//const char *vers = "0.7";//29.09.2019 //add new command, fixed minor bug in command 'request_status':'combined'
//const char *vers = "0.7.1";//29.09.2019 //pseudo-graphic character conversion
//const char *vers = "0.7.2";//30.09.2019 //add speed param (--speed=), fixed bug in init rs232 (add raw mode)
//const char *vers = "0.7.3";//30.09.2019 //add string status error
//const char *vers = "0.7.4";//01.10.2019 //minor changes in parser status
//const char *vers = "0.8";//01.10.2019 // minor changes : first release done
//const char *vers = "0.9";//15.10.2019 // minor changes in logginig part
//const char *vers = "1.0";//05.11.2019 // minor changes : add new commands
//const char *vers = "1.1";//06.11.2019 // minor changes : add new commands (fixed minor bug in status parser)
//const char *vers = "1.2";//06.11.2019 // minor changes : add new commands+
//const char *vers = "1.3";//07.11.2019 // minor changes in parser status FN
//const char *vers = "1.3.1";//07.11.2019 // minor changes+
//const char *vers = "1.4";//08.11.2019 // minor changes : add new commands+
//const char *vers = "1.4.1";//08.11.2019 // minor changes : add new commands++
//const char *vers = "1.4.2";//09.11.2019 // minor changes : add new commands+++
//const char *vers = "1.4.3";//09.11.2019 // minor changes+
//const char *vers = "1.4.4";//09.11.2019 // minor changes+
//const char *vers = "1.4.5";//10.11.2019 // minor changes++
//const char *vers = "1.4.6";//11.11.2019 // minor changes++
//const char *vers = "1.5";//11.11.2019 // minor changes : add commands for sending msg to OFD server
//const char *vers = "1.6";//12.11.2019 // minor changes : edit main (add ack parser functions)
//const char *vers = "1.6.1";//12.11.2019 // minor changes : add list mode (first step)
//const char *vers = "1.6.2";//13.11.2019 // minor changes : add list mode (next step)
//const char *vers = "1.7";//14.11.2019 // minor changes : fixed major bug in bin2bcd() and next step for list mode
//const char *vers = "1.8";//15.11.2019 // minor changes : add command 'send_test' in --mode=list for for exchange with OFD
//const char *vers = "1.9";//16.11.2019 // minor changes : set max block size for reading to 255 bytes
//const char *vers = "1.9.1";//17.11.2019 // minor changes : add new param --timeout=X (X=3.. sec); edit exchange with OFD
//const char *vers = "2.0";//18.11.2019 // minor changes : add status_error for FN; add 'send_msg' command
const char *vers = "2.1";//19.11.2019 // major changes : fixed bug and add to list 'send_msg' new command 'before_exch:0'




uint8_t from_list = 0;
s_list list;
int8_t listIndex = 0;
uint8_t *pmsgBuf = NULL;

const uint8_t def_swVer[2] = {0x01, 0x05};
uint8_t swVer[2] = {0};
char numberFN[17] = {0};

s_min_head test_sess = {
    .sign = {0x2A, 0x08, 0x41, 0x0A},
    .s_ver = {0x81, 0xA2},
    .p_ver = {0x01, 0x05},
    .numFN = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .msg_len = 0,
    .flags = FLAGI,
    .crc16 = 0
};


#ifdef SET_CLI_OFD
    int8_t QuitCli = 0;
    int cliRET = 0;
    int cliSTART = 0;
#endif

const char sep = '^';
const char *eol = "\r\n";

const char *codeCP866  = "CP866";
const char *codeUTF8   = "UTF-8";

uint8_t dbg = LOG_OFF;

int fd = -1;
int fd_log = -1;

uint8_t QuitAll  = 0;
uint8_t SIGTERMs = 1;
uint8_t SIGINTs  = 1;
uint8_t SIGKILLs = 1;
uint8_t SIGSEGVs = 1;
uint8_t SIGABRTs = 1;
uint8_t SIGSYSs  = 1;
uint8_t SIGTRAPs = 1;

const char *path_log = "/tmp/";
const char *file_log = "spark_log.txt";

char device[sdef] = {0};

int max_size_log = 1024 * 32000;
int MaxLogLevel = LOG_DEBUG + 1;

uint32_t SPEED = 0;

uint8_t byteENQ = 0;


uint8_t blkSize = max_blk_size;
uint8_t msgDone = 0;
uint16_t msgCount = 0;
uint16_t msgLen;
uint16_t msgLenBlk;
s_buf msgBuf = {NULL, 0};

uint8_t ackDone = 0;
uint16_t ackLen;
s_buf ackBuf = {NULL, 0};

uint32_t def_wait_ack_sec = wait_ack_min_sec;


#ifdef SET_CLI_OFD
    const int total_inCMD = totals;
#else
    const int total_inCMD = totals - 2;
#endif
const char *inCMD[] = {
    "set_speed",//Программирование скорости обмена с ПК//STX,‘PR',Код ско-рости,ETX,LRC
    "set_password",//Команда управления (CMD) - Ввод пароля управления ККТ//STX,‘W’,20h,20h,ПАРОЛЬ,ETX,LRC
    "shift_open",//Открытие смены//STX,‘o’,'01'..'99' = № КАССЫ,'00000'..'99999' = ПАРОЛЬ КАССИРА,ETX,LRC
    "shift_close",//Закрытие смены//STX,‘zz’,ETX,LRC
    "set_cashier_number",//Программирование номера кассы//STX,'PN’,№ (2 разряда),ETX,LRC
    "set_cashier_name_password",//Программирование имени и пароля кассира//STX,‘PC’, '01'..'16' = КОД КАССИ-РА,'00000'..'99999' = ПАРОЛЬ КАССИРА,ИМЯ КАССИРА – до 35симв.,ETX,LRC
    "register_cashier",//Регистрация кассира//STX,‘5’,'00000'..'99999' = ПАРОЛЬ КАССИРА,ETX,LRC
    "reset_cashier",//Сброс кассира//STX,‘6’,ETX,LRC
    "clear_error",//Исправление ошибки//STX,‘k’,ETX,LRC
    "line0_print",//Печать текстовых документов//STX,‘PP’,X='0',Текстовая строка = ,ETX,LRC
    "line1_print",//Печать текстовых документов//STX,‘PP’,X='1',Текстовая строка = ,ETX,LRC
    "end_of_print",//Команда завершения печати документа//STX,‘Pp0’,ETX,LRC
    "request_status",//Запрос статусов//ENQ - base; ENQ2 - additional; ENQT - combined
    "restart_device",//Рестарт ККТ//{ACK, ETX, 0, ETX, 0, ETX, 0, ETX, 0, ETX, ACK};
    "general_report",//Общий отчёт по программированию//STX,‘D’,ETX,LRC
    "set_coming_item",//Ввод и печать номера заказа//STX,‘g’,НОМЕР[3],ETX,LRC
    "set_return_item",//Ввод и печать номера заказа//STX,'i',НОМЕР[3],ETX,LRC
    "return_item",       //Возврат товара//STX,0x64,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
    "storno_return_item",//Возврат товара//STX,0xE4,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
                         //X='a'..'f' - ставка налога №1..№6
    "end_of_return_item",//Завершение операции возврата товара//STX,‘f’,ВИД ОПЛАТЫ[1],СУММА[10],ETX,LRC
                        //ВИД ОПЛАТЫ: Наличные ‘8’, Кредит ‘7’, Чек ‘6’, Платёжные карты ‘1’~’5’
    "reg_item",//Регистрации товарной позиции//STX,CMD,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
    "storno_item",//Сторнирования товарной позиции//STX,CMD,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
                 //CMD: '0' - НДС не облагается
                 //'1','2'- НДС, рассчитанный внутри стоимо-сти товара
                 //'3' - НДС 0%
                 //'4','5' - НДС рассчитанный поверх стоимости товара
    "end_of_chek",// Завершение операции (чека) //STX,‘1’,ВИД ОПЛАТЫ[1],ETX,LRC
    "end_of_pay",// Завершение операции оплаты"//STX,‘K’,ВИД ОПЛАТЫ[1],ETX,LRC
                 //ВИД ОПЛАТЫ: Наличные ‘8’, Кредит ‘7’, Чек ‘6’, Платёжные карты ‘1’~’5’
    "request_statFN",//Запрос состояния ФН//STX,‘S”’,ETX,LRC
    "error_reset",//Сброс ошибки (конец ленты или сбой принтера)//STX,‘e',ETX,LRC
    "get_stat_exch",//Получить статус информационного обмена//STX,‘SuF’,ETX,LRC
    "text_report0_print",//Печать текстового отчёта//STX,‘8’,X,Текстовая строка S переменной длины,'0',ETX,LRC
    "text_report1_print",//Печать текстового отчёта//STX,‘8’,X,Текстовая строка S переменной длины,'1',ETX,LRC
    "extra_lines_print",//Печать дополнительных строк в окончании чека//STX,‘1!’,Текстовая строка[N],ETX,LRC
    "cancel_chek",//Аннулирование текущего, открытого чека//STX,‘7’,ETX,LRC
    "last_doc_print",//Выдача дубликата последнего документа (печать)//STX,‘R’,ETX,LRC
    "last_doc_bin",//Выдача дубликата последнего документа (в электронном виде)//STX,‘Su]’,ETX,LRC
    "get_ver_software",//Считывание номера версии//STX,‘SV’,ETX,LRC
    "set_text_attr",//Программирование значений текстовых реквизитов//STX,‘P@’,X,Текстовая строка[N],ETX,LRC
    "read_reg_data",//Прочитать данные регистрации/перерегистрации//STX,‘Su#’,X,ETX,LRC
    "before_exch",//Передать статус транспортного соединения с Сервером ОФД//STX,‘SuG’,X,ETX,LRC
    "get_S1",//Считывание информации S1//STX,‘S1’,ETX,LRC"getS1"//Считывание информации S1//STX,‘S1’,ETX,LRC
    "begin_read_msg",//Начать чтение Сообщения для Сервера ОФД//STX,‘SuH’,ETX,LRC
    "close_chek",//Итог внесений/выплат, завершение кассового чека (стр.26)//STX,‘t’,ETX,LRC
    "info_about_done",//Считывание информации по итогам смены (основные данные)//STX,‘Sf’или‘Sg’,ETX,LRC
    "extra_info_about_done",//Считывание информации по итогам смены (дополнительные данные) (стр.35)//STX,‘Sh’или‘Si’,ETX,LRC

    "read_msg_block",//Прочитать блок сообщения для сервера ОФД//STX,‘SuI’,D[4],N[2],ETX,LRC (D,N - inBCD48)
    "cancel_exch",//Отменить чтение Сообщения для Сервера ОФД//STX,‘SuJ’,ETX,LRC
    "close_exch",//Завершить чтение Сообщения для Сервера ОФД//STX,‘SuK’,ETX,LRC
    "ack_srv"//Передать квитанцию от сервера ОФД//STX,‘SuL’,S(N),ETX,LRC (S[1..1024] - inBCD48)
#ifdef SET_CLI_OFD
    ,"send_test"//Отправить ОФД серверу пустое сообщение
    ,"send_msg"////Отправить ОФД серверу одно сообщение
#endif
};


const uint8_t rst_dev[] = {ACK, ETX, 0, ETX, 0, ETX, 0, ETX, 0, ETX, ACK};

const int max_param = 8;
const char *name_param[] = {
    "--dev=",
    "--cmd=",
    "--arg=",
    "--log=",
    "--speed=",
    "--mode=",
    "--password=",
    "--timeout="
};

const char *spd[] = {
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "76800",
    "115200"
};
const uint32_t ispd[] = {
    B4800,//0000014
    B9600,//0000015
    B19200,//0000016
    B38400,//0000017
    B57600,//0010001
    B115200,//0010002
    B115200//0010002
};

const uint16_t eCrc16Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


#ifdef SET_PRN_MUTEX
    pthread_mutex_t prn_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


//----------------------------------------------------------------------
uint8_t LRC(const uint8_t *buf, int len)
{
uint8_t ret = 0;
const uint8_t *uk = buf;

    while (len--) ret ^= *uk++;

    return ret;
}
//----------------------------------------------------------------------
uint16_t CRC16(const uint8_t *buf, int len)// CRC-16 CCITT , Poly  : 0x1021    x^16 + x^12 + x^5 + 1
{
uint16_t ret = 0xFFFF;
const uint8_t *uk = buf;

    while (len--) ret = (ret << 8) ^ eCrc16Table[(ret >> 8) ^ *uk++];

/*
    while (len--) {
        ret ^= *uk++ << 8;
        for (uint16_t i = 0; i < 8; i++) ret = ret & 0x8000 ? (ret << 1) ^ 0x1021 : ret << 1;

    }
*/
    return ret;
}
//----------------------------------------------------------------------
char *ThisTime()
{
time_t ct = time(NULL);
char *arba = ctime(&ct);

    arba[strlen(arba) - 1] = 0;//remove '\n'

    return (arba);
}
//----------------------------------------------------------------------
char *TimeNowPrn(char *ts)
{
struct tm *ctimka;
struct timeval tvl;

    gettimeofday(&tvl, NULL);
    ctimka = localtime(&tvl.tv_sec);
    sprintf(ts, "%02d.%02d %02d:%02d:%02d.%03d | ",
                ctimka->tm_mday, ctimka->tm_mon + 1,
                ctimka->tm_hour, ctimka->tm_min, ctimka->tm_sec, (int)(tvl.tv_usec/1000));
    return ts;
}
//------------------------------------------------------------------------
void ToSysLogMsg(int LogLevel, const char * const Msg)
{
    if (LogLevel <= MaxLogLevel) syslog(LogLevel, "%s", Msg);
}
//-----------------------------------------------------------------------
void print_msg(uint8_t dt, const char *fmt, ...)
{
size_t len = buf_size << 1;
char dts[TIME_STR_LEN] = {0};
char *udt = TimeNowPrn(dts);

#ifdef SET_PRN_MUTEX
    pthread_mutex_lock(&prn_mutex);

    if (dt) len += sizeof(dts);
    char *st = (char *)calloc(1, len + 1);
    if (st) {
        int dl = 0, sz;
        va_list args;

        if (dt) dl = sprintf(st, "%s", udt);
        sz = dl;

        va_start(args, fmt);
        sz += vsnprintf(st + dl, len - dl, fmt, args);
        va_end(args);

        if (dbg) printf("%s", st);
        if (fd_log) write(fd_log, st, strlen(st));

        struct stat sb;
        if (!fstat(fd_log, &sb)) {
            if (sb.st_size > max_size_log) {
                close(fd_log);
                fd_log = -1;
                char name[128] = {0};
                sprintf(name, "%u_%s", (uint32_t)time(NULL), udt);
                sprintf(dts,"%s%s", path_log, file_log);
                rename(dts, name);
                fd_log = open(dts, O_WRONLY | O_APPEND | O_CREAT, 0664); //create new file
                if (fd_log <= 0) sprintf(name, "%sVer.%s Can't open file %s (%d)\n", udt, vers, dts, fd_log);
                            else sprintf(name, "%sVer.%s Open new file %s (%d)\n", udt, vers, dts, fd_log);
                ToSysLogMsg(LOG_INFO, name);
            }
        }

        free(st);
    }

    pthread_mutex_unlock(&prn_mutex);
#else
    if (dt) len += sizeof(dts);
    char *st = (char *)calloc(1, len + 1);
    if (st) {
        int dl = 0, sz;
        va_list args;

        if (dt) dl = sprintf(st, "%s", udt);
        sz = dl;

        va_start(args, fmt);
        sz += vsnprintf(st + dl, len - dl, fmt, args);
        va_end(args);

        if (dbg) printf("%s", st);
        if (fd_log) write(fd_log, st, strlen(st));

        struct stat sb;
        if (!fstat(fd_log, &sb)) {
            if (sb.st_size > max_size_log) {
                close(fd_log);
                fd_log = -1;
                char name[128] = {0};
                sprintf(name, "%u_%s", (uint32_t)time(NULL), udt);
                sprintf(dts,"%s%s", path_log, file_log);
                rename(dts, name);
                fd_log = open(dts, O_WRONLY | O_APPEND | O_CREAT, 0664); //create new file
                if (fd_log <= 0) sprintf(name, "%sVer.%s Can't open file %s (%d)\n", udt, vers, dts, fd_log);
                            else sprintf(name, "%sVer.%s Open new file %s (%d)\n", udt, vers, dts, fd_log);
                ToSysLogMsg(LOG_INFO, name);
            }
        }

        free(st);
    }
#endif

}
//-----------------------------------------------------------------------
void initList()
{
/*typedef struct {
    int cmd;
    char *arg;
} s_one_cmd;
typedef struct {
    s_one_cmd work[8];
} s_list;*/

    if (!from_list) return;

    for (int i = 0; i < max_rec_list; i++) {
        list.work[i].cmd = -1;
        list.work[i].arg = NULL;
    }

}
//-----------------------------------------------------------------------
void delList()
{
    if (!from_list) return;

    for (int i = 0; i < max_rec_list; i++) {
        list.work[i].cmd = -1;
        if (list.work[i].arg) free(list.work[i].arg);
    }
}
//-----------------------------------------------------------------------
int8_t putToList(int cmd, const char *arg)
{
int8_t ret = -1;

    if (!from_list) return ret;

    for (int8_t i = 0; i < max_rec_list; i++) {
        if (list.work[i].cmd == -1) {
            if (list.work[i].arg) free(list.work[i].arg);
            int len = strlen(arg);
            if (len) {
                char *uk = (char *)calloc(1, len + 1);
                if (uk) {
                    list.work[i].cmd = cmd;
                    memcpy(uk, arg, len);
                    list.work[i].arg = uk;
                    ret = i;
                    break;
                }
            }
        }
    }

    return ret;//index in list
}
//-----------------------------------------------------------------------
int8_t getFromList(int8_t *id, int *cmd, char *arg, uint8_t del)
{
int8_t ret = -1, ix = 0;

    if (!from_list) return ret;

    if (id) {
        ix = *id;
        if ((ix >= 0) && (ix < max_rec_list)) {
            if ((list.work[ix].cmd != -1) && (list.work[ix].arg)) {
                *cmd = list.work[ix].cmd;
                int len = strlen(list.work[ix].arg);
                memcpy(arg, (uint8_t *)&list.work[ix].arg[0], len);
                arg[len] = '\0';
                ret = ix;
                if (del) {
                    list.work[ix].cmd = -1;
                    free(list.work[ix].arg);
                    list.work[ix].arg = NULL;
                }
            }
        }
    } else {
        for (int8_t i = 0; i < max_rec_list; i++) {
            if ((list.work[i].cmd != -1) && (list.work[i].arg)) {
                *cmd = list.work[i].cmd;
                int len = strlen(list.work[i].arg);
                memcpy(arg, (uint8_t *)&list.work[i].arg[0], len);
                arg[len] = '\0';
                ret = i;
                if (del) {
                    list.work[i].cmd = -1;
                    free(list.work[i].arg);
                    list.work[i].arg = NULL;
                }
                break;
            }
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
void GetSignal_(int sig)
{
int out = 0;
char stx[64] = {0};

    switch (sig) {
        case SIGKILL:
            if (SIGKILLs) {
                SIGKILLs = 0;
                strcpy(stx, "\tSignal SIGKILL\n");
                out = 1;
            }
        break;
        case SIGPIPE:
            strcpy(stx, "\tSignal SIGPIPE\n");
        break;
        case SIGTERM:
            if (SIGTERMs) {
                SIGTERMs = 0;
                strcpy(stx, "\tSignal SIGTERM\n");
                out = 1;
            }
        break;
        case SIGINT:
            if (SIGINTs) {
                SIGINTs = 0;
                strcpy(stx, "\tSignal SIGINT\n");
                out = 1;
            }
        break;
        case SIGSEGV:
            if (SIGSEGVs) {
                SIGSEGVs = 0;
                strcpy(stx, "\tSignal SIGSEGV\n");
                out = 1;
            }
        break;
        case SIGABRT:
            if (SIGABRTs) {
                SIGABRTs = 0;
                strcpy(stx, "\tSignal SIGABRT\n");
                out = 1;
            }
        break;
        case SIGSYS:
            if (SIGSYSs) {
                SIGSYSs = 0;
                strcpy(stx, "\tSignal SIGSYS\n");
                out = 1;
            }
        break;
        case SIGTRAP:
            if (SIGTRAPs) {
                SIGTRAPs = 0;
                strcpy(stx, "\tSignal SIGTRAP\n");
                out = 1;
            }
        break;
            default : sprintf(stx, "\tUNKNOWN signal %d", sig);
    }

    print_msg(1, stx);

    if (out) QuitAll = out;
}
//-----------------------------------------------------------------------
void FromCP866ToSpark(uint8_t *buf, int len)//from codepage cp866 to Spark codepage
{
    for (int i = 0; i < len; i++) {
        if (buf[i] > 0x7f) {
                 if (buf[i] == 0xf0) buf[i] = 0x86;
            else if (buf[i] == 0xf1) buf[i] = 0xb6;
            else if ((buf[i] >= 0x86) && (buf[i] <= 0x9f)) buf[i]++;
            else if ((buf[i] >= 0xa0) && (buf[i] <= 0xa5)) buf[i] += 0x10;
            else if ((buf[i] >= 0xa6) && (buf[i] <= 0xaf)) buf[i] += 0x11;
            else if ((buf[i] >= 0xe0) && (buf[i] <= 0xef)) buf[i] -= 0x1f;
            else if (buf[i] == 0xb0) buf[i] = 0xaf;
            else if ((buf[i] >= 0xb1) && (buf[i] <= 0xdf)) buf[i] -= 0xd1;
        }
    }
}
//-----------------------------------------------------------------------
int conv_text(char *in, char *out, int len)
{
int ret = 0;
size_t ilen = len;
size_t olen = sdef;
char buf[sdef] = {0};
char *uk = buf;

    iconv_t obj = iconv_open(codeCP866, codeUTF8);
    if (obj == (iconv_t)-1) {//Error !
        if (dbg != LOG_OFF) {
            if (errno == EINVAL)
                print_msg(1, "[%s] Conversion from '%s' to '%s' is not supported\n", __func__, codeUTF8, codeCP866);
            else
                print_msg(1, "[%s] Iconv initialization failure:\n", __func__);
        }
    } else {//Ok
        if (iconv(obj, &in, &ilen, &uk, &olen) == (size_t)-1) {//Error !
            if (dbg != LOG_OFF) print_msg(1, "[%s] Error conversion from %u'%s' to %u'%s'\n", __func__, ilen, codeUTF8, olen, codeCP866);
        } else {//Ok
            ret = sizeof(buf) - olen; ret &= 0xff;
            if (ret > 0) {
                FromCP866ToSpark((uint8_t *)buf, ret);
                memcpy(out, buf, ret);
            }
        }
    }

    iconv_close(obj);

    return ret;
}
//-----------------------------------------------------------------------
void FromSparkToCP866(uint8_t *buf, int len)//from Spark codepage to cp866 codepage
{
    for (int i = 0; i < len; i++) {
        if (buf[i] > 0x7f) {
                 if (buf[i] == 0x86) buf[i] = 0xf0;
            else if (buf[i] == 0xb6) buf[i] = 0xf1;
            else if ((buf[i] >= 0x86) && (buf[i] <= 0x9f)) buf[i]--;
            else if ((buf[i] >= 0xa0) && (buf[i] <= 0xa5)) buf[i] -= 0x10;
            else if ((buf[i] >= 0xa6) && (buf[i] <= 0xaf)) buf[i] -= 0x11;
            else if ((buf[i] >= 0xe0) && (buf[i] <= 0xef)) buf[i] += 0x1f;
            else if (buf[i] == 0xaf) buf[i] = 0xb0;
            else if ((buf[i] >= 0xb1) && (buf[i] <= 0xdf)) buf[i] += 0xd1;
        }
    }
}
//-----------------------------------------------------------------------
int deconv_text(char *in, char *out, int len)
{
int ret = 0;
size_t ilen = len;
size_t olen = sdef;
char obuf[sdef] = {0};
char *ouk = obuf;
char *iuk = in;

    iconv_t obj = iconv_open(codeUTF8, codeCP866);
    if (obj == (iconv_t)-1) {//Error !
        if (dbg != LOG_OFF) {
            if (errno == EINVAL)
                print_msg(1, "[%s] Conversion from '%s' to '%s' is not supported\n", __func__, codeCP866, codeUTF8);
            else
                print_msg(1, "[%s] Iconv initialization failure:\n", __func__);
        }
    } else {//Ok
        FromSparkToCP866((uint8_t *)in, len);
        if (iconv(obj, &iuk, &ilen, &ouk, &olen) == (size_t)-1) {//Error !
            if (dbg != LOG_OFF) print_msg(1, "[%s] Error conversion from %u'%s' to %u'%s'\n", __func__, ilen, codeCP866, olen, codeUTF8);
        } else {//Ok
            ret = sizeof(obuf) - olen; ret &= 0xff;
            if (ret > 0) memcpy(out, obuf, ret);
        }
    }

    iconv_close(obj);

    return ret;
}
//-----------------------------------------------------------------------
int parse_inCMD(char *in_cmd)
{
int ret = -1, i;

    for (i = 0; i < total_inCMD; i++) {
        if (!strncmp(in_cmd, inCMD[i], strlen(inCMD[i]))) {
            ret = i;
            break;
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
void setSPEED(uint8_t SpeedIndex)
{
    if (SpeedIndex < max_spd) {

        if (SPEED != ispd[SpeedIndex]) {

            SPEED = ispd[SpeedIndex];

            struct termios tio;

            tcgetattr(fd, &tio);
            cfmakeraw(&tio);//set RAW mode
            tio.c_cflag = CS8 | CLOCAL | PARENB  | CREAD | SPEED;
            tcflush(fd, TCIFLUSH);
            tcsetattr(fd, TCSANOW, &tio);

            if (dbg >= LOG_DEBUG) print_msg(1, "Set speed for device '%s' to %s 8E1 OK\n", device, spd[SpeedIndex]);
        }

    } else {
        if (dbg != LOG_OFF) print_msg(1, "Error speed index %u for device '%s'\n", SpeedIndex, device);
    }

}
//-----------------------------------------------------------------------
uint8_t findSPEED(char *param)
{
uint8_t byte = 1;

    byte = 1;//default speed = spd[1] -> 9600
    for (int i = 0; i < max_spd; i++) {
        if (!strncmp(param, spd[i], strlen(spd[i]))) {
            byte = i;
            break;
        }
    }
    if (byte == 5) byte++;//speed 76800 not support, goto speed 115200 !

    return byte;
}
//-----------------------------------------------------------------------
int makeCMD(uint8_t *buf, int idx, char *arg)
{
int ret = 0;
static char one[sdef]  = {0};

    switch (idx) {
        case CMD_SET_SPEED://0:// "set_speed" //STX, ‘PR’,Код ско-рости,ETX,LRC
        {
            s_cmd_spd *cmd_spd = (s_cmd_spd *)buf;
            cmd_spd->stx = STX;
            memcpy(cmd_spd->txt, "PR", 2);
            uint8_t byte = findSPEED(arg);
            cmd_spd->spd = byte + 0x30;
            cmd_spd->etx = ETX;
            cmd_spd->crc = LRC(buf + 1, sizeof(s_cmd_spd) - 2);
            ret = sizeof(s_cmd_spd);
            setSPEED(byte);
        }
        break;
        case CMD_SET_PASSWORD://1:// "set_password" //"W  ",//Команда управления (CMD) - Ввод пароля управления ККТ//STX,‘W’,20h,20h,ПАРОЛЬ=“012345”,ETX,LRC
        {
            s_cmd_pwd *cmd_pwd = (s_cmd_pwd *)buf;
            cmd_pwd->stx = STX;
            memcpy(cmd_pwd->txt, "W  ", 3);
            memcpy(cmd_pwd->pwd, arg, 6);
            cmd_pwd->etx = ETX;
            cmd_pwd->crc = LRC(buf + 1, sizeof(s_cmd_pwd) - 2);
            ret = sizeof(s_cmd_pwd);
        }
        break;
        case CMD_SHIFT_OPEN://2:// "shift_open"// //Открытие смены//STX,‘o’,'01'..'99' = № КАССЫ,'00000'..'99999' = ПАРОЛЬ КАССИРА,ETX,LRC
        {
            if (strchr(arg, sep)) {
                s_cmd_open *cmd_open = (s_cmd_open *)buf;
                cmd_open->stx = STX;
                cmd_open->txt = 'o';
                memcpy(cmd_open->cnum, arg, 2);//№ КАССЫ
                memcpy(cmd_open->cpwd, arg + 3, 5);//ПАРОЛЬ КАССИРА
                cmd_open->etx = ETX;
                cmd_open->crc = LRC(buf + 1, sizeof(s_cmd_open) - 2);
                ret = sizeof(s_cmd_open);
            }
        }
        break;
        case CMD_SHIFT_CLOSE://3:// "shift_close"// //Закрытие смены//STX,‘zz’,ETX,LRC
        {
            s_cmd_close *cmd_close = (s_cmd_close *)buf;
            cmd_close->stx = STX;
            memcpy(cmd_close->txt, "zz", 2);
            cmd_close->etx = ETX;
            cmd_close->crc = LRC(buf + 1, sizeof(s_cmd_close) - 2);
            ret = sizeof(s_cmd_close);
        }
        break;
        case CMD_SET_CASHIER_NUMBER://4:// "set_cashier_number"//Программирование номера кассы//STX,'PN’,№ (2 разряда),ETX,LRC
        {
            s_cmd_cash_num *cmd_cash_num = (s_cmd_cash_num *)buf;
            cmd_cash_num->stx = STX;
            memcpy(cmd_cash_num->txt, "PN", 2);
            memcpy(cmd_cash_num->cnum, arg, 2);
            cmd_cash_num->etx = ETX;
            cmd_cash_num->crc = LRC(buf + 1, sizeof(s_cmd_cash_num) - 2);
            ret = sizeof(s_cmd_cash_num);
        }
        break;
        case CMD_SET_CASHIER_NAME_PASSWORD://5:// "set_cashier_name_password"//STX,‘PC’,'01'..'16','00000'..'99999','name[<=35]',ETX,LRC
        {
            int ct = 0;
            char *uki = arg;
            if (strchr(uki, sep)) {
                ct++;
                uki++;
                if (strchr(uki, sep)) ct++;
            }
            if (ct == 2) {//arg = "01^00001^name"
                memset(buf, 0, sizeof(s_cmd_cash_pn) + 37);
                s_cmd_cash_pn *cmd_cash_pn = (s_cmd_cash_pn *)buf;
                cmd_cash_pn->stx = STX;
                memcpy(cmd_cash_pn->txt, "PC", 2);
                memcpy(cmd_cash_pn->ccode, arg, 2);//"01"
                uki = arg + 3;
                memcpy(cmd_cash_pn->cpwd, uki, 5);//"00001"
                uki = arg + 9;
                ct = strlen(uki); if (ct > 35) ct = 35;
                memcpy(buf + sizeof(s_cmd_cash_pn), uki, ct);
                ct += sizeof(s_cmd_cash_pn);
                buf[ct++] = ETX;
                buf[ct] = LRC(buf + 1, ct - 1);
                ret = ct + 1;
            }
        }
        break;
        case CMD_REGISTER_CASHIER://6:// "register_cashier"//Регистрация кассира//STX,‘5’,'00000'..'99999' = ПАРОЛЬ КАССИРА,ETX,LRC
        {
            s_cmd_reg_cash *cmd_reg_cash = (s_cmd_reg_cash *)buf;
            cmd_reg_cash->stx = STX;
            cmd_reg_cash->txt = '5';
            memcpy(cmd_reg_cash->cpwd, arg, 5);//"00001"
            cmd_reg_cash->etx = ETX;
            cmd_reg_cash->crc = LRC(buf + 1, sizeof(s_cmd_reg_cash) - 2);
            ret = sizeof(s_cmd_reg_cash);
        }
        break;
        case CMD_RESET_CASHIER://7:// "reset_cashier"//Сброс кассира//STX,‘6’,ETX,LRC
        {
            s_cmd_clr_cash *cmd_clr_cash = (s_cmd_clr_cash *)buf;
            cmd_clr_cash->stx = STX;
            cmd_clr_cash->txt = '6';
            cmd_clr_cash->etx = ETX;
            cmd_clr_cash->crc = LRC(buf + 1, sizeof(s_cmd_clr_cash) - 2);
            ret = sizeof(s_cmd_clr_cash);
        }
        break;
        case CMD_CLEAR_ERROR://8:// "clear_error"//Исправление ошибки//STX,‘k’,ETX,LRC
        {
            s_cmd_clr_err *cmd_clr_err = (s_cmd_clr_err *)buf;
            cmd_clr_err->stx = STX;
            cmd_clr_err->txt = 'k';
            cmd_clr_err->etx = ETX;
            cmd_clr_err->crc = LRC(buf + 1, sizeof(s_cmd_clr_err) - 2);
            ret = sizeof(s_cmd_clr_err);
        }
        break;
        case CMD_LINE0_PRINT://9: // "line0_print"//Печать текстовых документов//STX,‘PP’,X='0',Текстовая строка = ,ETX,LRC
        case CMD_LINE1_PRINT://10:// "line1_print"//Печать текстовых документов//STX,‘PP’,X='1',Текстовая строка = ,ETX,LRC
        {
            int len = strlen(arg), dl = 1;
            char *txt = arg;
            if (len > 0) {
                len = conv_text(arg, &one[0], len);
                if (len > 0) {
                    //
                    if (len > max_print_line) {
                        len = max_print_line;
                        one[len] = '\0';
                    }
                    //
                    txt = &one[0];
                    if (dbg == LOG_DUMP) {//--- for DUMP mode ---
                        int ltmp = (sdef << 1) + 64;
                        char *tmp = (char *)calloc(1, ltmp);
                        if (tmp) {
                            sprintf(tmp, "[%s] Text(%d):", __func__, len);
                            for (int j = 0; j < len; j++) sprintf(tmp+strlen(tmp), " %02X", (uint8_t)one[j]);
                            print_msg(1, "%s\n", tmp);
                            free(tmp);
                        } else {
                            if (dbg != LOG_OFF) print_msg(1, "[%s] Error calloc for tmp_buf with size %d\n", __func__, ltmp);
                        }
                    }//------------------------------------------
                }
                if (len > 0) {
                    *buf = STX;
                    if (idx == 9) memcpy(buf + dl, "PP0", 3);
                             else memcpy(buf + dl, "PP1", 3);
                    dl += 3;
                    if (strcmp(one, eol)) {
                        memcpy(buf + dl, txt, len);
                        dl += len;
                    }
                    buf[dl++] = ETX;
                    buf[dl] = LRC(buf + 1, dl - 1);
                    ret = dl + 1;
                }
            }
        }
        break;
        case CMD_END_OF_PRINT://11:// "end_of_print"//Для завершения документа следует использовать команду//STX,‘Pp0’,ETX,LRC
        {
            s_cmd_end_print *cmd_end_print = (s_cmd_end_print *)buf;
            cmd_end_print->stx = STX;
            memcpy(cmd_end_print->txt, "Pp0", 3);
            cmd_end_print->etx = ETX;
            cmd_end_print->crc = LRC(buf + 1, sizeof(s_cmd_end_print) - 2);
            ret = sizeof(s_cmd_end_print);
        }
        break;
        case CMD_REQUEST_STATUS://12:// "request_status"
                 if (!strcmp(arg, "base"))       byteENQ = ENQ;
            else if (!strcmp(arg, "additional")) byteENQ = ENQ2;
            else if (!strcmp(arg, "combined"))   byteENQ = ENQT;
            else byteENQ = 0;
            if (byteENQ) {
                *buf = byteENQ;
                ret = 1;
            }
        break;
        case CMD_RESTART_DEVICE://13:// "restart_device"
            ret = sizeof(rst_dev);
            memcpy(buf, rst_dev, ret);
        break;
        case CMD_GENERAL_REPORT://14:// "general_report"//Общий отчёт по программированию//STX,‘D’,ETX,LRC
        {
            s_cmd_report *cmd_report = (s_cmd_report *)buf;
            cmd_report->stx = STX;
            cmd_report->txt = 'D';
            cmd_report->etx = ETX;
            cmd_report->crc = LRC(buf + 1, sizeof(s_cmd_report) - 2);
            ret = sizeof(s_cmd_report);
        }
        break;
        case CMD_SET_COMING_ITEM://15:// "set_coming_item"//Ввод и печать номера заказа//STX,‘g’,НОМЕР[3],ETX,LRC
        case CMD_SET_RETURN_ITEM://16:// "set_return_item"//Ввод и печать номера заказа//STX,'i',НОМЕР[3],ETX,LRC
        {
            int dl = strlen(arg);
            if (dl > 0) {
                if (dl > 3) dl = 3;
                s_cmd_set_item *cmd_set_item = (s_cmd_set_item *)buf;
                cmd_set_item->stx = STX;
                if (idx == 15) cmd_set_item->txt = 'g';//Приход
                          else cmd_set_item->txt = 'i';//Возврат
                memset(&cmd_set_item->num[0], '0', 3);
                memcpy(&cmd_set_item->num[3 - dl], arg, dl);
                cmd_set_item->etx = ETX;
                cmd_set_item->crc = LRC(buf + 1, sizeof(s_cmd_set_item) - 2);
                ret = sizeof(s_cmd_set_item);
            }
        }
        break;
        case CMD_RETURN_ITEM://17://    "return_item"//Возврат товара//STX,0x64,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
        case CMD_STORNO_RETURN_ITEM://18://    "storno_return_item"//Возврат товара//STX,0xE4,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
        {//arg=a^00000123^00000001^01^Товар
         //X='a'..'f' //Цена[8]='00000123' //Количество[8]='00000001' //Отдел[2]='01' //Наименование[N]='Товар' не более 36 сивволов
            int dl = strlen(arg);
            if (dl > 0) {
                char *uk = arg;
                uint8_t byte = 0x64;
                s_cmd_ret_item *cmd_ret_item = (s_cmd_ret_item *)buf;
                cmd_ret_item->stx = STX;
                if (idx == 18) byte += 0x80;
                cmd_ret_item->cmd = byte;
                cmd_ret_item->type = *uk; uk += 2;
                memset(cmd_ret_item->price, '0', 18);
                memcpy(cmd_ret_item->price, uk, 8); uk += 9;
                memcpy(cmd_ret_item->quantity, uk, 8); uk += 9;
                memcpy(cmd_ret_item->dep, uk, 2); uk += 3;
                dl = strlen(uk); if (dl > 36) dl = 36;
                dl = conv_text(uk, one, dl);
                uk = (char *)(buf + sizeof(s_cmd_ret_item));
                if (dl) {
                    memcpy(uk, one, dl);
                    uk += dl;
                }
                *uk++ = ETX;
                *uk = LRC(buf + 1, sizeof(s_cmd_ret_item) + dl);
                ret = sizeof(s_cmd_ret_item) + dl + 2;
            }
        }
        break;
        case CMD_END_OF_RETURN_ITEM://19:// "end_of_return_item"//Завершение операции возврата товара//STX,‘f’,ВИД ОПЛАТЫ[1],СУММА[10],ETX,LRC
        {//arg=1^0000000056
            int dl = strlen(arg);
            if (dl >= 10) {
                char tp = '1';
                s_cmd_end_ret_item *cmd_end_ret_item = (s_cmd_end_ret_item *)buf;
                cmd_end_ret_item->stx = STX;
                cmd_end_ret_item->txt = 'f';
                char *uki = strchr(arg, '^');
                if (uki) {
                    uki++;
                    tp = arg[0];
                    if ((tp < '1') || (tp > '8')) tp = '1';
                } else uki = arg;
                cmd_end_ret_item->type = tp;
                memcpy(&cmd_end_ret_item->summa[0], uki, 10);
                cmd_end_ret_item->etx = ETX;
                cmd_end_ret_item->crc = LRC(buf + 1, sizeof(s_cmd_end_ret_item) - 2);
                ret = sizeof(s_cmd_end_ret_item);
            }
        }
        break;
        case CMD_REG_ITEM://20:// "reg_item",//Регистрации товарной позиции//STX,НДС,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
        case CMD_STORNO_ITEM://21:// "storno_item"//Сторнирования товарной позиции//STX,НДС,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
        {//arg=0^00000123^00000001^01^Товар
         //НДС=0..5//(0x20..0x25 - "reg_item") or (0xEA0..0xA5 - "storno_item")
         //Цена[8]='00000123'
         //Количество[8]='00000001'
         //Отдел[2]='01'
         //Наименование[N]='Товар' //не более 36 сивволов
            int dl = strlen(arg);
            if (dl > 0) {
                s_cmd_reg_item *cmd_reg_item = (s_cmd_reg_item *)buf;
                char *uk = arg;
                cmd_reg_item->stx = STX;
                char sym = *uk; sym -= 0x30;
                if (idx == 20) sym += 0x20; else sym += 0xA0;
                cmd_reg_item->cmd = sym; uk += 2;
                memset(cmd_reg_item->price, '0', 18);
                memcpy(cmd_reg_item->price, uk, 8); uk += 9;
                memcpy(cmd_reg_item->quantity, uk, 8); uk += 9;
                memcpy(cmd_reg_item->dep, uk, 2); uk += 3;
                dl = strlen(uk); if (dl > 36) dl = 36;
                dl = conv_text(uk, one, dl);
                uk = (char *)(buf + sizeof(s_cmd_reg_item));
                if (dl) {
                    memcpy(uk, one, dl);
                    uk += dl;
                }
                *uk++ = ETX;
                *uk = LRC(buf + 1, sizeof(s_cmd_reg_item) + dl);
                ret = sizeof(s_cmd_reg_item) + dl + 2;
            }
        }
        break;
        case CMD_END_OF_CHEK://22:// "end_of_chek"// Завершение операции (чека) //STX,‘1’,ВИД ОПЛАТЫ[1],ETX,LRC
        case CMD_END_OF_PAY://23:// "end_of_pay"  // Завершение операции оплаты"//STX,‘K’,ВИД ОПЛАТЫ[1],ETX,LRC
                                                  //ВИД ОПЛАТЫ: Наличные ‘8’, Кредит ‘7’, Чек ‘6’, Платёжные карты ‘1’~’5’
        {
            int dl = strlen(arg);
            if (dl > 0) {
                s_cmd_end_of_proc *cmd_end_of_proc = (s_cmd_end_of_proc *)buf;
                cmd_end_of_proc->stx = STX;
                if (idx == CMD_END_OF_CHEK) cmd_end_of_proc->txt = '1'; else cmd_end_of_proc->txt = 'K';
                char sym = arg[0];
                if ((sym < '1') || (sym > '8')) sym = 1;//card
                cmd_end_of_proc->type = sym;
                cmd_end_of_proc->etx = ETX;
                cmd_end_of_proc->crc = LRC(buf + 1, sizeof(s_cmd_end_of_proc) - 2);
                ret = sizeof(s_cmd_end_of_proc);
            }
        }
        break;
        case CMD_REQUEST_STATFN://24:// "request_statFN"//Запрос состояния ФН//STX,‘S”’,ETX,LRC
        case CMD_GET_VER_SOFTWARE://33://"get_ver_software"//Считывание номера версии//STX,‘SV’,ETX,LRC
        case CMD_GET_S1://37://"get_S1"//Считывание информации S1//STX,‘S1’,ETX,LRC
        case CMD_INFO_ABOUT_DONE://40://"info_about_done"//Считывание информации по итогам смены (основные данные)//STX,‘Sf’или‘Sg’,ETX,LRC
        case CMD_EXTRA_INFO_ABOUT_DONE://41://"extra_info_about_done"//Считывание информации по итогам смены (дополнительные данные) (стр.35)//STX,‘Sh’или‘Si’,ETX,LRC
        {
            s_cmd_req_statFN *cmd_req_statFN = (s_cmd_req_statFN *)buf;
            cmd_req_statFN->stx = STX;
            cmd_req_statFN->txt[0] = 'S';
            if (idx == CMD_REQUEST_STATFN)        cmd_req_statFN->txt[1] = '"';
            else if (idx == CMD_GET_VER_SOFTWARE) cmd_req_statFN->txt[1] = 'V';
            else if (idx == CMD_GET_S1)           cmd_req_statFN->txt[1] = '1';
            else if (idx == CMD_INFO_ABOUT_DONE) {
                if (!strcmp(arg, "shift")) cmd_req_statFN->txt[1] = 'f';//Для получения информации по итогам смены
                                      else cmd_req_statFN->txt[1] = 'g';//Для получения информации по общим итогам
            }
            else if (idx == CMD_EXTRA_INFO_ABOUT_DONE) {
                if (!strcmp(arg, "shift")) cmd_req_statFN->txt[1] = 'h';//Для получения информации по итогам смены
                                      else cmd_req_statFN->txt[1] = 'i';//Для получения информации по общим итогам
            }
            cmd_req_statFN->etx = ETX;
            cmd_req_statFN->crc = LRC(buf + 1, sizeof(s_cmd_req_statFN) - 2);
            ret = sizeof(s_cmd_req_statFN);
        }
        break;
        case CMD_ERROR_RESET://25:// "error_reset"//Сброс ошибки (конец ленты или сбой принтера)//STX,‘e',ETX,LRC
        {
            s_cmd_err_reset *cmd_err_reset = (s_cmd_err_reset *)buf;
            cmd_err_reset->stx = STX;
            cmd_err_reset->txt = 'e';
            cmd_err_reset->etx = ETX;
            cmd_err_reset->crc = LRC(buf + 1, sizeof(s_cmd_err_reset) - 2);
            ret = sizeof(s_cmd_err_reset);
        }
        break;
        case CMD_GET_STAT_EXCH://26:// "get_stat_exch"//Получить статус информационного обмена//STX,‘SuF’,ETX,LRC
        case CMD_LAST_DOC_BIN://32:// "last_doc_bin"//Выдача дубликата последнего документа (в электронном виде)//STX,‘Su]’,ETX,LRC
        case CMD_BEGIN_READ_MSG://38://"begin_read_msg"//Начать чтение Сообщения для Сервера ОФД//STX,‘SuH’,ETX,LRC
        {
            s_cmd_get_stat_exch *cmd_get_stat_exch = (s_cmd_get_stat_exch *)buf;
            cmd_get_stat_exch->stx = STX;
            if (idx == CMD_GET_STAT_EXCH) memcpy(&cmd_get_stat_exch->txt[0], "SuF", 3);
            else if (idx == CMD_LAST_DOC_BIN) memcpy(&cmd_get_stat_exch->txt[0], "Su]", 3);
            else if(idx == CMD_BEGIN_READ_MSG) memcpy(&cmd_get_stat_exch->txt[0], "SuH", 3);
            cmd_get_stat_exch->etx = ETX;
            cmd_get_stat_exch->crc = LRC(buf + 1, sizeof(s_cmd_get_stat_exch) - 2);
            ret = sizeof(s_cmd_get_stat_exch);
        }
        break;
        case CMD_TEXT_REPORT0_PRINT://27:// "text_report0_print"//Печать текстового отчёта//STX,‘8’,X,Текстовая строка S переменной длины,'0',ETX,LRC
        case CMD_TEXT_REPORT1_PRINT://28:// "text_report1_print"//Печать текстового отчёта//STX,‘8’,X,Текстовая строка S переменной длины,'1',ETX,LRC
        {
            int len = strlen(arg) - 2, dl = 0;
            char *txt = arg + 2;
            if (len > 0) {
                len = conv_text(arg + 2, &one[0], len);
                if (len > 0) {
                    if (len > max_print_line) {
                        len = max_print_line;
                        one[len] = '\0';
                    }
                    txt = &one[0];
                    if (dbg == LOG_DUMP) {//--- for DUMP mode ---
                        int ltmp = (sdef << 1) + 64;
                        char *tmp = (char *)calloc(1, ltmp);
                        if (tmp) {
                            sprintf(tmp, "[%s] Text(%d):", __func__, len);
                            for (int j = 0; j < len; j++) sprintf(tmp+strlen(tmp), " %02X", (uint8_t)one[j]);
                            print_msg(1, "%s\n", tmp);
                            free(tmp);
                        } else {
                            if (dbg != LOG_OFF) print_msg(1, "[%s] Error calloc for tmp_buf with size %d\n", __func__, ltmp);
                        }
                    }//------------------------------------------
                }
                if (len > 0) {//0^text
                    dl = 0;
                    buf[dl++] = STX;
                    buf[dl++] = '8';
                    buf[dl++] = arg[0];
                    memcpy(buf + dl, txt, len); dl += len;
                    if (idx == CMD_TEXT_REPORT0_PRINT)
                        buf[dl++] = '0';
                    else
                        buf[dl++] = '1';
                    buf[dl++] = ETX;
                    buf[dl] = LRC(buf + 1, dl - 1);
                    ret = dl + 1;
                }
            }
        }
        break;
        case CMD_EXTRA_LINES_PRINT://29://"extra_lines_print"//Печать дополнительных строк в окончании чека//STX,‘1!’,Текстовая строка[N],ETX,LRC
        {
            int len = strlen(arg), dl = 0;
            char *txt = arg;
            if (len > 0) {
                len = conv_text(arg, &one[0], len);
                if (len > 0) {
                    if (len > 42) {
                        len = 42;
                        one[len] = '\0';
                    }
                    txt = &one[0];
                    if (dbg == LOG_DUMP) {//--- for DUMP mode ---
                        int ltmp = (sdef << 1) + 64;
                        char *tmp = (char *)calloc(1, ltmp);
                        if (tmp) {
                            sprintf(tmp, "[%s] Text(%d):", __func__, len);
                            for (int j = 0; j < len; j++) sprintf(tmp+strlen(tmp), " %02X", (uint8_t)one[j]);
                            print_msg(1, "%s\n", tmp);
                            free(tmp);
                        } else {
                            if (dbg != LOG_OFF) print_msg(1, "[%s] Error calloc for tmp_buf with size %d\n", __func__, ltmp);
                        }
                    }//------------------------------------------
                }
                if (len > 0) {//text
                    buf[dl++] = STX;
                    memcpy(buf + dl, "1!", 2); dl += 2;
                    memcpy(buf + dl, txt, len); dl += len;
                    buf[dl++] = ETX;
                    buf[dl] = LRC(buf + 1, dl - 1);
                    ret = dl + 1;
                }
            }

        }
        break;
        case CMD_CANCEL_CHEK://30://"cancel_chek"//Аннулирование текущего, открытого чека//STX,‘7’,ETX,LRC
        case CMD_LAST_DOC_PRINT://31://"last_doc_print"//Выдача дубликата последнего документа//STX,‘R’,ETX,LRC
        {
            s_cmd_err_reset *cmd_err_reset = (s_cmd_err_reset *)buf;
            cmd_err_reset->stx = STX;
                 if (idx == CMD_CANCEL_CHEK) cmd_err_reset->txt = '7';
                                        else cmd_err_reset->txt = 'R';
            cmd_err_reset->etx = ETX;
            cmd_err_reset->crc = LRC(buf + 1, sizeof(s_cmd_err_reset) - 2);
            ret = sizeof(s_cmd_err_reset);
        }
        break;
        case CMD_SET_TEXT_ATTR://34://"set_text attr"//Программирование значений текстовых реквизитов//STX,‘P@’,X,Текстовая строка[N],ETX,LRC
        {
            char *uk = strchr(arg, '^');
            if (!uk) break;
            if ((uk - arg) != 2) break;
            uk++;
            int len = strlen(uk), dl = 0, j;
            if (len > 0) {
                len = conv_text(uk, &one[0], len);
                if (len > 0) {
                    if (len > 250) {
                        len = 250;
                        one[len] = '\0';
                    }
                    if (dbg == LOG_DUMP) {//--- for DUMP mode ---
                        int ltmp = (sdef << 1) + 64;
                        char *tmp = (char *)calloc(1, ltmp);
                        if (tmp) {
                            sprintf(tmp, "[%s] Text(%d):", __func__, len);
                            for (j = 0; j < len; j++) sprintf(tmp+strlen(tmp), " %02X", (uint8_t)one[j]);
                            print_msg(1, "%s\n", tmp);
                            free(tmp);
                        } else {
                            if (dbg != LOG_OFF) print_msg(1, "[%s] Error calloc for tmp_buf with size %d\n", __func__, ltmp);
                        }
                    }//------------------------------------------
                }
                if (len > 0) {//0^text
                    buf[dl++] = STX;
                    memcpy(buf + dl, "P@", 2); dl += 2;
                    memcpy(buf + dl, arg, 2);  dl += 2;
                    char attr[4] = {0};
                    memcpy(attr, arg, 2);
                    j = atoi(attr);
                    switch (j) {
                        case 0:
                            len = 0;
                        break;
                        case 3: case 4: case 5: case 10: case 12: case 14:
                            if (len > 19) len = 19;
                        break;
                        case 6: case 15:
                            if (len > 10) len = 10;
                        break;
                        case 7: case 16: case 17:
                            if (len > 64) len = 64;
                        break;
                        case 9:
                            if (len > 12) len = 12;
                        break;
                        case 11: case 13:
                            if (len > 24) len = 24;
                        break;
                            default : len = -1;
                    }
                    if (len < 0) break;
                    else if (len > 0) {
                        memcpy(buf + dl, one, len);
                        dl += len;
                    }
                    buf[dl++] = ETX;
                    buf[dl] = LRC(buf + 1, dl - 1);
                    ret = dl + 1;
                }
            }
        }
        break;
        case CMD_READ_REG_DATA://35:// "read_reg_data"//Прочитать данные регистрации/перерегистрации//STX,‘Su#’,X,ETX,LRC
        case CMD_BEFORE_EXCH://36:// "before_exch"//Передать статус транспортного соединения с Сервером ОФД//STX,‘SuG’,X,ETX,LRC    "before_exch"//Передать статус транспортного соединения с Сервером ОФД//STX,‘SuG’,X,ETX,LRC
        case CMD_CANCEL_EXCH://43://"cancel_exch"//Отменить чтение Сообщения для Сервера ОФД//STX,‘SuJ’,ETX,LRC
        case CMD_CLOSE_EXCH://44://"close_exch"//Завершить чтение Сообщения для Сервера ОФД//STX,‘SuK’,ETX,LRC
        {
            int dl = 0;
            buf[dl++] = STX;
            memcpy(&buf[dl], "Su", 2); dl += 2;
            if (idx == CMD_READ_REG_DATA) {
                buf[dl++] = '#';
                if (arg[0] != '0') buf[dl++] = arg[0];
            } else if (idx == CMD_BEFORE_EXCH) {//for CMD_BEFORE_EXCH
                buf[dl++] = 'G';
                if (!strlen(arg)) break;
                if ((arg[0] < '0') || (arg[0] > '1')) break;
                buf[dl++] = arg[0];
            } else if (idx == CMD_CANCEL_EXCH) {
                buf[dl++] = 'J';
            } else if (idx == CMD_CLOSE_EXCH) {
                buf[dl++] = 'K';
            }
            buf[dl++] = ETX;
            buf[dl] = LRC(buf + 1, dl - 1);
            ret = dl + 1;
        }
        break;
        case CMD_CLOSE_CHEK://39://"close_chek"//Итог внесений/выплат, завершение кассового чека (стр.26)//STX,‘t’,ETX,LRC
        {
            int dl = 0;
            buf[dl++] = STX;
            buf[dl++] = 't';
            if (!strcmp(arg, "nocat")) buf[dl++] = '0';//блокирует автоматическую обрезку чека
            buf[dl++] = ETX;
            buf[dl] = LRC(buf + 1, dl - 1);
            ret = dl + 1;
        }
        break;
        case CMD_READ_MSG_BLOCK://42://"read_msg_block"//Прочитать блок сообщения для сервера ОФД//STX,‘SuI’,D[4],N[2],ETX,LRC (D,N - inBCD48)
        {
            int dl = strlen(arg);
            if (!strlen(arg)) break;

            if (dl > sdef - 1) dl = sdef - 1;
            blkSize = max_blk_size;
            strncpy(one, arg, dl);
            char *uki = strchr(one, '^');
            if (uki) {
                blkSize = (uint8_t)atoi(uki + 1);
                if (!blkSize) blkSize = max_blk_size;
                *uki = '\0';
            }
            uint16_t shi = (uint16_t)atoi(one);
            if (!shi) msgDone = 0;

            if (msgLen > 0) {
                if (blkSize > msgLen) blkSize = msgLen;//--- !!! ---
            }

            uint8_t tmp[bin_len] = {0};
            uint8_t bcd[bcd_len] = {0};// = {0x30, 0x30, 0x30, 0x30, 0x37, 0x35};//size = 3 * 2 //(tp * 2)

            memcpy(&tmp[0], (uint8_t *)&shi, 2);
            tmp[2] = blkSize;

            int tp = bin2bcd(tmp, bin_len, bcd);
            if (tp != bcd_len) break;

            dl = 0;
            buf[dl++] = STX;
            memcpy(&buf[dl], "SuI", 3); dl += 3;
            memcpy(&buf[dl], bcd, tp); dl += tp;
            buf[dl++] = ETX;
            buf[dl] = LRC(buf + 1, dl - 1);
            ret = dl + 1;
        }
        break;
        case CMD_ACK_SRV://45://"ack_srv"//Передать квитанцию от сервера ОФД//STX,‘SuL’,S(N),ETX,LRC (S[1..1024] - inBCD48)
        {
            int dl = 0, dp = sizeof(s_min_head);
            uint16_t tp = 0;

            //---   conver arg/ackBuf to BCD48   ---
            if (ackDone && ackLen) {
                tp = ackLen - dp;
                if (tp <= 0) break;
            } else break;

            if (dbg >= LOG_DEBUG) {
                char *stz = (char *)calloc(1, (tp << 1) + 128);
                if (stz) {
                    sprintf(stz, "\t%u bytes for put to FN\n", tp);
                    for (int i = dp; i < (dp + tp); i++) sprintf(stz+strlen(stz), "%02X", ackBuf.mem[i]);
                    print_msg(0, "%s\n", stz);
                    free(stz);
                }
            }

            uint8_t *bcd = (uint8_t *)calloc(1, tp << 1);
            if (!bcd) break;

            tp = bin2bcd(&ackBuf.mem[dp], tp, bcd);
            //--------------------------------------

            buf[dl++] = STX;
            memcpy(&buf[dl], "SuL", 3); dl += 3;
            memcpy(&buf[dl], bcd, tp);  dl += tp;
            free(bcd);
            buf[dl++] = ETX;
            buf[dl] = LRC(buf + 1, dl - 1);
            ret = dl + 1;
        }
        break;

    }

    return ret;
}
//-----------------------------------------------------------------------
void parseSTATUS(char *chapa, uint8_t *ST1, uint8_t *ST2, uint8_t *ST3, uint8_t *ST4)
{
uint8_t CT1, CT2, CT3;

    strcat(chapa, "\n\tSTATUS:");

    if (ST1) {
        CT1 = *ST1;
        sprintf(chapa+strlen(chapa), "\n\tCT1:0x%02X", CT1);
             if (!((CT1&0x41)^0x41)) strcat(chapa, " 'В фискальной операции'");
        else if (!((CT1&0x42)^0x42)) strcat(chapa, " 'В нефискальной операции'");
        else if (!((CT1&0x44)^0x44)) strcat(chapa, " 'ККТ занятаили переполнен буфер'");
        else if (!(((CT1&0x48)^0x48)) || (!((CT1&0x50)^0x50))) strcat(chapa, " 'Резерв'");
        else if (!((CT1&0x60)^0x60)) strcat(chapa, " 'Фискальный режим'");
    }

    if (ST2) {
        CT2 = *ST2;
        sprintf(chapa+strlen(chapa), "\n\tCT2:0x%02X", CT2);
             if (CT2 == 0x40) strcat(chapa, " 'Ошибок нет'");
        else if (!((CT2&0x41)^0x41)) strcat(chapa, " 'Конец ленты'");
        else if (!((CT2&0x42)^0x42)) strcat(chapa, " 'Сбой принтера'");
        else if (!((CT2&0x6c)^0x6c)) strcat(chapa, " 'Резерв'");
        else if (!((CT2&0x64)^0x64)) strcat(chapa, " 'Ошибка фискальной памяти'");
        else if (!((CT2&0x60)^0x60)) strcat(chapa, " 'Фискальная ошибка'");
        else if (!((CT2&0x5c)^0x5c)) strcat(chapa, " 'Недействительная команда или неправильные данные'");
        else if (!((CT2&0x58)^0x58)) strcat(chapa, " 'Не зарегистрирован кассир'");
        else if (!((CT2&0x54)^0x54)) strcat(chapa, " 'Неправильно указан налог'");
        else if (!((CT2&0x50)^0x50)) strcat(chapa, " 'Неправильная величина / превышение предела'");
        else if (!((CT2&0x70)^0x70)) strcat(chapa, " 'Дата не установлена'");
        else if (!((CT2&0x48)^0x48)) strcat(chapa, " 'Требуется замена термо-бумаги'");
        else if (!((CT2&0x4c)^0x4c)) strcat(chapa, " 'Требуется замена бумаги подкладного документа'");
    }

    if (ST3) {
        CT3 = *ST3;
        sprintf(chapa+strlen(chapa), "\n\tCT3:0x%02X", CT3);
             if (!((CT3&0x41)^0x41)) strcat(chapa, " 'ККТ открыта'");
        else if (!((CT3&0x42)^0x42)) strcat(chapa, " 'Близость конца бумаги'");
        else if (!((CT3&0x44)^0x44)) strcat(chapa, " 'Расширенная ошибка'"); // -> get_stat 3 !
        else if (!((CT3&0x48)^0x48)) strcat(chapa, " 'ФН близок к заполнению'");
        else if (!((CT3&0x50)^0x50)) strcat(chapa, " 'Открыт денежный ящик'");
        else if (!((CT3&0x60)^0x60)) strcat(chapa, " 'Превышение лимита смены'");
    }

    if (ST4) sprintf(chapa+strlen(chapa), "\n\tCT4:0x%02X", *ST4);

}
//-----------------------------------------------------------------------
int bcd2bin(const uint8_t *bcd, int len, uint8_t *bin)
{
uint8_t bytes[2];
const uint8_t *in = bcd;
int ret = 0, dl = len >> 1;

    if (len < 2) return ret;

    while (ret < dl) {
        memcpy(bytes, in, 2); in += 2;
        bytes[0] -= 0x30; bytes[0] <<= 4;
        bytes[1] -= 0x30; bytes[1] &= 0x0f;
        *(bin + ret) = bytes[0] | bytes[1];
        ret++;
    }

    return ret;
}
//----------------------------------------------------------------------
int bin2bcd(const uint8_t *bin, int len, uint8_t *bcd)
{
uint8_t bytes[2] = {0};
const uint8_t *in = bin;
int ret = 0, ix = 0;

    if (!len) return ret;

    while (ret < len) {
        bytes[0] = bytes[1] = *in++;
        bytes[0] >>= 4;   bytes[0] |= 0x30;
        bytes[1] &= 0x0f; bytes[1] |= 0x30;
        memcpy(bcd + ix, bytes, 2); ix += 2;
        ret++;
    }

    return (ret << 1);
}
//----------------------------------------------------------------------
size_t addToBuf(s_buf *buf, uint8_t *data, size_t size)
{
    if (buf->mem == NULL ) buf->mem = (uint8_t *)calloc(1, size);
                     else  buf->mem = (uint8_t *)realloc(buf->mem, buf->size + size);
    if (!buf->mem) { buf->size = 0; return 0; }

    memcpy(&buf->mem[buf->size], data, size);
    buf->size += size;

    return buf->size;
}
//-----------------------------------------------------------------------
void clearBuf(s_buf *buf)
{
    if (buf) {
        if (buf->mem) { free(buf->mem); buf->mem = NULL; }
        buf->size = 0;
    }
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
void ack_REQUEST_STATFN(char *chaps, const uint8_t *in, int len)//CMD_REQUEST_STATFN)//24://ack for command "request_statusFN"
{
uint8_t *obin = (uint8_t *)calloc(1, len);

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[1], len - 3, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG)) for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        s_ack_req_statFN *ack_req_statFN = (s_ack_req_statFN *)obin;
        uint16_t ccrc16 = CRC16(obin + 1, tdl - 3);
        sprintf(chaps+strlen(chaps), "\n\tFlags:%X '%s'\n\tcurDoc:%02X '%s'\n\tdataDoc:%02X '%s'\n\tstatShift:%02X '%s'\n"
                                     "\talarmFlags:%02X '%s'\n\tdatetime:%02u.%02u.%02u %02u:%02u\n"
                                     "\tnumberFN:%.*s\n\tnumberFD:%u\n\tCRC16:%04X/%04X",
                                     ack_req_statFN->flags, flagsSTR(ack_req_statFN->flags),
                                     ack_req_statFN->curDoc, curDocSTR(ack_req_statFN->curDoc),
                                     ack_req_statFN->dataDoc, dataDocSTR(ack_req_statFN->dataDoc),
                                     ack_req_statFN->statShift, statShiftSTR(ack_req_statFN->statShift),
                                     ack_req_statFN->alarmFlags, aflagsSTR(ack_req_statFN->alarmFlags),
                                     ack_req_statFN->datetime[0], ack_req_statFN->datetime[1],
                                     ack_req_statFN->datetime[2], ack_req_statFN->datetime[3],
                                     ack_req_statFN->datetime[4],
                                     16, ack_req_statFN->numberFN,
                                     ack_req_statFN->numberFD, ack_req_statFN->crc16, ccrc16);
        memcpy(numberFN, ack_req_statFN->numberFN, 16);
        free(obin);
    }
}
//-----------------------------------------------------------------------
int ack_GET_STAT_EXCH(int rt, char *chaps, const uint8_t *in, int len, uint16_t *cMsg)
//CMD_GET_STAT_EXCH)//26://ack for command "get_stat_exch"
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt;
uint16_t ccrc16 = 0;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG)) for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl == sizeof(s_ack_req_stat_exch_err)) {//
            s_ack_req_stat_exch_err *ack_req_stat_exch_err = (s_ack_req_stat_exch_err *)obin;
            ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tack:0x%02X\n\tCRC16:%04X/%04X",
                                       ack_req_stat_exch_err->ack, ack_req_stat_exch_err->crc16, ccrc16);
            ret = RET_MAJOR_ERROR;// -1  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        } else {
            s_ack_req_stat_exch *ack_req_stat_exch = (s_ack_req_stat_exch *)obin;
            char tmp[512] = {0};
            ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tack:0x%02X\n\tstat:0x%02X%s\n"
                                       "\trdBegin:%u\n\ttotalMsg:%u\n"
                                       "\tfirstMsg:%u\n\tdatetime:%02u.%02u.%02u %02u:%02u\n\tCRC16:%04X/%04X",
                                       (ack_req_stat_exch->ack & 0x7f),
                                       ack_req_stat_exch->stat, statExchSTR(ack_req_stat_exch->stat, tmp),
                                       ack_req_stat_exch->rdBegin, ack_req_stat_exch->totalMsg,
                                       ack_req_stat_exch->first,
                                       ack_req_stat_exch->datetime[0], ack_req_stat_exch->datetime[1],
                                       ack_req_stat_exch->datetime[2], ack_req_stat_exch->datetime[3],
                                       ack_req_stat_exch->datetime[4], ack_req_stat_exch->crc16, ccrc16);
            *cMsg = ack_req_stat_exch->totalMsg;//    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            ret = (ack_req_stat_exch->ack & 0x7f);
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_GET_VER_SOFTWARE(char *chaps, const uint8_t *in, uint8_t *icrc, uint8_t *ccrc)//CMD_GET_VER_SOFTWARE)//33://"get_ver_software"//Считывание номера версии//STX,‘SV’,ETX,LRC
{
s_ack_get_ver *ack_get_ver = (s_ack_get_ver *)in;
char tmps[sdef] = {0};
char tmp[sdef] = {0};
int tdl = 0, i;
int ret = RET_OK;

    memcpy(tmp, &ack_get_ver->data, sizeof(ack_get_ver->data));
    for (i = 0; i < sizeof(ack_get_ver->data); i++) {
        if (tmp[i] != 0x7f) {
            tmps[tdl] = tmp[i];
            if (dbg > LOG_DEBUG) sprintf(chaps+strlen(chaps), " %02X", (uint8_t)tmps[tdl]);
            tdl++;
        }
    }
    if (tdl && (dbg > LOG_DEBUG)) strcat(chaps, "\n");
    memset(tmp, 0, sizeof(tmp));
    if (deconv_text(tmps, tmp, tdl) > 0) sprintf(chaps+strlen(chaps), "'%s'", tmp);
    *icrc = ack_get_ver->crc;
    *ccrc = LRC(&in[1], sizeof(s_ack_get_ver) - 2);
    if (*icrc != *ccrc) ret |= 0x1000000;//+16777216

    char *ukn = NULL;
    char *uki = &tmp[0];
    char *uk = strchr(uki, '.');
    if (uk) {
        uki = uk + 1;
        ukn = strchr(uki, '.'); if (ukn) uk = ukn;
        swVer[1] = (uint8_t)atoi(uk + 1);
        *uk = '\0';
         uk--;
        swVer[0] = (uint8_t)atoi(uk);
    }
    if ((swVer[0] | swVer[1]) == 0) memcpy(swVer, def_swVer, 2);
    sprintf(chaps+strlen(chaps), " | swVer:0x%02X%02X", swVer[0], swVer[1]);


    return ret;
}
//-----------------------------------------------------------------------
int ack_READ_REG_DATA(int rt, char *chaps, const uint8_t *in, int len)// CMD_READ_REG_DATA //35://"read_reg_data"//Прочитать данные регистрации/перерегистрации//STX,‘Su#’,X,ETX,LRC
{
uint8_t *obin = (uint8_t *)calloc(1, len + 1);
int ret = rt;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG)) for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        s_ack_read_reg_data *ack_read_reg_data = (s_ack_read_reg_data *)obin;
        uint16_t ccrc16 = CRC16(obin + 1, tdl - 3);
        sprintf(chaps+strlen(chaps), "\n\thdr:%u\n\tdatetime:%02u.%02u.%02u %02u:%02u\n"
                                   "\tinn:%.*s\n\treg_num_dev:%.*s\n\tcode1:%u\n"
                                   "\tmode:%u\n\tcode2:%u\n\tnumFD:%u\n\tfflag:0x%08X\n\tCRC16:%04X/%04X",
                                    ack_read_reg_data->hdr,
                                    ack_read_reg_data->datetime[0],
                                    ack_read_reg_data->datetime[1], ack_read_reg_data->datetime[2],
                                    ack_read_reg_data->datetime[3], ack_read_reg_data->datetime[4],
                                    12, ack_read_reg_data->inn,
                                    20, ack_read_reg_data->reg_num_dev,
                                    ack_read_reg_data->code1, ack_read_reg_data->mode,
                                    ack_read_reg_data->code2, ack_read_reg_data->numFD,
                                    ack_read_reg_data->fflag, ack_read_reg_data->crc16, ccrc16);
        ret = RET_OK;
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_BEFORE_EXCH(int rt, char *chaps, const uint8_t *in, int len)
//CMD_BEFORE_EXCH //"before_exch"//Передать статус транспортного соединения с Сервером ОФД//STX,‘SuG’,X,ETX,LRC
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG)) for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl == sizeof(s_ack_req_stat_exch_err)) {//
            s_ack_req_stat_exch_err *ack_req_stat_exch_err = (s_ack_req_stat_exch_err *)obin;
            uint16_t ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tack:0x%02X\n\tCRC16:%04X/%04X",
                                         ack_req_stat_exch_err->ack, ack_req_stat_exch_err->crc16, ccrc16);
            ret = (ack_req_stat_exch_err->ack & 0x7f);//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_GET_S1(char *chaps, const uint8_t *in, uint8_t *icrc, uint8_t *ccrc)//CMD_GET_S1 //"get_S1"//Считывание информации S1//STX,‘S1’,ETX,LRC
{
s_ack_get_s1 *ack_get_s1 = (s_ack_get_s1 *)in;
int ret = RET_OK;

    sprintf(chaps+strlen(chaps), "\n\tcashier_code:%.*s\n\tprixod:%.*s\n"
                               "\tsch_fd:%.*s\n\tsch_rec:%.*s\n"
                               "\tsch_nfd:%.*s\n\tsch_ind_doc:%.*s\n"
                               "\tid_owner:%.*s\n\treg_num_dev:%.*s\n"
                               "\tnum_dev:%.*s\n\tdatetime:%.*s.%.*s.%.*s %.*s:%.*s:%.*s",
                               2, ack_get_s1->cashier_code, 16, ack_get_s1->prixod,
                               8, ack_get_s1->sch_fd, 4, ack_get_s1->sch_rec,
                               8, ack_get_s1->sch_nfd, 8, ack_get_s1->sch_ind_doc,
                               12, ack_get_s1->id_owner, 20, ack_get_s1->reg_num_dev,
                               14, ack_get_s1->num_dev,
                               2, ack_get_s1->date.year, 2, ack_get_s1->date.mon, 2, ack_get_s1->date.day,
                               2, ack_get_s1->time.hour, 2, ack_get_s1->time.min, 2, ack_get_s1->time.sec);
    *icrc = ack_get_s1->crc;
    *ccrc = LRC(&in[1], sizeof(s_ack_get_s1) - 2);
    if (*icrc != *ccrc) ret |= 0x1000000;//+16777216

    return ret;
}
//-----------------------------------------------------------------------
int ack_BEGIN_READ_MSG(int rt, char *chaps, const uint8_t *in, int len, uint16_t *lMsg)
//CMD_BEGIN_READ_MSG //38://"begin_read_msg"//Начать чтение Сообщения для Сервера ОФД//STX,‘SuH’,ETX,LRC
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt;
uint16_t ccrc16 = 0;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG))
            for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl == sizeof(s_ack_req_stat_exch_err)) {//
            s_ack_req_stat_exch_err *ack_req_stat_exch_err = (s_ack_req_stat_exch_err *)obin;
            ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tack:0x%02X\n\tCRC16:%04X/%04X",
                                           ack_req_stat_exch_err->ack, ack_req_stat_exch_err->crc16, ccrc16);
            ret = ack_req_stat_exch_err->ack & 0x7f;
            if (ret) {
                if (ret != 8) ret *= -1;//RET_OK;
                         else *lMsg = 0;
            }
        } else {
            s_ack_begin_read_msg *ack_begin_read_msg = (s_ack_begin_read_msg *)obin;
            ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tack:0x%02X\n\tlen_msg:%u/0x%04X\n\tCRC16:%04X/%04X",
                                           ack_begin_read_msg->ack,
                                           ack_begin_read_msg->len_msg, ack_begin_read_msg->len_msg,
                                           ack_begin_read_msg->crc16, ccrc16);
            ret = ack_begin_read_msg->ack & 0x7f;
            *lMsg = ack_begin_read_msg->len_msg;
            if (ret) {
                if (ret != 8) ret *= -1;//RET_OK;
            }
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_INFO_ABOUT_DONE(int rt, char *chaps, const uint8_t *in, int len)// CMD_INFO_ABOUT_DONE //40://"info_about_done"//Считывание информации по итогам смены (основные данные)//STX,‘Sf’или‘Sg’,ETX,LRC
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt, i;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[3], len - 5, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG))
            for (i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl) {//
            s_ack_info_about_done *ack_info_about_done = (s_ack_info_about_done *)obin;
            sprintf(chaps+strlen(chaps), "\n\tshift_num:%u\n\tmoney:\n", ack_info_about_done->shift_num);
            for (i = 0; i < 8; i++) sprintf(chaps+strlen(chaps), "\t\t[%d] S1=%d S2=%u Summa=%d\n", i + 1,
                                            ack_info_about_done->money[i].S1,
                                            ack_info_about_done->money[i].S2,
                                            (ack_info_about_done->money[i].S1 * 232) + ack_info_about_done->money[i].S2);
            sprintf(chaps+strlen(chaps), "\tinputs: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->inputs.quantity,
                                            ack_info_about_done->inputs.sum.S1, ack_info_about_done->inputs.sum.S2,
                                            (ack_info_about_done->inputs.sum.S1 * 232) + ack_info_about_done->inputs.sum.S2);
            sprintf(chaps+strlen(chaps), "\titems: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->items.quantity,
                                            ack_info_about_done->items.sum.S1, ack_info_about_done->items.sum.S2,
                                            (ack_info_about_done->items.sum.S1 * 232) + ack_info_about_done->items.sum.S2);
            sprintf(chaps+strlen(chaps), "\tret_inputs: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->ret_inputs.quantity,
                                            ack_info_about_done->ret_inputs.sum.S1, ack_info_about_done->ret_inputs.sum.S2,
                                            (ack_info_about_done->ret_inputs.sum.S1 * 232) + ack_info_about_done->ret_inputs.sum.S2);
            sprintf(chaps+strlen(chaps), "\tret_items: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->ret_items.quantity,
                                            ack_info_about_done->ret_items.sum.S1, ack_info_about_done->ret_items.sum.S2,
                                            (ack_info_about_done->ret_items.sum.S1 * 232) + ack_info_about_done->ret_items.sum.S2);
            sprintf(chaps+strlen(chaps), "\tplus: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->plus.quantity,
                                            ack_info_about_done->plus.sum.S1, ack_info_about_done->plus.sum.S2,
                                            (ack_info_about_done->plus.sum.S1 * 232) + ack_info_about_done->plus.sum.S2);
            sprintf(chaps+strlen(chaps), "\tminus: quantity=%u S1=%d S2=%u Summa=%d\n",
                                            ack_info_about_done->minus.quantity,
                                            ack_info_about_done->minus.sum.S1, ack_info_about_done->minus.sum.S2,
                                            (ack_info_about_done->minus.sum.S1 * 232) + ack_info_about_done->minus.sum.S2);
            sprintf(chaps+strlen(chaps), "\tbalance: S1=%d S2=%u Summa=%d",
                                            ack_info_about_done->balance.S1,
                                            ack_info_about_done->balance.S2,
                                            (ack_info_about_done->balance.S1 * 232) + ack_info_about_done->balance.S2);
            ret = RET_OK;
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_EXTRA_INFO_ABOUT_DONE(int rt, char *chaps, const uint8_t *in, int len)// CMD_EXTRA_INFO_ABOUT_DONE //41://"extra_info_about_done"//Считывание информации по итогам смены (дополнительные данные) (стр.35)//STX,‘Sh’или‘Si’,ETX,LRC
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt, i;

    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[3], len - 5, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG))
            for (i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl) {
            s_ack_extra_info_about_done *ack_extra_info_about_done = (s_ack_extra_info_about_done *)obin;
            sprintf(chaps+strlen(chaps), "\n\tshift_num:%u\n\tmoney:\n", ack_extra_info_about_done->shift_num);
            for (i = 0; i < 8; i++) sprintf(chaps+strlen(chaps), "\t\t[%d] S1=%d S2=%u Summa=%d\n", i + 1,
                                                                ack_extra_info_about_done->money[i].S1,
                                                                ack_extra_info_about_done->money[i].S2,
                                                                (ack_extra_info_about_done->money[i].S1 * 232) + ack_extra_info_about_done->money[i].S2);
            sprintf(chaps+strlen(chaps), "\tcancel: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->cancel.quantity,
                                        ack_extra_info_about_done->cancel.sum.S1, ack_extra_info_about_done->cancel.sum.S2,
                                        (ack_extra_info_about_done->cancel.sum.S1 * 232) + ack_extra_info_about_done->cancel.sum.S2);
            sprintf(chaps+strlen(chaps), "\temergency_cancel: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->emergency_cancel.quantity,
                                        ack_extra_info_about_done->emergency_cancel.sum.S1, ack_extra_info_about_done->emergency_cancel.sum.S2,
                                        (ack_extra_info_about_done->emergency_cancel.sum.S1 * 232) + ack_extra_info_about_done->emergency_cancel.sum.S2);
            sprintf(chaps+strlen(chaps), "\tstorno: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->storno.quantity,
                                        ack_extra_info_about_done->storno.sum.S1, ack_extra_info_about_done->storno.sum.S2,
                                        (ack_extra_info_about_done->storno.sum.S1 * 232) + ack_extra_info_about_done->storno.sum.S2);
            sprintf(chaps+strlen(chaps), "\tcorrection: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->correction.quantity,
                                        ack_extra_info_about_done->correction.sum.S1, ack_extra_info_about_done->correction.sum.S2,
                                        (ack_extra_info_about_done->correction.sum.S1 * 232) + ack_extra_info_about_done->correction.sum.S2);
            sprintf(chaps+strlen(chaps), "\tdup_chek: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->dup_chek.quantity,
                                        ack_extra_info_about_done->dup_chek.sum.S1, ack_extra_info_about_done->dup_chek.sum.S2,
                                        (ack_extra_info_about_done->dup_chek.sum.S1 * 232) + ack_extra_info_about_done->dup_chek.sum.S2);
            sprintf(chaps+strlen(chaps), "\textra_charge_plus: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->extra_charge_plus.quantity,
                                        ack_extra_info_about_done->extra_charge_plus.sum.S1, ack_extra_info_about_done->extra_charge_plus.sum.S2,
                                        (ack_extra_info_about_done->extra_charge_plus.sum.S1 * 232) + ack_extra_info_about_done->extra_charge_plus.sum.S2);
            sprintf(chaps+strlen(chaps), "\tdiscount_plus: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->discount_plus.quantity,
                                        ack_extra_info_about_done->discount_plus.sum.S1, ack_extra_info_about_done->discount_plus.sum.S2,
                                        (ack_extra_info_about_done->discount_plus.sum.S1 * 232) + ack_extra_info_about_done->discount_plus.sum.S2);
            sprintf(chaps+strlen(chaps), "\textra_charge_minus: quantity=%u S1=%d S2=%u Summa=%d\n",
                                        ack_extra_info_about_done->extra_charge_minus.quantity,
                                        ack_extra_info_about_done->extra_charge_minus.sum.S1, ack_extra_info_about_done->extra_charge_minus.sum.S2,
                                        (ack_extra_info_about_done->extra_charge_minus.sum.S1 * 232) + ack_extra_info_about_done->extra_charge_minus.sum.S2);
            sprintf(chaps+strlen(chaps), "\tdiscount_minus: quantity=%u S1=%d S2=%u Summa=%d",
                                        ack_extra_info_about_done->discount_minus.quantity,
                                        ack_extra_info_about_done->discount_minus.sum.S1, ack_extra_info_about_done->discount_minus.sum.S2,
                                        (ack_extra_info_about_done->discount_minus.sum.S1 * 232) + ack_extra_info_about_done->discount_minus.sum.S2);
            ret = RET_OK;
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_READ_MSG_BLOCK(int rt, char *chaps, const uint8_t *in, int len)//CMD_READ_MSG_BLOCK //42://"read_msg_block"//Прочитать блок сообщения для сервера ОФД//STX,‘SuI’,D[4],N[2],ETX,LRC (D,N - inBCD48)
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt, i;
/*typedef struct {
    uint8_t start;//0x04
    uint16_t len;//Длина строки от 4й до 6 позиции включительно (03H)
    uint8_t ack;//Нулевой код ошибки (00H или 80H)
//    uint8_t bytes[<=117];
//    uint16_r ctc16;
} s_ack_read_msg_block;*/

    if (obin) {


        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG)) {
            for (i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), "%02X", obin[i]);
        }
        if (tdl) {
            s_ack_read_msg_block *ack_read_msg_block = (s_ack_read_msg_block *)obin;
            uint16_t icrc16; memcpy(&icrc16, &obin[tdl - 2], 2);
            uint16_t ccrc16 = CRC16(&obin[1], tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tstart:%02X\n\tlen:%u/0x%04X\n\tack:0x%02X\n\tCRC16:%04X/%04X",
                                           ack_read_msg_block->start,
                                           ack_read_msg_block->len, ack_read_msg_block->len,
                                           ack_read_msg_block->ack,
                                           icrc16, ccrc16);
            msgLenBlk = ack_read_msg_block->len;
            if (msgLenBlk > 1) msgLenBlk--;

            ret = (ack_read_msg_block->ack & 0x7f);;
            if (ret) {
                ret *= -1;
                sprintf(chaps+strlen(chaps), "\nError :%s", readBlkSTR(ret));
            } else {//  read msg block OK
                if (msgLenBlk > 0) {
                    uint8_t *uDat = obin + sizeof(s_ack_read_msg_block);

                    // !!!!!!!!! append block to msgBuf !!!!!!!!!!
                    int lng = addToBuf(&msgBuf, uDat, msgLenBlk);
                    if (lng > 0) {
                        if (lng >= msgLen) msgDone = 1;
                    }
                    if (!msgDone) {
                        if (dbg >= LOG_DEBUG) {
                            strcat(chaps, "\n");
                            for (i = 0; i < tdl/*msgLenBlk*/; i++) sprintf(chaps+strlen(chaps), "%02X", *(uint8_t *)(uDat + i));
                        }
                    }
                    if (msgDone && lng) {
                        //uint16_t lenm = lng - 3;
                        //s_ack_read_msg_block *all = (s_ack_read_msg_block *)msgBuf.mem;
                        //all->len = lenm;
                        //ccrc16 = CRC16(msgBuf.mem + 1, lng - 1);
                        //lng = addToBuf(&msgBuf, (uint8_t *)&ccrc16, 2);
                        //
                        if (dbg != LOG_OFF) {
                            if (!lng) sprintf(chaps+strlen(chaps), "\nCan't add %u bytes to msgBuf (Internal error !)", msgLenBlk);
                            else {
                                sprintf(chaps+strlen(chaps), "\nReady msgBuf(%d):\n", lng);
                                for (i = 0; i < lng; i++) sprintf(chaps+strlen(chaps), "%02X", *(msgBuf.mem + i));
                            }
                        }
                        //
                    }
                }
            }
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_CANCEL_CLOSE_EXCH(int rt, char *chaps, const uint8_t *in, int len)// CMD_CANCEL_EXCH) || CMD_CLOSE_EXCH
{
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt;

    if (obin) {
        int tdl = bcd2bin(&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG))
            for (int i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl == sizeof(s_ack_req_stat_exch_err)) {//
            s_ack_req_stat_exch_err *ack_req_stat_exch_err = (s_ack_req_stat_exch_err *)obin;
            uint16_t ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tlen:%u/0x%04X\n\tack:0x%02X\n\tCRC16:%04X/%04X",
                                           ack_req_stat_exch_err->len, ack_req_stat_exch_err->len,
                                           ack_req_stat_exch_err->ack, ack_req_stat_exch_err->crc16, ccrc16);
            ret = (ack_req_stat_exch_err->ack & 0x7f);
            if (ret) ret *= -1;
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int ack_ACK_SRV(int rt, char *chaps, const uint8_t *in, int len)//CMD_ACK_SRV //45://"ack_srv"//Передать квитанцию от сервера ОФД//STX,‘SuL’,S(N),ETX,LRC (S[1..1024] - inBCD48)
{
/*typedef struct {
    uint8_t start;//0x04
    uint16_t len;//Длина строки от 4й до 6+N1 позиции включительно (N1+1)
    uint8_t ack;//Нулевой код ошибки (00H или 80H)
    uint8_t answer;//Код ответа ОФД (1)
//    uint8_t STLV[N];//Сообщение оператора для ККТ (может отсутствовать) | STLV
//    uint16_r ctc16;
} s_ack_ack_srv;*/
uint8_t *obin = (uint8_t *)calloc(1, len);
int ret = rt, i;


    if (obin) {
        int tdl = bcd2bin((const uint8_t *)&in[4], len - 6, obin);
        sprintf(chaps+strlen(chaps), "'%s' (%d):", __func__, tdl);
        if ((tdl) && (dbg >= LOG_DEBUG))
            for (i = 0; i < tdl; i++) sprintf(chaps+strlen(chaps), " %02X", obin[i]);
        if (tdl) {
            s_ack_ack_srv *ack_ack_srv = (s_ack_ack_srv *)obin;
            uint16_t icrc16; memcpy(&icrc16, &obin[tdl - 2], 2);
            uint16_t ccrc16 = CRC16(obin + 1, tdl - 3);
            sprintf(chaps+strlen(chaps), "\n\tstart:%u\n\tlen:%u\n\tack:0x%02X",
                                           ack_ack_srv->start,
                                           ack_ack_srv->len,
                                           ack_ack_srv->ack);
            if (ack_ack_srv->len > 1) sprintf(chaps+strlen(chaps), "\n\tanswerFN:0x%02X", ack_ack_srv->answer);
            sprintf(chaps+strlen(chaps), "\n\tCRC16:%04X/%04X", icrc16, ccrc16);
            ret = (ack_ack_srv->ack & 0x7f);
            if (ret) {
                ret *= -1;
                sprintf(chaps+strlen(chaps), "\n\tError :%s", ackSrvSTR(ret));
            } else {
                uint16_t len_stlv = ack_ack_srv->len;
                if (len_stlv > 2) {
                    len_stlv -=2;
                    uint8_t *uDat = obin + sizeof(s_ack_ack_srv);
                    if (dbg >= LOG_DEBUG) {
                        strcat(chaps, "\n");
                        for (i = 0; i < len_stlv; i++) sprintf(chaps+strlen(chaps), " %02X", *(uint8_t *)(uDat + i));
                    }
                }
            }
        }
        free(obin);
    }

    return ret;
}
//-----------------------------------------------------------------------
int makeMsg(uint8_t *msg, uint16_t mlen, uint8_t *out, uint8_t *sw, char *numFN)
{
/*
numFN - "request_statFN"
sw    - "get_ver_software"
typedef struct {//Структура заголовка Сообщения (Квитанции) сеансового уровня
    uint8_t sign[4];  //Сигнатура // BE //Константа ‘2A08410A’h, массив передается в порядке нумерации байтов
    uint8_t s_ver[2]; //Версия S-протокола// BE //Для версии протокола сеансового уровня 1.1 имеет значение: ‘81A2’h,
                      // массив передается в порядке нумерации байтов
    uint8_t p_ver[2]; //Версия P-протокола//Соответствует значению реквизита «номер версии ФФД» (тег 1209) из документа [2] и кодируется в виде:
                      // ‘0100’h – рекомендуется для версии ФФД 1.0 (допускаются также коды ‘0001’h и ‘0002’h);
                      // ‘0105’h - для версии ФФД 1.05;
                      // ‘0110’h - для версии ФФД 1.1, массив передается в порядке нумерации байтов
    char numFN[16];   //Номер ФН//Идентификатор Клиента ККТ в форме номера ФН
    uint16_t msg_len; //Размер тела //2 Int, LE//Содержит длину тела Сообщения (Контейнера).
    uint16_t flags;   //Флаги //2, Int, LE //Содержит флаги режима обработки Сообщения (см. Таблицу 2)
    uint16_t crc16;   //Проверочный код// 2, Int, LE
                      // Содержит контрольный проверочный код, вычисленный для Сообщения,
                      // включая заголовок (исключая данное поле).
                      // Режим вычисления проверочного кода определяется Флагами.
                      // Алгоритм вычисления контрольного проверочного кода: CRC16-CCITT.
                      // Заполняется нулями в случае отсутствия проверочного кода
} s_min_head;*/
int ret = 0;

    if (!numFN || !sw || !out) return ret;

    if (!strlen(numFN)) return ret;

    s_min_head *hdr = (s_min_head *)out;

    ret = sizeof(s_min_head);
    memcpy(out, &test_sess, ret);
    hdr->msg_len = 0;//mlen;
    memcpy(&hdr->p_ver, sw, 2);
    memcpy(&hdr->numFN, numFN, sizeof(hdr->numFN));
    //if (msg && (mlen > 0)) {
    //    memcpy(out + ret, msg, mlen);
    //    ret += mlen;
    //}

    return ret;

}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------


