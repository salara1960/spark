#include "functions.h"

//----------------------------------------------------------------------
char *statExchSTR(uint8_t fl, char *st)
{
    if (st) {
        if (fl & 1)    strcat(st, "\n\t  Транспортное соединение установлено");//bit0
        if (fl & 2)    strcat(st, "\n\t  Есть сообщение для передачи в ОФД");//bit1
        if (fl & 4)    strcat(st, "\n\t  Ожидание ответного сообщения (квитанции) от ОФД");//bit2
        if (fl & 8)    strcat(st, "\n\t  Есть команда от ОФД");//bit3
        if (fl & 0x10) strcat(st, "\n\t  Изменились настройки соединения с ОФД");//bit4
        if (fl & 0x20) strcat(st, "\n\t  Ожидание ответа на команду от ОФД");//bit5
    }

    return st;
}
//----------------------------------------------------------------------
static char *flagsSTR(uint8_t fl)
{

    switch (fl & 0x0f) {
        case 0:
            return "Настройка";
        case 1:
            return "Готовность к регистрации";
        case 3:
            return "Фискальный режим";
        case 7:
            return "Постфискальный режим, идет передача в ОФД";
        case 15:
            return "Чтение данных из Архива ФН";
    }

    return "???";
}
//----------------------------------------------------------------------
static char *aflagsSTR(uint8_t fl)
{

    if (fl & 0x80) return "Критическая ошибка ФН";

    switch (fl & 0x0F) {
        case 0:
            return "Нет предупреждений";
        case 1:
            return "Срочная замена КС (до окончания срока действия 3 дня)";
        case 2:
            return "Исчерпание ресурса КС (до окончания срока дей-ствия 30 дней)";
        case 4:
            return "Переполнение памяти ФН (Архив ФН заполнен на 90 %)";
        case 8:
            return "Превышено время ожидания ответа ОФД";
    }

    return "???";
}
//----------------------------------------------------------------------
static char *curDocSTR(uint8_t fl)
{

    switch (fl) {
        case 0:
            return "Нет открытого документа";
        case 1:
            return "Отчёт о регистрации ККТ";
        case 2:
            return "Отчёт об открытии смены";
        case 4:
            return "Кассовый чек";
        case 8:
            return "Отчёт о закрытии смены";
        case 0x10:
            return "отчёт о закрытии фискального режима";
        case 0x11:
            return "Бланк строкой отчетности";
        case 0x12:
            return "Отчет об изменении параметров регистрации ККТ в связи с заменой ФН";
        case 0x13:
            return "Отчет об изменении параметров регистрации ККТ";
        case 0x14:
            return "Кассовый чек коррекции";
        case 0x15:
            return "БСО коррекции";
        case 0x17:
            return "Отчет о текущем состоянии расчетов";
    }

    return "???";
}
//----------------------------------------------------------------------
static char *dataDocSTR(uint8_t fl)
{

    switch (fl) {
        case 0:
            return "Нет данных документа";
        case 1:
            return "Получены данные документа";
    }

    return "???";
}
//----------------------------------------------------------------------
static char *statShiftSTR(uint8_t fl)
{
    switch (fl) {
        case 0:
            return "Смена закрыта";
        case 1:
            return "Смена открыта";
    }

    return "???";
}
//-----------------------------------------------------------------------
static char *readBlkSTR(int rt)
{
    if (rt < 0) rt *= -1;

    switch (rt) {
        case 17:
            return "Отсутствует транспортное соединение";
        case 2:
            return "Неверное состояние ФН";
        case 1:
            return "Значение смещения больше, чем длина сообщения";
        case 3:
            return "Ошибка ФН, получить расширенные данные ошибки запросом 'Su5'";
        case 4:
            return "Ошибка КС, получить расширенные данные ошибки запросом Su5";
        case 5:
            return "Неверное CRC16, уведомить оператора причину, по которой сообщение от ОФД не может быть принято";
    }

    return "???";
}
//-----------------------------------------------------------------------
static char *ackSrvSTR(int rt)
{
    if (rt < 0) rt *= -1;

    switch (rt) {
	case 1:
	    return "Некорректный формат, или параметр команды";
	case 2:
	    return "Некорректное состояние ФН";
	case 3:
	    return "Авария ФН";
	case 4:
	    return "Авария КС ФН";
	case 5:
	    return "Исчерпан временной ресурс использования ФН";
	case 6:
	    return "ФН переполнен";
	case 7:
	    return "Неверные дата, или время";
	case 9:
	    return "Переполнение";
	case 12:
	    return "В режиме регистрации прихода эта операция запрещена";
	case 16:
	    return "Нет ответа от ФН";
	case 17:
	    return "Чужой ФН";
	case 20:
	    return "Неверная длина ответа от ФН";
	case 21:
	    return "Исчерпан лимит перерегистраций";
	case 22:
	    return "Активный чужой ФН";
	case 26:
	    return "Запрещенное состояние ФН";
	case 27:
	    return "ФН открыт";
	case 28:
	    return "Нет ФП в ответе ФН";
	case 29:
	    return "ФН содержит дефектные данные";
	case 34:
	    return "ФН не активизирован";
	case 38:
	    return "Исчерпан ресурс ФН";
	case 40:
	    return "Неверная длина команды";
	case 41:
	    return "Пароль не введен, или неверный";
	case 42:
	    return "Переполнение, в т.ч. отрицательное. значение";
	case 43:
	    return "ККТ закрыта";
	case 44:
	    return "В транзакции";
	case 45:
	    return "Нечисловая информация";
	case 46:
	    return "Кассир не установлен";
	case 47:
	    return "Неверный тип документа";
	case 48:
	    return "Переполнение количества строк";
	case 49:
	    return "Синтаксис команды неверный";
	case 50:
	    return "Подфункция не найдена";
	case 51:
	    return "Неверное плат. ср-во";
	case 52:
	    return "Запрещено для этого плат. ср-ва";
	case 53:
	    return "Неверный номер отдела";
	case 54:
	    return "Запрещено по программируемому флагу №5";
	case 55:
	    return "Не введены номер кредитной карты и документ авторизации";
	case 56:
	    return "Знак числа неверный";
	case 57:
	    return "Строка содержит непечатный символ";
	case 58:
	    return "Пустая строка";
	case 59:
	    return "Дата не установлена";
	case 60:
	    return "Дата меньше чем дата посл. Закрытия смены";
	case 61:
	    return "Регистрация невозможна";
	case 62:
	    return "Конец бумаги";
	case 63:
	    return "Не закрыта смена";
	case 64:
	    return "Не закрыта смена";
	case 68:
	    return "ККТ открыта";
	case 69:
	    return "Параметр уже запрограммирован";
	case 70:
	    return "Два последних символа должны быть пробелами";
	case 73:
	    return "Строка содержит запрещённые символы в имени файла";
	case 74:
	    return "Фиск режим уже запущен";
	case 75:
	    return "Этот пароль имеется у другого кассира";
	case 76:
	    return "Дата превышает предыдущую на 1 день";
	case 77:
	    return "Ввод даты не подтвердился";
	case 78:
	    return "В фискальном режиме запрещено";
	case 79:
	    return "В нефискальном режиме запрещено";
	case 81:
	    return "Нет учетной записи в чеке";
	case 82:
	    return "Ошибка обмена с ПК";
	case 83:
	    return "Коррекция на некорректируемую операцию";
	case 84:
	    return "Исчерпан список пл. средств";
	case 85:
	    return "Испорчена дата последнего отчёта";
	case 86:
	    return "Исчерпано время";
	case 87:
	    return "Испорчена таблица в памяти (fstatus)";
	case 89:
	    return "Значение выходит за пределы допустимого";
	case 90:
	    return "Неверная команда вне транзакции";
	case 91:
	    return "Нев. команда для регистрации приходов";
	case 92:
	    return "Нев. команда для ввода оплат";
	case 93:
	    return "Нев. команда внесения/выплаты из кассы";
	case 94:
	    return "Нев. команда режима печати текста";
	case 95:
	    return "Нев. команда регистрации аннулирования";
	case 96:
	    return "Нев. команда ввода сумм выдачи при аннулировании";
	case 97:
	    return "Нев. команда для неопознанного режима";
	case 98:
	    return "Запрещено программирование этого дескриптора";
	case 99:
	    return "Не поддерживается обработка этой ситуации";
	case 100:
	    return "Запрос не найден";
	case 101:
	    return "Ссылка на незапрограммированную ставку";
	case 103:
	    return "Ввод запрещенного слова";
	case 104:
	    return "Чек с нулевым итогом запрещен. Выполнена аварийная отмена чека";
	case 105:
	    return "Вводимая дата меньше даты посл. док-та в ФН";
	case 107:
	    return "Поднят рычаг принтера";
	case 108:
	    return "Печатающее устройство не обнаружено";
	case 109:
	    return "Регистрация невозможна";
	case 111:
	    return "ККТ закрыта и/или кассир не установлен";
	case 112:
	    return "Номер кассы не запрограммирован";
	case 117:
	    return "Запрещено по программируемому флагу №9";
	case 118:
	    return "Запрещено по программируемому флагу №10";
	case 119:
	    return "ИНН не запрограммирован";
	case 120:
	    return "Переполнение стека в режиме 0";
	case 121:
	    return "Переполнение стека в режиме 1";
	case 122:
	    return "Переполнение стека в режиме 2";
	case 123:
	    return "Переполнение стека в режиме 3";
	case 124:
	    return "Переполнение стека в режиме 4";
	case 125:
	    return "Переполнение стека в режиме 5";
	case 126:
	    return "Переполнение стека в режиме 6";
	case 127:
	    return "Переполнение стека в неопознанном режиме";
	case 128:
	    return "Ввод нулевого количества запрещен";
	case 129:
	    return "Cбой ОЗУ";
	case 130:
	    return "Нет итогов смены в ФН по запросу";
	case 131:
	    return "Частичное переполнение, в т.ч. отрицательное значение";
	case 132:
	    return "Открыт денежный ящик";
	case 133:
	    return "Чек с нулевым итогом запрещен. Невозможно выполнить аварийную отмену чека";
	case 138:
	    return "Не хватает денег в кассе для сдачи";
	case 140: case 141: case 142: case 143: case 144: case 145: case 146: case 147:
	    return "Не хватает денег по (ERROR-140+1) платёжному средству для сдачи";
	case 161:
	    return "Биты 4..6 флага 4 защищены PF13:0 от модификации, или бит 5 флага 3 защищен от сброса";
	case 162:
	    return "Переполнение регистрационного буфера";
	case 163:
	    return "Подфункция 2-го порядка не найдена";
	case 164:
	    return "Программирование нулевого пароля запрещено";
	case 165:
	    return "Ненулевой остаток в кассе";
	case 168:
	    return "Глубина дерева налогов слишком велика, дерево содержит цикл или обратную ссылку";
	case 169:
	    return "Предупреждение: ставки налогов во включенном режиме цепочек не программируются";
	case 170:
	    return "Неверный номер ставки в дереве налогов";
	case 171:
	    return "Дерево налогов не запрограммировано";
	case 172:
	    return "Дерево налогов состоит более, чем из одной компоненты";
	case 173:
	    return "Режим налоговых цепочек не включён";
	case 179:
	    return "Заголовок чека не запрограммирован";
	case 180:
	    return "Выполнение отчёта прервано";
	case 181:
	    return "ККТ заблокирована для перерегистрации";
	case 182:
	    return "Невозможно закрыть смену в ФН";
	case 183:
	    return "Невозможно снять запрос состояния тип 2";
	case 184:
	    return "Не заданы параметры вводимого графического заголовка";
	case 185:
	    return "Ввод графического заголовка не доведен до конца";
	case 186:
	    return "Размер этикетки с номером >31 не должен превышать 3*24";
	case 187:
	    return "Ложное выключение питания";
	case 192:
	    return "Процедура подготовки к выключению питания не была завершена";
	case 197:
	    return "Время не установлено";
	case 200:
	    return "Нулевое количество и цена в итоге чека";
	case 201:
	    return "По ставке налогов расход не выполняется";
	case 202:
	    return "Операции расхода по карте запрещены";
	case 203:
	    return "Переплата сдачи клиенту";
	case 207:
	    return "Сдача по карте запрещена";
	case 208:
	    return "Предупреждение: установлено только одно наличное пл. ср-во №8";
	case 210:
	    return "Допустимые команды: Либо завершение чека, либо отмена";
	case 211:
	    return "Чек не сформирован";
	case 212:
	    return "Неверная контрольная сумма серийного номера";
	case 213:
	    return "Неверная контрольная сумма фиск. Реквизитов";
	case 214:
	    return "Количество значащих символов меньше трёх";
	case 215:
	    return "Скорость обмена с ПК должна превышать скорость обмена с принтером";
	case 219:
	    return "ККТ заблокирована";
	case 226:
	    return "Близок конец бумаги";
	case 227:
	    return "Переполнение буфера реквизитов";
	case 230:
	    return "Нет ответа от SRAM";
	case 231:
	    return "Невозможно очистить буфер USART1";
	case 232:
	    return "Некорректное значение стека после выключения питания";
	case 233:
	    return "Принтер не готов";
	case 235:
	    return "Примечание: изменения вступят в силу после выключения питания";
	case 241:
	    return "Превышение итога документа";
	case 247:
	    return "Нет напряжения питания на принтере (выключен тумблер)";
	case 248:
	    return "Некорректные параметры обмена принтера (DSW1)";
	case 250:
	    return "Выключение питания";
	case 281:
	    return "Активация ФН недопустима";
	case 282:
	    return "Неверное состояние ФН";
	case 283:
	    return "Превышено время ожидания передачи";
	case 284:
	    return "Разница во времени >5 минут";
	case 285:
	    return "Невозможно открыть документ";
	case 286:
	    return "Отказ в передаче данных документа";
	case 287:
	    return "Невозможно завершить документ";
	case 288:
	    return "Неверная контрольная сумма рег. номера";
	case 289:
	    return "Отсутствуют регистрационные данные";
	case 290:
	    return "Не введён номер автомата";
	case 291:
	    return "Выбранная система налогообложения не была задана при регистрации";
    }

    return "???";
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
