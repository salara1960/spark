#include "client.h"


#ifdef SET_CLI_OFD

//---------------------------------------------------------------------------------------------------
void closeTCP(int *sc)
{
int s = *sc;

    if (s > 0) {
        shutdown(s, SHUT_RDWR);
        close(s);
        *sc = -1;
    }
}
//---------------------------------------------------------------------------------------------------
int openTCP(const char *eurl)
{
int esoc = -1;
uint16_t t_p, tcp_port = 7778;
struct sockaddr_in srv_conn;
socklen_t srvlen;
struct hostent *hostPtr = NULL;
char line[max_url_len + 1] = {0};
char chap[512];

    if (!eurl) return esoc;

    esoc = socket(AF_INET, SOCK_STREAM, 6);
    if (esoc < 0) {
        if (dbg != LOG_OFF) print_msg(1, "[%s] FATAL ERROR: open socket (%d)\n", __func__, esoc);
        return -1;
    }

    strncpy(line, eurl, max_url_len);
    char *uk = strchr(line, ':');
    if (uk) {
        t_p = atoi(uk + 1);
        if (t_p > 0) tcp_port = t_p;
        *uk = '\0';
    }

    hostPtr = gethostbyname(line);
    if (!hostPtr) {
        hostPtr = gethostbyaddr(line, strlen(line), AF_INET);
        if (!hostPtr) {
            if (dbg != LOG_OFF) print_msg(1,"[%s] ERROR resolving OFD server address '%s:%u'\n", __func__, line, tcp_port);
            return -1;
        }
    }

    srvlen = sizeof(struct sockaddr);
    memset(&srv_conn, 0, srvlen);
    srv_conn.sin_family = AF_INET;
    srv_conn.sin_port   = htons(tcp_port);
    (void)memcpy(&srv_conn.sin_addr, hostPtr->h_addr, hostPtr->h_length);

    if (connect(esoc, (struct sockaddr *)&srv_conn, srvlen) == -1) {
        sprintf(chap, "[%s] ERROR: connect to OFD '%s:%d'\n", __func__, line, tcp_port);
        esoc = -1;
    } else {
        fcntl(esoc, F_SETFL, (fcntl(esoc, F_GETFL)) | O_NONBLOCK);
        sprintf(chap, "[%s] Connect to OFD '%s:%d' OK (sock %d)\n", __func__, line, tcp_port, esoc);
    }
    if (dbg != LOG_OFF) print_msg(1, chap);


    return esoc;
}

//----------------------------------------------------------------------------------------------

void *cli_nitka(void *arg)
{
    cliSTART = 1;
    cliRET = -1;

    int soc = -1;
    uint8_t cikl = 1;
    s_ses *ses = (s_ses *)arg;

    if (!ses) goto done;

    uint8_t *buf = (uint8_t *)ses->data;
    if (!buf) goto done;

    int i, sndlen = ses->datalen;

    char srvURL[max_url_len] = {0};
    if (!ses->url) strcpy(srvURL, def_srv_url);
              else strcpy(srvURL, ses->url);

    soc = openTCP((const char *)srvURL);
    if (soc < 0) goto done;

    if (dbg > LOG_DEBUG) print_msg(1, "[%s] Start client OFD thread\n", __func__);

    char tps[buf_size << 1] = {0};
    int lenrecv = 0, lenrecv_tmp = 0, ready = 0, uk_ofd = 0;
    fd_set read_Fds;
    struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = 50000
    };
    uint8_t from_ofd[buf_size] = {0};



    // send data to OFD server
    if (send(soc, buf, sndlen, MSG_DONTWAIT) != sndlen) {
        if (dbg != LOG_OFF) print_msg(1, "[%s] Send to OFD (%d) Error !!!\n", __func__, sndlen);
        cikl = 0;
    } else {
        if (dbg >= LOG_DEBUG) {
            sprintf(tps, "[%s] Send to OFD (%d) : ", __func__, sndlen);
            for (i = 0; i < sndlen; i++) sprintf(tps+strlen(tps),"%02X", *(buf + i));
            print_msg(1, "%s\n", tps);
        }
    }
    uint32_t tmr = get_timer_sec(def_wait_ack_sec);//wait_ack_lit_sec);

    s_min_head *header = (s_min_head *)from_ofd;
    int hdr_len = sizeof(s_min_head);
    int rx_len = 0, yes = 0;


    // wait answer from OFD server
    while (cikl) {
        //
        if (check_delay_sec(tmr)) {
            if (dbg != LOG_OFF) {
                if (!yes) sprintf(tps, "[%s] Timeout (%d sec), no answer from OFD\n", __func__, def_wait_ack_sec);
                     else sprintf(tps, "[%s] Timeout (%d sec), no more data from OFD\n", __func__, def_wait_ack_sec);
                print_msg(1, tps);
            }
            cikl = 0;
            cliRET = RET_TIMEOUT;
        } else {// read data from OFD
            FD_ZERO(&read_Fds);
            FD_SET(soc, &read_Fds);
            if (select(soc + 1, &read_Fds, NULL, NULL, &tv) > 0) {
                if (FD_ISSET(soc, &read_Fds)) {
                    lenrecv_tmp = recv(soc, &from_ofd[uk_ofd], 1, MSG_DONTWAIT);
                    if (!lenrecv_tmp) {
                        if (dbg != LOG_OFF) print_msg(1, "[%s] Disconnect from OFD side (sock %d)\n", __func__, soc);
                        cliRET = RET_MINOR_ERROR;
                        cikl = 0;
                    } else if (lenrecv_tmp > 0) {
                        ready = 0;
                        lenrecv += lenrecv_tmp;
                        uk_ofd  += lenrecv_tmp;
                        if ( uk_ofd >= sizeof(from_ofd) - 1) ready = 1;
                        else {
                            if (!rx_len) {
                                if (lenrecv == hdr_len) {
                                    rx_len = lenrecv + header->msg_len;
                                    if (lenrecv == rx_len) ready = 1;
                                }
                            } else {
                                if (lenrecv == rx_len) ready = 1;
                            }
                        }
                    }
                }//if (FD_ISSET(soc, &read_Fds))
                if (ready) {
                    ready = 0;
                    cliRET = RET_OK;
                    yes = 1;
                    if (dbg >= LOG_DEBUG) {
                        sprintf(tps, "[%s] Recv from OFD (%d) : ", __func__, lenrecv);
                        for (i = 0; i < lenrecv; i++) sprintf(tps+strlen(tps),"%02X", from_ofd[i]);
                        print_msg(1, "%s\n", tps);
                    }
                    if ((from_ofd[0] == 0x2A) && (from_ofd[1] == 0x08)) {
                        if (dbg != LOG_OFF) {
                            print_msg(1, "[%s] Data from OFD:\n\tsign:0x%02X%02X%02X%02X\n"
                                     "\ts_ver:0x%02X%02X\n"
                                     "\tp_ver:0x%02X%02X\n"
                                     "\tnumFN:%.*s\n"
                                     "\tmsg_len:%u/0x%04X\n"
                                     "\tflags:0x%04X\n"
                                     "\tcrc16:0x%04X\n",
                                     __func__,
                                     header->sign[0], header->sign[1],
                                     header->sign[2], header->sign[3],
                                     header->s_ver[0], header->s_ver[1],
                                     header->p_ver[0], header->p_ver[1],
                                     (int)sizeof(header->numFN), header->numFN,
                                     header->msg_len, header->msg_len,
                                     header->flags, header->crc16);
                            if (header->msg_len) {
                                memset(tps, 0, sizeof(tps));
                                for (i = 0; i < header->msg_len; i++) sprintf(tps+strlen(tps),"%02X", from_ofd[hdr_len + i]);
                                print_msg(1, "%s\n", tps);
                            }
                        }
                        if (header->msg_len) {
                            // !!!!!!!!! append block to ackBuf !!!!!!!!!!
                            int lng = addToBuf(&ackBuf, from_ofd, lenrecv);
                            if (!lng) sprintf(tps, "\nCan't add %u bytes to ackBuf (Internal error !)", lenrecv);
                            else {
                                ackDone = 1;
                                ackLen = lng;
                                sprintf(tps, "\nReady ackBuf(%d):\n", lng);
                                for (i = 0; i < lng; i++) sprintf(tps+strlen(tps), "%02X", *(ackBuf.mem + i));
                            }
                            if (dbg != LOG_OFF) print_msg(0, "%s\n", tps);
                        }
                    }
                    rx_len = 0;
                    lenrecv = lenrecv_tmp = 0;
                    uk_ofd = 0;
                    memset(tps, 0, sizeof(tps));
                    memset(from_ofd, 0, sizeof(from_ofd));
                    cikl = 0;
                }
            }//if (select....
        }//rnd block 'read data from OFD'
        if (QuitCli) break;
        //
    }//while(1)


    closeTCP(&soc);


    if ((dbg != LOG_OFF) && lenrecv) {
        if (cliRET != RET_OK) {
            sprintf(tps, "[%s] Recv from OFD (%d) : ", __func__, lenrecv);
            for (i = 0; i < lenrecv; i++) sprintf(tps+strlen(tps)," %02X", from_ofd[i]);
            print_msg(1, "%s\n", tps);
        }
    }


done:
/**/
    if (ses) {
        if (ses->url) free(ses->url);
        if (ses->data) free(ses->data);
    }
/**/
    if (dbg > LOG_DEBUG) print_msg(1, "[%s] Done client OFD thread\n", __func__);

    cliSTART = 0;

    pthread_exit(NULL);
}


#endif
