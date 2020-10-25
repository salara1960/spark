#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <termios.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <pthread.h>

#include <locale.h>

#include <iconv_hook/iconv.h>
#include <iconv_hook/iconv_hook.h>


//-----------------------------------------------------------------------


#define SET_CLI_OFD


#ifdef SET_CLI_OFD
    #define SET_PRN_MUTEX
#endif



#define buf_size     2048
#define sdef          256
#define TIME_STR_LEN   64

#define STX  0x02
#define ETX  0x03
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define NAK  0x15
#define ENQ2 0x11
#define ENQT 0x1A

//#define wait_ack_lit_sec  3
#define wait_ack_min_sec  3
//#define wait_ack_max_sec 10
//#define wait_ack_net_sec 15

#define wait_cmd_sec      2
#define max_try           3
#define max_print_line   56
#define max_spd           7

#define max_rec_list     16

#define max_blk_size    255//117
#define bin_len 3
#define bcd_len (bin_len << 1)

//-----------------------------------------------------------------------

#define get_timer_sec(tm) (time(NULL) + tm)
#define check_delay_sec(tm) (time(NULL) >= tm ? 1 : 0)

//-----------------------------------------------------------------------
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

typedef enum {
    RET_MAJOR_ERROR = -1,
    RET_OK,//0
    RET_MINOR_ERROR,//1
    RET_TIMEOUT,//2
    RET_NAK,//3
    RET_ENQ//4
} res_t;

typedef enum {
    LOG_OFF = 0,//0
    LOG_ON,//1
    LOG_DEBUG,//2
    LOG_DUMP//3
} log_t;

#define totals 48

typedef enum {
    CMD_SET_SPEED = 0,            //"set_speed"
    CMD_SET_PASSWORD,             //"set_password"
    CMD_SHIFT_OPEN,               //"shift_open"
    CMD_SHIFT_CLOSE,              //"shift_close"
    CMD_SET_CASHIER_NUMBER,       //"set_cashier_number"
    CMD_SET_CASHIER_NAME_PASSWORD,//"set_cashier_name_password"
    CMD_REGISTER_CASHIER,         //"register_cashier"
    CMD_RESET_CASHIER,            //"reset_cashier"
    CMD_CLEAR_ERROR,              //"clear_error"
    CMD_LINE0_PRINT,              //"line0_print"
    CMD_LINE1_PRINT,              //"line1_print"
    CMD_END_OF_PRINT,             //"end_of_print"
    CMD_REQUEST_STATUS,           //"request_status"
    CMD_RESTART_DEVICE,           //"restart_device"
    CMD_GENERAL_REPORT,           //"general_report"
    CMD_SET_COMING_ITEM,          //"set_coming_item"
    CMD_SET_RETURN_ITEM,          //"set_return_item"
    CMD_RETURN_ITEM,              //"return_item",
    CMD_STORNO_RETURN_ITEM,       //"storno_return_item"
    CMD_END_OF_RETURN_ITEM,       //"end_of_return_item"
    CMD_REG_ITEM,                 //"reg_item"
    CMD_STORNO_ITEM,              //"storno_item"
    CMD_END_OF_CHEK,              //"end_of_chek"
    CMD_END_OF_PAY,               //"end_of_pay"
    CMD_REQUEST_STATFN,           //"request_statFN"
    CMD_ERROR_RESET,              //"error_reset"
    CMD_GET_STAT_EXCH,            //"get_stat_exch"
    CMD_TEXT_REPORT0_PRINT,       //"text_report0_print"
    CMD_TEXT_REPORT1_PRINT,       //"text_report1_print"
    CMD_EXTRA_LINES_PRINT,        //"extra_lines_print"
    CMD_CANCEL_CHEK,              //"cancel_chek"
    CMD_LAST_DOC_PRINT,           //"last_doc_print"
    CMD_LAST_DOC_BIN,             //"last_doc_bin"
    CMD_GET_VER_SOFTWARE,         //"get_ver_software"
    CMD_SET_TEXT_ATTR,            //"set_text_attr"
    CMD_READ_REG_DATA,            //"read_reg_data"
    CMD_BEFORE_EXCH,              //"before_exch"
    CMD_GET_S1,                   //"ge_S1"
    CMD_BEGIN_READ_MSG,           //"begin_read_msg"
    CMD_CLOSE_CHEK,               //"close_chek"
    CMD_INFO_ABOUT_DONE,          //"info_about_done"
    CMD_EXTRA_INFO_ABOUT_DONE,    //"extra_info_about_done"
    CMD_READ_MSG_BLOCK,           //"read_msg_block"
    CMD_CANCEL_EXCH,              //"cancel_exch"
    CMD_CLOSE_EXCH,               //"close_exch"
    CMD_ACK_SRV                   //"ack_srv"
#ifdef SET_CLI_OFD
    ,CMD_SEND_TEST                //"send_test"
    ,CMD_SEND_MSG                 //"send_msg"
#endif
} cmd_t;

#pragma pack(push,1)
typedef struct {
    int cmd;
    char *arg;
} s_one_cmd;
typedef struct {
    s_one_cmd work[max_rec_list];
} s_list;
#pragma pack(pop)


typedef struct {
  uint8_t *mem;
  size_t size;
} s_buf;


#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[2];
    uint8_t spd;
    uint8_t etx;
    uint8_t crc;
} s_cmd_spd;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[3];
    char pwd[6];
    uint8_t etx;
    uint8_t crc;
} s_cmd_pwd;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'o'
    char cnum[2];//'01'..'99'
    char cpwd[5];//'00000'..'99999'
    uint8_t etx;
    uint8_t crc;
} s_cmd_open;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {//STX,‘zz’,ETX,LRC
    uint8_t stx;
    char txt[2];//'zz'
    uint8_t etx;
    uint8_t crc;
} s_cmd_close;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[2];//'PN'
    char cnum[2];//'01'..'99'
    uint8_t etx;
    uint8_t crc;
} s_cmd_cash_num;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[2];//'PC'
    char ccode[2];//'01'..'16'
    char cpwd[5];//'00000'..'99999'
//    char cname[35];
//    uint8_t etx;
//    uint8_t crc;
} s_cmd_cash_pn;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'5'
    char cpwd[5];//'00000'..'99999'
    uint8_t etx;
    uint8_t crc;
} s_cmd_reg_cash;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'6'
    uint8_t etx;
    uint8_t crc;
} s_cmd_clr_cash;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'k'
    uint8_t etx;
    uint8_t crc;
} s_cmd_clr_err;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[3];//'Pp0'
    uint8_t etx;
    uint8_t crc;
} s_cmd_end_print;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    uint8_t CT1;
    uint8_t CT2;
    uint8_t etx;
    uint8_t crc;
} s_ack_stat12;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'D'
    uint8_t etx;
    uint8_t crc;
} s_cmd_report;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    uint8_t CT1;//CT1
    uint8_t CT2;//CT2
    uint8_t CT3;//CT3
    uint16_t flagsFN;//Флаги ФН (*)
    uint16_t codeErr;//Код ошибки
    uint8_t openDocType;//Тип открытого документа
    uint8_t F[3];//F
    uint8_t N1;//N1
    uint8_t N2;//N@
    uint16_t PPI0CD;//PPI0CD
    uint16_t PSTATUS;//PSTATUS
    uint8_t D1[3];//D1
    uint16_t T1;//T1
    uint16_t codeUnknownErr;//Код неопознан-ной ошибки ФН (*)
    uint8_t Sm;//Sm
    uint8_t etx;
    uint8_t crc;
} s_ack_stat3;
#pragma pack(pop)

//    "set_coming_item"//Ввод и печать номера заказа//STX,‘g’,НОМЕР[3],ETX,LRC
//    "set_return_item"//Ввод и печать номера заказа//STX,'i',НОМЕР[3],ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'g' or 'i'
    char num[3];
    uint8_t etx;
    uint8_t crc;
} s_cmd_set_item;
#pragma pack(pop)

//    "return_item"//Возврат товара//STX,0x64,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
//    "storno_return_item"//Возврат товара//STX,0xE4,X,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    uint8_t cmd;//0x64 or 0xE4
    char type;//'a'..'f'
    char price[8];
    char quantity[8];
    char dep[2];
//    char name[N];
//    uint8_t etx;
//    uint8_t crc;
} s_cmd_ret_item;
#pragma pack(pop)

// "enf_of_return_item"//Завершение операции возврата товара//STX,‘f’,ВИД ОПЛАТЫ[1],СУММА[10],ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'f'
    char type;//ВИД ОПЛАТЫ[1] - '1'..'8'
    char summa[10];//СУММА[10]
    uint8_t etx;
    uint8_t crc;
} s_cmd_end_ret_item;
#pragma pack(pop)

// "reg_item",//Приход : Операции регистрации товарной позиции//STX,CMD,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
// "storno_item"//Сторно : Операции сторнирования товарной позиции//STX,CMD,Цена[8],Количество[8],Отдел[2],Наименование[N],ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    uint8_t cmd;//(0x20..0x25 - "reg_item") or (0xEA0..0xA5 - "storno_item")
    char price[8];
    char quantity[8];
    char dep[2];
//    char name[N];
//    uint8_t etx;
//    uint8_t crc;
} s_cmd_reg_item;
#pragma pack(pop)

// "end_of_chek"// Завершение операции (чека) //STX,‘1’,ВИД ОПЛАТЫ[1],ETX,LRC
// "end_of_pay" // Завершение операции оплаты"//STX,‘K’,ВИД ОПЛАТЫ[1],ETX,LRC
                 //ВИД ОПЛАТЫ: Наличные ‘8’, Кредит ‘7’, Чек ‘6’, Платёжные карты ‘1’~’5’
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'1' or 'K'
    char type;//ВИД ОПЛАТЫ[1] - '1'..'8'
    uint8_t etx;
    uint8_t crc;
} s_cmd_end_of_proc;
#pragma pack(pop)

// "request_statFN"//Запрос состояния ФН//STX,‘S”’,ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[2];//'S"'
    uint8_t etx;
    uint8_t crc;
} s_cmd_req_statFN;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//0x001f
    uint8_t zero;
    uint8_t flags;//bits 0,1,2,3; 4..7 - not used
    uint8_t curDoc;
    uint8_t dataDoc;
    uint8_t statShift;
    uint8_t alarmFlags;
    uint8_t datetime[5];
    char numberFN[16];
    uint32_t numberFD;
    uint16_t crc16;
} s_ack_req_statFN;
#pragma pack(pop)

// "error_reset"//Сброс ошибки (конец ленты или сбой принтера)//STX,‘e',ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt;//'e'
    uint8_t etx;
    uint8_t crc;
} s_cmd_err_reset;
#pragma pack(pop)

// "get_stat_exch"//Получить статус информационного обмена//STX,‘SuF’,ETX,LRC
#pragma pack(push,1)
typedef struct {
    uint8_t stx;
    char txt[3];//'SuF','Su]','SuH'
    uint8_t etx;
    uint8_t crc;
} s_cmd_get_stat_exch;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//0x000E
    uint8_t ack;//Код ответа: 0 or 0x80
    uint8_t stat;//Статус информационного обмена:
    uint8_t rdBegin;//Начато чтение сообщения для ОФД (1 – да, 0 – нет)
    uint16_t totalMsg;//Количество сообщений для передачи в ОФД
    uint32_t first;//Номер документа для ОФД, первого в очереди
    uint8_t datetime[5];//Дата-время первого в очереди документа для ОФД
    uint16_t crc16;//Контрольная сумма
} s_ack_req_stat_exch;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//0x0001
    uint8_t ack;//Код ответа: 0x02 or 0x03
    uint16_t crc16;//Контрольная сумма
} s_ack_req_stat_exch_err;
#pragma pack(pop)



#pragma pack(push,1)
typedef struct {
    uint8_t stx;//0x02
    char txt[2];//'SV'
    char data[35];//Код ответа: 0 or 0x80
    uint8_t ext;//0x03
    uint8_t crc;
} s_ack_get_ver;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint32_t hdr;//Заголовок сообщения
    uint8_t datetime[5];//Дата и время
    char inn[12];//ИНН
    char reg_num_dev[20];//Регистрационный номер ККТ
    uint8_t code1;//Код налогообложения
    uint8_t mode;//Режим работы
    uint8_t code2;//Код причины перерегистрации
    uint32_t numFD;//Номер ФД
    uint32_t fflag;//Фискальный признак
    uint16_t crc16;//?
} s_ack_read_reg_data;
#pragma pack(pop)


//STX,‘S1’,ДАННЫЕ (114 байт),ETX,LRC
#pragma pack(push,1)
typedef struct {
    char hour[2];
    char min[2];
    char sec[2];
} s_time6;
typedef struct {
    char day[2];
    char mon[2];
    char year[2];
} s_date6;
typedef struct {
    uint8_t start;//stx
    char txt[2];//S1
    char cashier_code[3];//2+1//Код кассира
    char prixod[17];//16+1//Знак + общий итог приходов (*)
    char sch_fd[9];//8+1//Счетчик фискальных документов (*)
    char sch_rec[5];//4+1//Счетчик записей в ФН (смен)
    char sch_nfd[9];//8+1//Счетчик нефискальный документов
    char sch_ind_doc[9];//8+1//Счетчик порядковых номеров документов
    char id_owner[13];//12+1//Идентификационный код владельца ККТ
    char reg_num_dev[21];//20+1//Регистрационный номер ККТ
    char num_dev[15];//14+1//Серийный (заводской) номер ККТ
    s_time6 time;//char time[7];//6+1//Текущее время: часы (2) + минуты (2) + секунды (2)
    char sep;//0x0a
    s_date6 date;//char date[6];//6//Текущая дата: день (2) + месяц (2) + год (2)
    uint8_t ext;
    uint8_t crc;
} s_ack_get_s1;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//Длина строки от 4й до 6 позиции включительно (03H)
    uint8_t ack;//Нулевой код ошибки (00H или 80H)
    uint16_t len_msg;//Длина Сообщения в байтах (Nmax)
    uint16_t crc16;//Контрольная сумма
} s_ack_begin_read_msg;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    short S1;
    uint32_t S2;
} s_int6a;// S = S1 * 232 + S2
typedef struct {
    uint16_t quantity;
    s_int6a sum;
} s_ILongS;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint16_t shift_num;//Текущий номер смены
    s_int6a money[8];//Всего денежных средств по платёжному средству №1..№8
    s_ILongS inputs;//Сменный / общий итог приходов (***)
    s_ILongS items;//Сменный / общий итог товарных позиций (***)
    s_ILongS ret_inputs;//Сменный / общий итог возвратов приходов (***)
    s_ILongS ret_items;//Сменный / общий итог возвратов товарных позиций (***)
    s_ILongS plus;//Сменный / общий итог внесений
    s_ILongS minus;//Сменный / общий итог выплат
    s_int6a balance;//Остаток в кассе на начало смены
} s_ack_info_about_done;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint16_t shift_num;//Текущий номер смены
    s_int6a money[4];//Сумма рассчитанных налогов по ставке №1..№4
    s_ILongS cancel;//Отмена – количество и сумма
    s_ILongS emergency_cancel;//Аварийная отмена – количество и сумма
    s_ILongS storno;//Сторно – количество и сумма
    s_ILongS correction;//Коррекция – количество и сумма
    s_ILongS dup_chek;//Дубликаты чека – количество и сумма
    s_ILongS extra_charge_plus;//Наценка приходов – количество и сумма
    s_ILongS discount_plus;//Скидка приходов – количество и сумма
    s_ILongS extra_charge_minus;//Наценка расхода – количество и сумма
    s_ILongS discount_minus;//Скидка расхода – количество и сумма
} s_ack_extra_info_about_done;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//Длина строки от 4й до 6 позиции включительно (03H)
    uint8_t ack;//Нулевой код ошибки (00H или 80H)
//    uint8_t bytes[<=117];
//    uint16_r ctc16;
} s_ack_read_msg_block;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t start;//0x04
    uint16_t len;//Длина строки от 4й до 6+N1 позиции включительно (N1+1)
    uint8_t ack;//Нулевой код ошибки (00H или 80H)
    uint8_t answer;//Код ответа ОФД (1)
//    uint8_t STLV[N];//Сообщение оператора для ККТ (может отсутствовать) | STLV
//    uint16_r ctc16;
} s_ack_ack_srv;
#pragma pack(pop)



#define FLAG_CALC_NO  0x0000
#define FLAG_CONT_NO  0x0000
#define FLAG_FEAT_ACK 0x0010
#define FLAG_PRIO     0x0000

#define FLAGI (FLAG_CALC_NO | FLAG_CONT_NO | FLAG_FEAT_ACK | FLAG_PRIO)

#pragma pack(push,1)
typedef struct {
    unsigned calc:2; //Режим вычисления проверочного кода:
                     // 00 – проверочный код не вычисляется.
                     // 01 – проверочный код вычисляется по заголовку Сообщения.
                     // 10 – проверочный код вычисляется по заголовку и телу Сообщения.
                     // 11 – запрещенное значение кода
    unsigned cont:1; //Флаг передачи Контейнеров:
                     // 0 – тело Сообщения не содержит Контейнер.
                     // 1 – тело Сообщения содержит Контейнер
    unsigned rfu:1;  //RFU (зарезервировано, значение 0)
    unsigned feat:2; //Флаг функциональных возможностей Клиента ККТ:
                     // 00 – Клиент не ожидает получение ответа на переданный Контейнер.
                     // 01 – Клиент ожидает получение ответа на переданный Контейнер.
                     // 10, 11 – запрещенное значение кода
    unsigned prio:2; //Уровень приоритета Сообщения:
                     // 00 – нормальный приоритет Сообщения.
                     // Остальные значения – уровень приоритета Сообщения (в порядке возрастания, выше нормального приоритета)
    uint8_t zero;    //15–8 // RFU (зарезервировано, заполнено двоичными нулями)
} s_flags_head;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {//Структура заголовка Сообщения (Квитанции) сеансового уровня
    uint8_t start;
    uint16_t blen;
    uint8_t ack;

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
} s_head;
#pragma pack(pop)

#pragma pack(push,1)
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
} s_min_head;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {//Структура заголовка Контейнера
    uint16_t len;//Длина//2, Int, LE//Содержит размер данных Контейнера (размер данных А-объекта в формате передачи).
                 // Максимальная длина данных Контейнера – 32 Кб
    uint16_t crc16;//Проверочный код//2, Int, LE // Содержит контрольный проверочный код, вычисленный для Контейнера,
                   // включая заголовок (исключая это поле).Алгоритм вычисления контрольного проверочного кода: CRC16-CCITT
    uint8_t type;//Тип Контейнера//1//Константа A5h/5Ah, для обработки в ФН и ПКЗ Сервера
    uint8_t data_type;//Тип данных Контейнера//1//Код типа данных, содержащихся в Контейнере, для обработки в ПКЗ Сервера
    uint8_t ver;//Версия формата Контейнера//1
                // Версия формата Контейнера определяет длину заголовка и режим его обработки в ФН и ПКЗ Сервера.
                // Принимает значения: ‘0x’h – заголовок длиной 32 байта
//    uint8_t sd[N];//Служебные данные Контейнера//(переменная)
//                  // Служебные данные заголовка Контейнера для обработки в ФН и ПКЗ Сервера.
//                  // Длина поля определяется с учетом значения версии формата Контейнера, указанной в соответствующем поле заголовка
} s_cont;
#pragma pack(pop)



#ifdef SET_PRN_MUTEX
    extern pthread_mutex_t prn_mutex;
#endif


//-----------------------------------------------------------------------

extern const char *vers;
extern const char *eol;

extern uint8_t from_list;
extern s_list list;
extern int8_t listIndex;
extern uint8_t *pmsgBuf;

extern uint8_t blkSize;
extern uint8_t msgDone;
extern uint16_t msgCount;
extern uint16_t msgLen;
extern uint16_t msgLenBlk;
extern s_buf msgBuf;

extern uint8_t ackDone;
extern uint16_t ackLen;
extern s_buf ackBuf;

extern uint32_t def_wait_ack_sec;

extern const uint8_t def_swVer[2];
extern uint8_t swVer[2];
extern char numberFN[17];

extern s_min_head test_sess;

#ifdef SET_CLI_OFD
    extern int8_t QuitCli;
    extern int cliRET;
    extern int cliSTART;
#endif

extern uint8_t dbg;

extern int fd;
extern int fd_log;

extern uint8_t QuitAll;
extern uint8_t SIGTERMs;
extern uint8_t SIGINTs;
extern uint8_t SIGKILLs;
extern uint8_t SIGSEGVs;
extern uint8_t SIGABRTs;
extern uint8_t SIGSYSs;
extern uint8_t SIGTRAPs;

extern const char *path_log;
extern const char *file_log;

extern char device[sdef];

extern int max_size_log;
extern int MaxLogLevel;

extern uint32_t SPEED;
extern uint8_t byteENQ;

extern const int total_inCMD;
extern const char *inCMD[];

extern const uint8_t rst_dev[];

extern const int max_param;
extern const char *name_param[];

extern const uint32_t ispd[];

//----------------------------------------------------------------------

extern int bcd2bin(const uint8_t *bcd, int len, uint8_t *bin);
extern int bin2bcd(const uint8_t *bin, int len, uint8_t *bcd);

extern char *ThisTime();
extern uint8_t LRC(const uint8_t *buf, int len);
extern uint16_t CRC16(const uint8_t *buf, int len);

extern char *TimeNowPrn(char *ts);
extern void ToSysLogMsg(int LogLevel, const char * const Msg);
extern void print_msg(uint8_t dt, const char *fmt, ...);
extern void GetSignal_(int sig);

extern uint8_t findSPEED(char *param);
extern int deconv_text(char *in, char *out, int len);
extern int parse_inCMD(char *in_cmd);
extern void setSPEED(uint8_t SpeedIndex);
extern int makeCMD(uint8_t *buf, int idx, char *arg);

extern void parseSTATUS(char *str, uint8_t *ST1, uint8_t *ST2, uint8_t *ST3, uint8_t *ST4);

extern size_t addToBuf(s_buf *buf, uint8_t *data, size_t size);
extern void clearBuf(s_buf *buf);

extern void ack_REQUEST_STATFN(char *chaps, const uint8_t *in, int len);
extern int ack_GET_STAT_EXCH(int rt, char *chaps, const uint8_t *in, int len, uint16_t *cMsg);
extern int ack_GET_VER_SOFTWARE(char *chaps, const uint8_t *in, uint8_t *icrc, uint8_t *ccrc);
extern int ack_READ_REG_DATA(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_BEFORE_EXCH(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_GET_S1(char *chaps, const uint8_t *in, uint8_t *icrc, uint8_t *ccrc);
extern int ack_BEGIN_READ_MSG(int rt, char *chaps, const uint8_t *in, int len, uint16_t *lMsg);
extern int ack_INFO_ABOUT_DONE(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_EXTRA_INFO_ABOUT_DONE(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_READ_MSG_BLOCK(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_CANCEL_CLOSE_EXCH(int rt, char *chaps, const uint8_t *in, int len);
extern int ack_ACK_SRV(int rt, char *chaps, const uint8_t *in, int len);

extern void initList();
extern void delList();
extern int8_t putToList(int cmd, const char *arg);
extern int8_t getFromList(int8_t *id, int *cmd, char *arg, uint8_t del);

extern int makeMsg(uint8_t *msg, uint16_t mlen, uint8_t *out, uint8_t *sw, char *numFN);

//----------------------------------------------------------------------

#endif
