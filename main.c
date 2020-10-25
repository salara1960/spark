#include "client.h"


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------


int main (int argc, char *argv[])
{
uint8_t to_dev[buf_size];
uint8_t from_dev[buf_size];
char dev_name[sdef] = {0};
char dev_cmd[sdef] = {0};
char dev_pwd[16] = {0};
char dev_arg[buf_size] = {0};
char chap[(buf_size << 2) + 1024];
char tmp[sdef];
char tmp_dev_arg[sdef] = {0};
int Vixod = 0, lenr = 0, lenr_tmp = 0, i, j, k = 1, rdy = 0, all_len = 0, ret = 0;
int lens = 0, inc = -1, incs = -1, incSave = -1;
struct timeval mytv;
fd_set Fds;
uint8_t byte = 0, yes, rx_stat = 0, faza = 0, cnt = 0;
struct sigaction Act, OldAct;
char *uk = NULL;
uint32_t tmr, tmr_cmd, wait_ack_sec = wait_ack_min_sec;
s_ack_stat12 *ack_stat12 = NULL;
s_ack_stat3  *ack_stat3  = NULL;
uint8_t inCRC, calcCRC, clr_done = 0;
uint8_t dev_spd = 4;//spd[dev_spd] = 57600
int8_t iList = -1;
uint8_t delIndex = 0;
const char *defPassword = "111111";

    setlocale(LC_ALL, "en_US.UTF-8");


    sprintf(tmp, "%s%s", path_log, file_log);
    fd_log = open(tmp, O_WRONLY | O_APPEND | O_CREAT, 0664);//open log file
    if (fd_log < 0) {
        ret = RET_MAJOR_ERROR;
        sprintf(chap, "[Ver.%s] %s Can't open %s file (return %d)\n", vers, TimeNowPrn(tmp), tmp, ret);
        ToSysLogMsg(LOG_INFO, chap);
        return ret;
    }

    // make symlink if not present
    sprintf(tmp, "ls | grep %s >>/dev/null", file_log);
    if (system(tmp)) {
        sprintf(tmp, "ln -s %s%s ./%s", path_log, file_log, file_log);
        system(tmp);
    }

    if (argc < 4) {
        ret = RET_MAJOR_ERROR;//-1;
        if (dbg != LOG_OFF)  print_msg(1, "ERROR: you must enter incomming options. For example: ./spark --dev=/dev/ttyUSB0 --cmd=set_speed --arg=9600 (return %d)\n\n", ret);
        close(fd_log);
        return ret;
    }


    while (k < argc) {
        yes = 0;
        strcpy(chap, argv[k]);
        for (i = 0; i < max_param; i++) {
            if ((uk = strstr(chap, name_param[i]))) {
                uk += strlen(name_param[i]);
                switch (i) {
                    case 0://"--dev="
                        strcpy(dev_name, uk);
                        cnt++;
                        yes = 1;
                    break;
                    case 1://"--cmd="
                        strcpy(dev_cmd, uk);
                        cnt++;
                        yes = 1;
                    break;
                    case 2://"--arg="
                        strcpy(dev_arg, uk);
                        cnt++;
                        yes = 1;
                    break;
                    case 3://"--log="
                             if (strstr(uk, "on"))     dbg = LOG_ON;
                        else if (strstr(uk, "debug"))  dbg = LOG_DEBUG;
                        else if (strstr(uk, "dump"))   dbg = LOG_DUMP;
                        yes = 1;
                    break;
                    case 4://"--speed="
                        dev_spd = findSPEED(uk);
                        yes = 1;
                    break;
                    case 5://"--mode="
                        if (strstr(uk, "list")) from_list = 1;
                        yes = 1;
                    break;
                    case 6://"--password="
                        if (strlen(uk) > 15) strncpy(dev_pwd, uk, 15);
                                        else strcpy(dev_pwd, uk);
                        yes = 1;
                    break;
                    case 7://"--timeout="//def_wait_ack_sec
                        j = atoi(uk);
                        if (j > wait_ack_min_sec) def_wait_ack_sec = j;
                        yes = 1;
                    break;
                }
            }
            if (yes) break;
        }
        k++;
    }
    if (cnt < 3) {
        ret = RET_MAJOR_ERROR;//-1;
        sprintf(chap, "Error: you must enter incomming options. For example: ./spark --dev=/dev/ttyUSB0 --cmd=set_speed --arg=9600 (return %d)\n\n", ret);
        if (dbg != LOG_OFF) print_msg(1, chap);
        close(fd_log);
        ToSysLogMsg(LOG_INFO, chap);
        return ret;
    }
    strcpy(device, dev_name);

    // remove 0x0A 0x0D from dev_arg
    uk = strchr(dev_arg, '\n');
    if (uk) {
        *uk = '\0';
        uk = strchr(dev_arg, '\r');
        if (uk) *uk = '\0';
    }


    inc = parse_inCMD(dev_cmd);
    if (inc < 0) {
        ret = RET_MINOR_ERROR;//1;
        if (dbg != LOG_OFF) print_msg(1, "Error: Unknown incomming command '%s' (return %d)\n", dev_cmd, ret);
        close(fd_log);
        return ret;
    }
    if ((inc == CMD_LINE0_PRINT) || (inc == CMD_LINE1_PRINT)) {//command "line*_print"
        if (!strlen(dev_arg)) strcpy(dev_arg, eol);//if arg="" set arg="\r\n"
    }


    if (dbg != LOG_OFF) print_msg(1, "[Ver.%s] Start spark with dev='%s' cmd[%d]='%s' arg(%d)='%s' dbg=%d list=%u timeout=%u\n",
                 vers, dev_name, inc, dev_cmd, strlen(dev_arg), dev_arg, dbg, from_list, def_wait_ack_sec);

    incSave = inc;

#ifdef SET_CLI_OFD
    char ofdURL[sdef] = {0};
    // ---   get ip:port OFD server   ---
    if (inc >= CMD_SEND_TEST) {
        i = strlen(dev_arg);
        if (i > sdef - 1) i = sdef - 1;
        if (i) strncpy(ofdURL, dev_arg, i);
    }
    //----------------------------------
#endif

    if (from_list) {
        initList();
        if (!strlen(dev_pwd)) strcpy(dev_pwd, defPassword);
        if ((inc == CMD_SHIFT_OPEN) ||
                (inc == CMD_SHIFT_CLOSE) ||
                    (inc ==  CMD_SET_CASHIER_NAME_PASSWORD)) {
            incs = CMD_SET_PASSWORD;
            iList = putToList(incs, dev_pwd);
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], dev_pwd, iList);
            incs = inc;
            iList = putToList(inc, dev_arg);
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], dev_arg, iList);
            //
            incs = inc;
            delIndex = 1;
            listIndex = getFromList(NULL, &inc, dev_arg, delIndex);//with del
            if (listIndex < 0) goto err_list;
                          else if (dbg > LOG_ON) print_msg(1, "Get from list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], dev_arg, listIndex);
            //
        }
#ifdef SET_CLI_OFD
        else if (inc == CMD_SEND_TEST) {//total_inCMD - 1) //"send_test"
            incs = CMD_GET_VER_SOFTWARE;
            strcpy(tmp_dev_arg, "empty");
            iList = putToList(incs, tmp_dev_arg);//"get_ver_software"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            incs = CMD_REQUEST_STATFN;
            //strcpy(tmp_dev_arg, "empty");
            iList = putToList(incs, tmp_dev_arg);//"request_statFN"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            incs = inc;
            strcpy(tmp_dev_arg, dev_arg);
            iList = putToList(inc, tmp_dev_arg);//"send_test"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            //
            incs = inc;
            delIndex = 1;
            iList = 0;
            listIndex = getFromList(&iList, &inc, dev_arg, delIndex);
            if (iList < 0) goto err_list;
            else {
                incs = inc;
                if (dbg > LOG_ON) print_msg(1, "Get from list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], dev_arg, listIndex);
            }
            //
        } else if (inc == CMD_SEND_MSG) {//"send_msg"
            //
            char *ui = strchr(dev_arg, '^');//arg=127.0.0.1:7778^255
            if (ui) {
                i = atoi(ui + 1);
                if ((i > 0) && (i < max_blk_size)) blkSize = i;
                *ui = '\0';
            }
            //
            incs = CMD_GET_STAT_EXCH;
            strcpy(tmp_dev_arg, "empty");
            iList = putToList(incs, tmp_dev_arg);//"get_stat_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);

            incs = CMD_BEFORE_EXCH;
            strcpy(tmp_dev_arg, "0");
            iList = putToList(incs, tmp_dev_arg);//"before_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);

            incs = CMD_BEFORE_EXCH;
            strcpy(tmp_dev_arg, "1");
            iList = putToList(incs, tmp_dev_arg);//"before_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);

            incs = CMD_BEGIN_READ_MSG;
            strcpy(tmp_dev_arg, "empty");
            iList = putToList(incs, tmp_dev_arg);//"begin_read_msg"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);

            incs = CMD_READ_MSG_BLOCK;
            sprintf(tmp_dev_arg, "0^%u", blkSize);
            iList = putToList(incs, tmp_dev_arg);//"read_msg_block"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            //
            incs = inc;
            strncpy(tmp_dev_arg, dev_arg, sdef - 1);
            iList = putToList(inc, tmp_dev_arg);//"send_msg"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            //
            incs = CMD_CLOSE_EXCH;
            strcpy(tmp_dev_arg, "empty");
            iList = putToList(incs, tmp_dev_arg);//"close_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            /**/
            incs = CMD_ACK_SRV;
            strcpy(tmp_dev_arg, "z");
            iList = putToList(incs, tmp_dev_arg);//"ack_srv"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            /**/
            incs = CMD_BEFORE_EXCH;
            strcpy(tmp_dev_arg, "0");
            iList = putToList(incs, tmp_dev_arg);//"before_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            //
            incs = CMD_GET_STAT_EXCH;
            strcpy(tmp_dev_arg, "emppty");
            iList = putToList(incs, tmp_dev_arg);//"get_stat_exch"
            if (iList < 0) goto err_list;
                      else if (dbg > LOG_ON) print_msg(1, "Add to list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], tmp_dev_arg, iList);
            //
            //--------
            //
            incs = inc;
            delIndex = 0;
            iList = 0;
            listIndex = getFromList(&iList, &inc, dev_arg, delIndex);
            if (iList < 0) goto err_list;
            else {
                incs = inc;
                if (dbg > LOG_ON) print_msg(1, "Get from list cmd='%s':'%s' (iList %d) OK\n", inCMD[incs], dev_arg, listIndex);
            }
            //
        }
#endif
    }


    fd = open(dev_name, O_RDWR , 0664);
    if (fd < 0) {
        ret = RET_MAJOR_ERROR;//-1;
        if (dbg != LOG_OFF) print_msg(1, "Error: Can't open device '%s' (return %d)\n", dev_name, ret);
        close(fd_log);
        return ret;
    }

    //--------------------  set Signals route function ------------------

    memset((uint8_t *)&Act,    0, sizeof(struct sigaction));
    memset((uint8_t *)&OldAct, 0, sizeof(struct sigaction));
    Act.sa_handler = &GetSignal_;
    Act.sa_flags   = 0;
    sigaction(SIGPIPE, &Act, &OldAct);
    sigaction(SIGSEGV, &Act, &OldAct);
    sigaction(SIGTERM, &Act, &OldAct);
    sigaction(SIGABRT, &Act, &OldAct);
    sigaction(SIGINT,  &Act, &OldAct);
    sigaction(SIGSYS,  &Act, &OldAct);
    sigaction(SIGKILL, &Act, &OldAct);
    sigaction(SIGTRAP, &Act, &OldAct);

    //-------------------------------------------------------------------

    setSPEED(dev_spd);//6-115200 // 4-57600 // 1-9600

    //-------------------------------------------------------------------

    byte = 0;
    faza = cnt = 0;
    lens = 0;
    tmr = 0;
    tmr_cmd = get_timer_sec(0);

    //--------------------------------------------------------------------
#ifdef SET_CLI_OFD
    int mlen = 0;
    uint8_t to_srv[buf_size] = {0};
    s_ses session = {NULL, NULL, 0};
    pthread_t tid = 0;
    pthread_attr_t threadAttr;
    if (inc >= CMD_SEND_TEST) {//total_inCMD - 1) //27 "send_test"
        pthread_attr_init(&threadAttr);
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    }
#endif
    //--------------------------------------------------------------------


    while (!Vixod) {

        switch (faza) {
            case 0:
                if (check_delay_sec(tmr_cmd)) {
                    clr_done = 1;
                    lens = makeCMD(to_dev, inc, dev_arg);
                    if (lens > 0) {//команда сформирована успешно
                        if (dbg >= LOG_DEBUG) {
                            sprintf(chap, "to device (%d): >", lens);
                            for (i = 0; i < lens; i++) sprintf(chap+strlen(chap), " %02X", to_dev[i]);
                            print_msg(1, "%s\n", chap);
                        }
                        if (write(fd, to_dev, lens) != lens) {
                            if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                            ret = RET_MINOR_ERROR;//1;
                            Vixod = 1;
                        } else {
                            faza = 1;
                            wait_ack_sec = def_wait_ack_sec;
                            tmr = get_timer_sec(wait_ack_sec);
                            tmr_cmd = 0;
                            cnt++;
                        }
                    } else {
#ifdef SET_CLI_OFD
                        if (inc >= CMD_SEND_TEST) {//total_inCMD - 1) //"send_test"
                            if (pthread_create(&tid, &threadAttr, cli_nitka, (void *)&session)) {
                                if (dbg != LOG_OFF) print_msg(1, "Error: Can't exec thread for '%s':'%s'\n", dev_cmd, dev_arg);
                                ret = RET_MAJOR_ERROR;//-1;
                                Vixod = 1;
                            } else {
                                cnt++;
                                faza = 1;
                                tmr_cmd = 0;
                                wait_ack_sec = def_wait_ack_sec;//wait_ack_net_sec;
                                tmr = get_timer_sec(wait_ack_sec);
                                cliSTART = 1;
                            }
                        } else {
                            if (dbg != LOG_OFF) print_msg(1, "Error: Can't make CMD for '%s':'%s'\n", dev_cmd, dev_arg);
                            ret = RET_MAJOR_ERROR;//-1;
                            Vixod = 1;
                        }
#else
                        if (dbg != LOG_OFF) print_msg(1, "Error: Can't make CMD for '%s':'%s'\n", dev_cmd, dev_arg);
                        ret = RET_MINOR_ERROR;//1;
                        Vixod = 1;
#endif
                    }
                }
            break;
            case 1://wait ack
                if (tmr) {
                    if (check_delay_sec(tmr)) {
                        if (dbg != LOG_OFF) print_msg(1, "Error: Timeout (%u sec) wait ack for '%s':'%s' \n", wait_ack_sec, dev_cmd, dev_arg);
                        if (inc == 26) ret = RET_MAJOR_ERROR;//"get_stat_exch"
                                  else ret = RET_TIMEOUT;//2;
                        Vixod = 1;
                    }
#ifdef SET_CLI_OFD
                    else if (inc >= CMD_SEND_TEST) {//total_inCMD - 1) //"send_test"
                        if (!cliSTART) {
                            ret = cliRET;
                            Vixod = 1;
                        }
                    }
#endif
                }
            break;
        }//switch(faza)

        if (!Vixod) {
            FD_ZERO(&Fds);
            FD_SET(fd, &Fds);
            mytv.tv_sec  = 0;
            mytv.tv_usec = 20000;
            if (select(fd + 1, &Fds, NULL, NULL, &mytv) > 0) {
                if (FD_ISSET(fd, &Fds)) {// rx_event from my device
                    //
                    lenr_tmp = read(fd, &byte, 1);
                    //
                    if (lenr_tmp <= 0) {
                        //Vixod = 1;
                        if (dbg != LOG_OFF) print_msg(1, "Error: Can't read from device '%s'\n", dev_name);
                        ret = RET_MINOR_ERROR;//1;
                        break;
                    } else if (lenr_tmp > 0) {
                        if (dbg == LOG_DUMP) print_msg(0, " %02X", byte);
                        if (clr_done) {
                            from_dev[lenr++] = byte;

                            if (byte == STX) {
                                rx_stat = 1;
                            } else if (byte == ACK) {//6
                                all_len = lenr;
                                ret = RET_OK;//0;
                            } else if (byte == NAK) {//21=0x15
                                all_len = lenr;
                                ret = RET_NAK;//3;
                            } if (byte == ENQ) {
                                all_len = lenr;
                                ret = RET_ENQ;//4;
                            } else if (byte == ETX) {
                                all_len = lenr + 1;
                            }
                        } else byte = 0;

                        if (lenr >= buf_size - 1) rdy = 1;
                        else {
                            if (lenr == all_len) rdy = 1;
                        }
                    }
                    if (rdy) {
                        if (dbg == LOG_DUMP) print_msg(0, "\n");
                        tmr = 0;
                        memset(chap, 0, sizeof(chap));
                        if (dbg >= LOG_DEBUG) {
                            sprintf(chap, "from device (%d): <", lenr);
                            for (i = 0; i < lenr; i++) sprintf(chap+strlen(chap), " %02X", from_dev[i]);
                            strcat(chap, "\n");
                        }
                        if (rx_stat) {
                            inCRC = calcCRC = 0;
                            if ((lenr == sizeof(s_ack_stat12)) && (inc != CMD_READ_MSG_BLOCK)) {
                                ack_stat12 = (s_ack_stat12 *)from_dev;
                                inCRC = ack_stat12->crc;
                                calcCRC = LRC(&ack_stat12->CT1, sizeof(s_ack_stat12) - 2);
                                if (byteENQ == ENQ) {
                                    parseSTATUS(chap, &ack_stat12->CT1, &ack_stat12->CT2, NULL, NULL);
                                } else {
                                    parseSTATUS(chap, NULL, NULL, &ack_stat12->CT1, &ack_stat12->CT2);
                                }
                                ret = (ack_stat12->CT2 << 8) | ack_stat12->CT1;
                                if (inCRC != calcCRC) ret |= 0x1000000;//+16777216
                            } else if ((lenr == sizeof(s_ack_stat3)) && (inc != CMD_READ_MSG_BLOCK)) {
                                ack_stat3 = (s_ack_stat3 *)from_dev;
                                inCRC = ack_stat3->crc;
                                calcCRC = LRC(&ack_stat3->CT1, sizeof(s_ack_stat3) - 2);

                                parseSTATUS(chap, &ack_stat3->CT1, &ack_stat3->CT2, &ack_stat3->CT3, NULL);

                                ret = (ack_stat3->CT3 << 16) | (ack_stat3->CT2 << 8) | ack_stat3->CT1;
                                if (inCRC != calcCRC) ret |= 0x1000000;//+16777216
                            } else {//------------  begin parser ack from dev  ------------------
                                memset(chap, 0, sizeof(chap));
                                if (inc == CMD_REQUEST_STATFN) {//24://ack for command "request_statusFN"
                                    ack_REQUEST_STATFN(chap, from_dev, lenr);
                                } else if (inc == CMD_GET_STAT_EXCH) {//26://ack for command "get_stat_exch"
                                    ret = ack_GET_STAT_EXCH(ret, chap, from_dev, lenr, &msgCount);
                                } else if (inc == CMD_GET_VER_SOFTWARE) {//33://"get_ver_software"//Считывание номера версии//STX,‘SV’,ETX,LRC
                                    ret = ack_GET_VER_SOFTWARE(chap, from_dev, &inCRC, &calcCRC);
                                } else if (inc == CMD_READ_REG_DATA) {//35://"read_reg_data"//Прочитать данные регистрации/перерегистрации//STX,‘Su#’,X,ETX,LRC
                                    ret = ack_READ_REG_DATA(ret, chap, from_dev, lenr);
                                } else if (inc == CMD_BEFORE_EXCH) {//"before_exch"//Передать статус транспортного соединения с Сервером ОФД//STX,‘SuG’,X,ETX,LRC
                                    ret = ack_BEFORE_EXCH(ret, chap, from_dev, lenr);
                                } else if (inc == CMD_GET_S1) {//"get_S1"//Считывание информации S1//STX,‘S1’,ETX,LRC
                                    ret = ack_GET_S1(chap, from_dev, &inCRC, &calcCRC);
                                } else if (inc == CMD_BEGIN_READ_MSG) {//38://"begin_read_msg"//Начать чтение Сообщения для Сервера ОФД//STX,‘SuH’,ETX,LRC
                                    ret = ack_BEGIN_READ_MSG(ret, chap, from_dev, lenr, &msgLen);
                                } else if (inc == CMD_INFO_ABOUT_DONE) {////40://"info_about_done"//Считывание информации по итогам смены (основные данные)//STX,‘Sf’или‘Sg’,ETX,LRC
                                    ret = ack_INFO_ABOUT_DONE(ret, chap, from_dev, lenr);
                                } else if (inc == CMD_EXTRA_INFO_ABOUT_DONE) {//41://"extra_info_about_done"//Считывание информации по итогам смены (дополнительные данные) (стр.35)//STX,‘Sh’или‘Si’,ETX,LRC
                                    ret = ack_EXTRA_INFO_ABOUT_DONE(ret, chap, from_dev, lenr);
                                } else if (inc == CMD_READ_MSG_BLOCK) {//42://"read_msg_block"//Прочитать блок сообщения для сервера ОФД//STX,‘SuI’,D[4],N[2],ETX,LRC (D,N - inBCD48)
                                    ret = ack_READ_MSG_BLOCK(ret, chap, from_dev, lenr);
                                } else if ((inc == CMD_CANCEL_EXCH) || //43://"cancel_exch"//Отменить чтение Сообщения для Сервера ОФД//STX,‘SuJ’,ETX,LRC
                                              (inc == CMD_CLOSE_EXCH)) {//44://"close_exch"//Завершить чтение Сообщения для Сервера ОФД//STX,‘SuK’,ETX,LRC
                                    ret = ack_CANCEL_CLOSE_EXCH(ret, chap, from_dev, lenr);
                                } else if (inc == CMD_ACK_SRV) {//45://"ack_srv"//Передать квитанцию от сервера ОФД//STX,‘SuL’,S(N),ETX,LRC (S[1..1024] - inBCD48)
                                    ret = ack_ACK_SRV(ret, chap, from_dev, lenr);
                                }
                                //----------  end of parser ack from dev  ------------------
                            }
                            if (inCRC != calcCRC) sprintf(chap+strlen(chap), "\n\tError CRC: 0x%02X/0x%02X", inCRC, calcCRC);
                            Vixod = 1;
                        }
                        if ((dbg != LOG_OFF) && strlen(chap)) print_msg(1, "%s\n", chap);

                        switch (ret) {
                            case RET_NAK://got NAK from device
                                if (cnt < max_try) {
                                    faza = 0;
                                    tmr = 0;
                                    tmr_cmd = get_timer_sec(wait_cmd_sec);
                                } else Vixod = 1;
                            break;
                            case RET_ENQ://got ENQ from device
                                to_dev[0] = ACK;
                                byteENQ = ENQ;
                                if (dbg >= LOG_DEBUG) print_msg(1, "to device (%d): > %02X\n", 1, to_dev[0]);
                                if (write(fd, to_dev, 1) != 1) {
                                    if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                                    ret = RET_MINOR_ERROR;//1;
                                    Vixod = 1;
                                } else {
                                    faza = 1;
                                    wait_ack_sec = def_wait_ack_sec;//wait_ack_min_sec;
                                    tmr = get_timer_sec(wait_ack_sec);
                                }
                            break;
                            case RET_OK://got ACK from device - > command is done
                                Vixod = 1;
                            break;
                        }//switch (ret)

                        if (!Vixod) {
                            memset(from_dev, 0, sizeof(from_dev));
                            rdy = all_len = 0;
                            lenr = lenr_tmp = 0;
                            rx_stat = 0;
                            byte = 0;
                        }
                    }//if (rdy)
                }//if (FD_ISSET(fd, &Fds))
            }//if (select
        }//if (!Vixod)

        if (QuitAll) { ret = RET_MINOR_ERROR; break; }
        else {//---------   for list mode   ----------------
            if (from_list && Vixod) {
                if (inc == CMD_SEND_MSG) {
                    if (cliRET != RET_OK) ret = RET_OK;
                } else if (inc == CMD_ACK_SRV) {
                    if (ret != RET_OK) ret = RET_OK;
                }
                if (ret == RET_OK) {
                    int8_t iList;
                    if ((inc == CMD_GET_STAT_EXCH) && !msgCount) {//no message for OFD
                       iList = -1;
                    } else {
                        if (inc == CMD_READ_MSG_BLOCK) {
                            if ((msgBuf.size > 0) && (msgBuf.size >= msgLen)) {
                                listIndex++;
                                if (listIndex == max_rec_list) listIndex = 0;
                            }
                        } else {
                            listIndex++;
                            if (listIndex == max_rec_list) listIndex = 0;
                        }
                        incs = listIndex;
                        iList = getFromList(&listIndex, &inc, dev_arg, delIndex);
                        if (inc == CMD_ACK_SRV) memset(dev_arg, 0, sizeof(dev_arg));
                        else
                        if (inc == CMD_READ_MSG_BLOCK) sprintf(dev_arg, "%lu^%u", msgBuf.size, blkSize);
                    }
                    if (iList < 0) {
                        if (dbg > LOG_DEBUG) print_msg(1, "List empty | msgBuf.size=%lu msgLen=%u msgDone=%u ackLen=%u ackDone=%u\n",
                                                           msgBuf.size, msgLen, msgDone, ackLen, ackDone);
                        Vixod = 1;
                    } else {
                        listIndex = iList;
                        if (dbg > LOG_ON) print_msg(1, "Get from list cmd='%s':'%s' (iList %d) OK\n", inCMD[inc], dev_arg, listIndex);
                        Vixod = 0;
                        memset(from_dev, 0, sizeof(from_dev));
                        rdy = all_len = 0;
                        lenr = lenr_tmp = 0;
                        rx_stat = 0;
                        byte = 0;
                        faza = cnt = 0;
                        lens = 0;
                        tmr = 0;
                        tmr_cmd = get_timer_sec(0);
                        //
#ifdef SET_CLI_OFD
                        if (inc >= CMD_SEND_TEST) {//total_inCMD - 1) //27 "send_test"
                            session.url = NULL;
                            i = strlen(ofdURL);
                            if (i) {
                                session.url = (char *)calloc(1, i + 1);
                                if (session.url) memcpy(session.url, ofdURL, i);
                            }
                            if (inc == CMD_SEND_TEST) { pmsgBuf = NULL;           mlen = sizeof(s_min_head); }
                                                 else { pmsgBuf = &msgBuf.mem[0]; mlen = msgBuf.size;        }
                            session.datalen = mlen;
                            session.data = (uint8_t *)calloc(1, session.datalen);// + 2);
                            if (session.data) {
                                if (inc == CMD_SEND_TEST) {
                                    makeMsg(NULL, 0, to_srv, swVer, numberFN);
                                    memcpy(session.data, to_srv, session.datalen);
                                } else memcpy(session.data, pmsgBuf, session.datalen);
                            } else session.datalen = 0;
                        }
#endif
                       //
                    }
                }
            }
        }
        //---------------------------------------------------
    }//while (!Vixod)

#ifdef SET_CLI_OFD
    if (incSave >= CMD_SEND_TEST) {//total_inCMD - 1)
        QuitCli = 1;
        lenr = 6;
        while (cliSTART) {
            usleep(500000);
            lenr--; if (!lenr) break;
        }
        pthread_attr_destroy(&threadAttr);
        if (cliRET != RET_OK) ret = cliRET;
        if (incSave == CMD_SEND_MSG) {
            if (ret > 0) ret *= -1;
            if (ret == RET_OK) ret = msgCount;
        }
    }
#endif

    if (from_list) delList();

    clearBuf(&msgBuf);
    clearBuf(&ackBuf);


    sprintf(chap, "[Ver.%s] Stop spark (return 0x%X/%d)\n", vers, (uint32_t)ret, ret);
    if (dbg != LOG_OFF) print_msg(1, chap);

    ToSysLogMsg(LOG_INFO, chap);

    close(fd_log);
    return ret;


err_list:

    if (from_list) delList();

    ret = RET_MINOR_ERROR;//1;
    if (dbg != LOG_OFF) print_msg(1, "Error: Can't add to list cmd='%s':'%s' (return %d)\n", inCMD[incs], tmp_dev_arg, ret);

    close(fd_log);
    return ret;

}
