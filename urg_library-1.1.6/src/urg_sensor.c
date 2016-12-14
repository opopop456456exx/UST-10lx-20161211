/*!
  \brief URG 僙儞僒惂屼
         URG传感器控制部分
  \author Satofumi KAMIMURA

  $Id: urg_sensor.c,v 66816edea765 2011/05/03 06:53:52 satofumi $

  \todo Mx 寁應拞偵懠偺 Mx 僐儅儞僪傪憲怣偟偨偲偒偵丄揔愗偵摦嶌偡傞傛偆偵偡傞
  当传送其他待办事项的Mx测量期间的Mx命令，以便适当地操作
*/

#include "urg_sensor.h"
#include "urg_errno.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(URG_MSC)
#define snprintf _snprintf
#endif
#define pi 3.141592653

enum {
    URG_FALSE = 0,
    URG_TRUE = 1,

    BUFFER_SIZE = 64 + 2 + 6,       //64B data block + 1B sum +LF ('\0'),+6B最大可能性缓冲区平凑区……
	                                //一个数据行加上上次最后未解码的几个数据最大不超过72B
    EXPECTED_END = -1,

    RECEIVE_DATA_TIMEOUT,
    RECEIVE_DATA_COMPLETE,      /*!< 僨乕僞傪惓忢偵庴怣 */

    PP_RESPONSE_LINES = 10,
    VV_RESPONSE_LINES = 7,
    II_RESPONSE_LINES = 9,

    MAX_TIMEOUT = 140,
};


static const char NOT_CONNECTED_MESSAGE[] = "not connected.";
static const char RECEIVE_ERROR_MESSAGE[] = "receive error.";


//! 僠僃僢僋僒儉偺寁嶼
static char scip_checksum(const char buffer[], int size)
{
    unsigned char sum = 0x00;
    int i;

    for (i = 0; i < size; ++i) {
        sum += buffer[i];
    }

    // 寁嶼偺堄枴偼 SCIP 巇條彂傪嶲徠偺偙偲
    return (sum & 0x3f) + 0x30;
}


static int set_errno_and_return(urg_t *urg, int urg_errno)
{
    urg->last_errno = urg_errno;
    return urg_errno;
}


// 庴怣偟偨墳摎偺峴悢傪曉偡
static int scip_response(urg_t *urg, const char* command,
                         const int expected_ret[], int timeout,
                         char *receive_buffer, int receive_buffer_max_size)
{
    char *p = receive_buffer;
    char buffer[BUFFER_SIZE];
    int filled_size = 0;
    int line_number = 0;
    int ret = URG_UNKNOWN_ERROR;

    int write_size = (int)strlen(command);
    int n = connection_write(&urg->connection, command, write_size);

    if (n != write_size) {
        return set_errno_and_return(urg, URG_SEND_ERROR);
    }

    if (p) {
        *p = '\0';
    }

    do {
        n = connection_readline(&urg->connection, buffer, BUFFER_SIZE, timeout);
        if (n < 0) {
            return set_errno_and_return(urg, URG_NO_RESPONSE);

        } else if (p && (line_number > 0)
                   && (n < (receive_buffer_max_size - filled_size))) {
            // 僄僐乕僶僢僋偼姰慡堦抳偺僠僃僢僋傪峴偆偨傔丄奿擺偟側偄
            memcpy(p, buffer, n);
            p += n;
            *p++ = '\0';
            filled_size += n;
        }

        if (line_number == 0) {
            // 僄僐乕僶僢僋暥帤楍偑丄堦抳偡傞偐傪妋擣偡傞
            if (strncmp(buffer, command, write_size - 1)) {
                return set_errno_and_return(urg, URG_INVALID_RESPONSE);
            }
        } else if (n > 0 && !(line_number == 1 && n == 1)) {
            // 僄僐乕僶僢僋埲奜偺峴偺僠僃僢僋僒儉傪昡壙偡傞(SCIP 1.1 墳摎偺応崌偼柍帇偡傞)
            char checksum = buffer[n - 1];
            if ((checksum != scip_checksum(buffer, n - 1)) &&
                (checksum != scip_checksum(buffer, n - 2))) {
                return set_errno_and_return(urg, URG_CHECKSUM_ERROR);
            }
        }

        // 僗僥乕僞僗墳摎傪昡壙偟偰丄栠傝抣傪寛掕偡傞
        if (line_number == 1) {
            if (n == 1) {
                // SCIP 1.1 墳摎偺応崌偼丄惓忢墳摎偲傒側偡
                ret = 0;

            } else if (n != 3) {
                return set_errno_and_return(urg, URG_INVALID_RESPONSE);

            } else {
                int i;
                int actual_ret = strtol(buffer, NULL, 10);
                for (i = 0; expected_ret[i] != EXPECTED_END; ++i) {
                    if (expected_ret[i] == actual_ret) {
                        ret = 0;
                        break;
                    }
                }
            }
        }

        ++line_number;
    } while (n > 0);

    return (ret < 0) ? ret : (line_number - 1);
}


static void ignore_receive_data(urg_t *urg, int timeout)
{
    char buffer[BUFFER_SIZE];
    int n;

    if (urg->is_sending == URG_FALSE) {
        return;
    }

    do {
        n = connection_readline(&urg->connection,
                                buffer, BUFFER_SIZE, timeout);
    } while (n >= 0);

    urg->is_sending = URG_FALSE;
}


static void ignore_receive_data_with_qt(urg_t *urg, int timeout)
{
    if ((urg->is_sending == URG_FALSE) && (urg->is_laser_on == URG_FALSE)) {
        return;
    }

    connection_write(&urg->connection, "QT\n", 3);
    urg->is_laser_on = URG_FALSE;
    ignore_receive_data(urg, timeout);
}


static int change_sensor_baudrate(urg_t *urg,
                                  long current_baudrate, long next_baudrate)
{
    enum { SS_COMMAND_SIZE = 10 };
    char buffer[SS_COMMAND_SIZE];
    int ss_expected[] = { 0, 3, 4, EXPECTED_END };
    int ret;

    if (current_baudrate == next_baudrate) {
        // 尰嵼偺儃乕儗乕僩偲愝掕偡傞儃乕儗乕僩偑堦弿側傜偽丄栠傞
        return set_errno_and_return(urg, URG_NO_ERROR);
    }

    // "SS" 僐儅儞僪偱儃乕儗乕僩傪曄峏偡傞
    snprintf(buffer, SS_COMMAND_SIZE, "SS%06ld\n", next_baudrate);
    ret = scip_response(urg, buffer, ss_expected, urg->timeout, NULL, 0);

    // 0F 墳摎偺偲偒偼 Ethernet 梡偺僙儞僒偲傒側偟丄惓忢墳摎傪曉偡
    if (ret == -15) {
        return set_errno_and_return(urg, URG_NO_ERROR);
    }
    if (ret <= 0) {
        return set_errno_and_return(urg, URG_INVALID_PARAMETER);
    }

    // 惓忢墳摎側傜偽丄儂僗僩懁偺儃乕儗乕僩傪曄峏偡傞
    ret = connection_set_baudrate(&urg->connection, next_baudrate);

    // 僙儞僒懁偺愝掕斀塮傪懸偮偨傔偵彮偟偩偗懸婡偡傞
    ignore_receive_data(urg, MAX_TIMEOUT);

    return set_errno_and_return(urg, ret);
}


// 儃乕儗乕僩傪曄峏偟側偑傜愙懕偡傞
static int connect_urg_device(urg_t *urg, long baudrate)
{
    long try_baudrate[] = { 19200, 38400, 115200 };
    int try_times = sizeof(try_baudrate) / sizeof(try_baudrate[0]);
    int i;

    // 巜帵偝傟偨儃乕儗乕僩偐傜愙懕偡傞
    for (i = 0; i < try_times; ++i) {
        if (try_baudrate[i] == baudrate) {
            try_baudrate[i] = try_baudrate[0];
            try_baudrate[0] = baudrate;
            break;
        }
    }

    for (i = 0; i < try_times; ++i) {
        enum { RECEIVE_BUFFER_SIZE = 4 };
        int qt_expected[] = { 0, EXPECTED_END };
        char receive_buffer[RECEIVE_BUFFER_SIZE + 1];
        int ret;

        connection_set_baudrate(&urg->connection, try_baudrate[i]);

        // QT 傪憲怣偟丄墳摎偑曉偝傟傞偐偱儃乕儗乕僩偑堦抳偟偰偄傞偐傪妋擣偡傞
        ret = scip_response(urg, "QT\n", qt_expected, MAX_TIMEOUT,
                            receive_buffer, RECEIVE_BUFFER_SIZE);
        if (ret > 0) {
            if (!strcmp(receive_buffer, "E")) {
                int scip20_expected[] = { 0, EXPECTED_END };

                // QT 墳摎偺嵟屻偺夵峴傪撉傒旘偽偡
                ignore_receive_data(urg, MAX_TIMEOUT);

                // "E" 偑曉偝傟偨応崌偼丄SCIP 1.1 偲傒側偟 "SCIP2.0" 傪憲怣偡傞
                ret = scip_response(urg, "SCIP2.0\n", scip20_expected,
                                    MAX_TIMEOUT, NULL, 0);

                // SCIP2.0 墳摎偺嵟屻偺夵峴傪撉傒旘偽偡
                ignore_receive_data(urg, MAX_TIMEOUT);

                // 儃乕儗乕僩傪曄峏偟偰栠傞
                return change_sensor_baudrate(urg, try_baudrate[i], baudrate);

            } else if (!strcmp(receive_buffer, "0Ee")) {
                int tm2_expected[] = { 0, EXPECTED_END };

                // "0Ee" 偑曉偝傟偨応崌偼丄TM 儌乕僪偲傒側偟 "TM2" 傪憲怣偡傞
                scip_response(urg, "TM2\n", tm2_expected,
                              MAX_TIMEOUT, NULL, 0);

                // 儃乕儗乕僩傪曄峏偟偰栠傞
                return change_sensor_baudrate(urg, try_baudrate[i], baudrate);
            }
        }

        if (ret <= 0) {
            if (ret == URG_INVALID_RESPONSE) {
                // 堎忢側僄僐乕僶僢僋偺偲偒偼丄嫍棧僨乕僞庴怣拞偲傒側偟偰
                // 僨乕僞傪撉傒旘偽偡
                ignore_receive_data_with_qt(urg, MAX_TIMEOUT);

                // 儃乕儗乕僩傪曄峏偟偰栠傞
                return change_sensor_baudrate(urg, try_baudrate[i], baudrate);

            } else {
                // 墳摎偑側偄偲偒偼丄儃乕儗乕僩傪曄峏偟偰丄嵞搙愙懕傪峴偆
                ignore_receive_data_with_qt(urg, MAX_TIMEOUT);
                continue;
            }
        } else if (!strcmp("00P", receive_buffer)) {

            // 僙儞僒偲儂僗僩偺儃乕儗乕僩傪曄峏偟偰栠傞
            return change_sensor_baudrate(urg, try_baudrate[i], baudrate);
        }
    }

    return set_errno_and_return(urg, URG_NOT_DETECT_BAUDRATE_ERROR);
}


// PP 僐儅儞僪偺墳摎傪 urg_t 偵奿擺偡傞
static int receive_parameter(urg_t *urg)
{
    enum { RECEIVE_BUFFER_SIZE = BUFFER_SIZE * 9, };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    int pp_expected[] = { 0, EXPECTED_END };
    unsigned short received_bits = 0x0000;
    char *p;
    int i;

    int ret = scip_response(urg, "PP\n", pp_expected, MAX_TIMEOUT,
                            receive_buffer, RECEIVE_BUFFER_SIZE);
    if (ret < 0) {
        return ret;
    } else if (ret < PP_RESPONSE_LINES) {
        ignore_receive_data_with_qt(urg, MAX_TIMEOUT);
        return set_errno_and_return(urg, URG_INVALID_RESPONSE);
    }

    p = receive_buffer;
    for (i = 0; i < (ret - 1); ++i) {

        if (!strncmp(p, "DMIN:", 5)) {
            urg->min_distance = strtol(p + 5, NULL, 10);
            received_bits |= 0x0001;

        } else if (!strncmp(p, "DMAX:", 5)) {
            urg->max_distance = strtol(p + 5, NULL, 10);
            received_bits |= 0x0002;

        } else if (!strncmp(p, "ARES:", 5)) {
            urg->area_resolution = strtol(p + 5, NULL, 10);
            received_bits |= 0x0004;

        } else if (!strncmp(p, "AMIN:", 5)) {
            urg->first_data_index = strtol(p + 5, NULL, 10);
            received_bits |= 0x0008;

        } else if (!strncmp(p, "AMAX:", 5)) {
            urg->last_data_index = strtol(p + 5, NULL, 10);
            received_bits |= 0x0010;

        } else if (!strncmp(p, "AFRT:", 5)) {
            urg->front_data_index = strtol(p + 5, NULL, 10);
            received_bits |= 0x0020;

        } else if (!strncmp(p, "SCAN:", 5)) {
            int rpm = strtol(p + 5, NULL, 10);
            // 僞僀儉傾僂僩帪娫偼丄寁應廃婜偺 16 攞掱搙偺抣偵偡傞
            urg->scan_usec = 1000 * 1000 * 60 / rpm;
            urg->timeout = urg->scan_usec >> (10 - 4);          // 390
            received_bits |= 0x0040;
        }
        p += strlen(p) + 1;
    }

    // 慡偰偺僷儔儊乕僞傪庴怣偟偨偐妋擣
    if (received_bits != 0x007f) {
        return set_errno_and_return(urg, URG_RECEIVE_ERROR);
    }

    urg_set_scanning_parameter(urg,
                               urg->first_data_index - urg->front_data_index,
                               urg->last_data_index - urg->front_data_index,
                               1);

    return set_errno_and_return(urg, URG_NO_ERROR);
}


//! SCIP 暥帤楍偺僨僐乕僪
long urg_scip_decode(const char data[], int size)
{
    const char* p = data;
    const char* last_p = p + size;
    int value = 0;

    while (p < last_p) {
        value <<= 6;
        value &= ~0x3f;
        value |= *p++ - 0x30;
    }
    return value;
}


static int parse_parameter(const char *parameter, int size)
{
    char buffer[5];

    memcpy(buffer, parameter, size);
    buffer[size] = '\0';

    return strtol(buffer, NULL, 10);
}


static urg_measurement_type_t parse_distance_parameter(urg_t *urg,
                                                       const char echoback[])
{
    urg_measurement_type_t ret_type = URG_UNKNOWN;

    urg->received_range_data_byte = URG_COMMUNICATION_3_BYTE;
    if (echoback[1] == 'S') {
        urg->received_range_data_byte = URG_COMMUNICATION_2_BYTE;
        ret_type = URG_DISTANCE;

    } else if (echoback[1] == 'D') {
        if ((echoback[0] == 'G') || (echoback[0] == 'M')) {
            ret_type = URG_DISTANCE;
        } else if ((echoback[0] == 'H') || (echoback[0] == 'N')) {
            ret_type = URG_MULTIECHO;
        }
    } else if (echoback[1] == 'E') {
        if ((echoback[0] == 'G') || (echoback[0] == 'M')) {
            ret_type = URG_DISTANCE_INTENSITY;
        } else if ((echoback[0] == 'H') || (echoback[0] == 'N')) {
            ret_type = URG_MULTIECHO_INTENSITY;
        }
    } else {
        return URG_UNKNOWN;
    }

    // 参数的保存
    urg->received_first_index = parse_parameter(&echoback[2], 4);
    urg->received_last_index = parse_parameter(&echoback[6], 4);
    urg->received_skip_step = parse_parameter(&echoback[10], 2);

    return ret_type;
}


static urg_measurement_type_t parse_distance_echoback(urg_t *urg,
                                                      const char echoback[])
{
    size_t line_length;
    urg_measurement_type_t ret_type = URG_UNKNOWN;

    if (!strcmp("QT", echoback)) {
        return URG_STOP;
    }

    line_length = strlen(echoback);          //最后的\n被\0替换，\0不会被strlen计及
    if ((line_length == 12) &&
        ((echoback[0] == 'G') || (echoback[0] == 'H'))) {
        ret_type = parse_distance_parameter(urg, echoback);

    } else if ((line_length == 15) &&
               ((echoback[0] == 'M') || (echoback[0] == 'N'))) {
        ret_type = parse_distance_parameter(urg, echoback);
    }
    return ret_type;
}


static int receive_length_data(urg_t *urg, long length[],double theta[],
                               unsigned short intensity[],
                               urg_measurement_type_t type, char buffer[])
{
    int n;
    int step_filled = 0;
    int line_filled = 0;
    int multiecho_index = 0;
	int break_point = 0;
	int last_rho;
    int each_size =
        (urg->received_range_data_byte == URG_COMMUNICATION_2_BYTE) ? 2 : 3;     //此处URG_COMMUNICATION_2_BYTE为1,each_size为3
    int data_size = each_size;
    int is_intensity = URG_FALSE;
    int is_multiecho = URG_FALSE;
    int multiecho_max_size = 1;

    if ((type == URG_DISTANCE_INTENSITY) || (type == URG_MULTIECHO_INTENSITY)) {
        data_size *= 2;                  //2*3 或者2*2
        is_intensity = URG_TRUE;
    }
    if ((type == URG_MULTIECHO) || (type == URG_MULTIECHO_INTENSITY)) {
        is_multiecho = URG_TRUE;
        multiecho_max_size = URG_MAX_ECHO;
    }

    do {
        char *p = buffer;
        char *last_p;

        n = connection_readline(&urg->connection,
                                &buffer[line_filled], BUFFER_SIZE - line_filled,
                                urg->timeout);

        if (n > 0) {
            // 僠僃僢僋僒儉偺昡壙        检查和评价，校验和
            if (buffer[line_filled + n - 1] !=
                scip_checksum(&buffer[line_filled], n - 1)) {
                ignore_receive_data_with_qt(urg, urg->timeout);
                return set_errno_and_return(urg, URG_CHECKSUM_ERROR);
            }
        }

        if (n > 0) {
            line_filled += n - 1;
        }

        last_p = p + line_filled;

        while ((last_p - p) >= data_size) {
            int index;

            if (*p == '&') {
                // 愭摢暥帤偑 '&' 偩偭偨偲偒偼丄儅儖僠僄僐乕偺僨乕僞偲傒側偡     先头文字里有“&”，视为有回波数据

                if ((last_p - (p + 1)) < data_size) {
                    // '&' 傪彍偄偰丄data_size 暘僨乕僞偑柍偗傟偽敳偗傞    '&'以外，data _ size分数据；如果没有穿过
                    break;
                }

                --step_filled;
                ++multiecho_index;
                ++p;
                --line_filled;

            } else {
                // 師偺僨乕僞      下一数据
                multiecho_index = 0;
            }

          //  index = (step_filled * multiecho_max_size) + multiecho_index;   //DISTANT模式下，index=step_filled
			index = (step_filled + break_point) ;                      //新的数据存放方式20161214

            if (step_filled >
                (urg->received_last_index - urg->received_first_index)) {
                // 僨乕僞偑懡夁偓傞応崌偼丄巆傝偺僨乕僞傪柍帇偟偰栠傞        数据有多的情况下，无视这些数据返回
                ignore_receive_data_with_qt(urg, urg->timeout);
                return set_errno_and_return(urg, URG_RECEIVE_ERROR);
            }


            if (is_multiecho && (multiecho_index == 0)) {
                // 儅儖僠僄僐乕偺僨乕僞奿擺愭傪僟儈乕僨乕僞偱杽傔傞     用虚拟数据填补了多个回声的数据存储
                int i;
                if (length) {
                    for (i = 1; i < multiecho_max_size; ++i) {
                        length[index + i] = 0;
                    }
                }
                if (intensity) {
                    for (i = 1; i < multiecho_max_size; ++i) {
                        intensity[index + i] = 0;
                    }
                }
            }

            // 嫍棧僨乕僞偺奿擺        距离数据的保存

            if (length) {
                length[index] = urg_scip_decode(p, 3);                          // 解码
				theta[index] = (0.25*pi / 180)*step_filled;                 //此处的弧度数要根据具体扫描角度来改

//******************在此处内嵌代码*************************//
			//point break//
				
				if (index > 0) {
					if ((length[index] - last_rho)< DMAX){

					}
					else {
						break_point++;
						length[index + 1] = 0;
						theta[index+1] = 1000;
					}
				}
				last_rho = length[index];



			//point break//

//******************在此处内嵌代码*************************//

            }
            p += 3;



            // 嫮搙僨乕僞偺奿擺     强度数据的保存

            if (is_intensity) {
                if (intensity) {
                    intensity[index] = (unsigned short)urg_scip_decode(p, 3);
					
					

                }
                p += 3;
            }

            ++step_filled;
            line_filled -= data_size;

        }

        // 師偵張棟偡傞暥帤傪戅旔       把下面处理的文字退避，，不足三个的得弄到前面去形成新的一个
        memmove(buffer, p, line_filled);
    } while (n > 0);             //   最后一个LF的检测、、代表着一帧回复的结束。

    return step_filled+ break_point;
}


//! 嫍棧僨乕僞偺庢摼         距离数据采集
static int receive_data(urg_t *urg, long  data[], double theta[], unsigned short intensity[],
                        long *time_stamp)
{
    urg_measurement_type_t type;
    char buffer[BUFFER_SIZE];
    int ret = 0;
    int n;
    int extended_timeout = urg->timeout
        + 2 * (urg->scan_usec * (urg->scanning_skip_scan) / 1000);     // 390 //timeout=25*16??

//	printf("timeout=%d\nS", extended_timeout);
    // 僄僐乕僶僢僋偺庢摼     回响取得
    n = connection_readline(&urg->connection,                         //此函数返回值不计及\n，返回但缓存区里的\n已被\0替换了。
                            buffer, BUFFER_SIZE, extended_timeout);
    if (n <= 0) {
        return set_errno_and_return(urg, URG_NO_RESPONSE);
    }   
    // 僄僐乕僶僢僋偺夝愅     回波分析
    type = parse_distance_echoback(urg, buffer);    //  得到指令类型。以及各类参数

    // 墳摎偺庢摼     取得响应
    n = connection_readline(&urg->connection,
                            buffer, BUFFER_SIZE, urg->timeout);

    if (n != 3) {                                                            // status +status sum   3byte
        ignore_receive_data_with_qt(urg, urg->timeout);
        return set_errno_and_return(urg, URG_INVALID_RESPONSE);
    }

    if (buffer[n - 1] != scip_checksum(buffer, n - 1)) {
        // 僠僃僢僋僒儉偺昡壙         status校验     99b或者00P
        ignore_receive_data_with_qt(urg, urg->timeout);
        return set_errno_and_return(urg, URG_CHECKSUM_ERROR);
    }

    if (type == URG_STOP) {
        // QT 墳摎偺応崌偵偼丄嵟屻偺夵峴傪撉傒幪偰丄惓忢墳摎偲偟偰張棟偡傞
		//在QT的响应的情况下，
        //丢弃最后新线，
        //它被视为正常响应
        n = connection_readline(&urg->connection,
                                buffer, BUFFER_SIZE, urg->timeout);
        if (n == 0) {
            return 0;
        } else {
            return set_errno_and_return(urg, URG_INVALID_RESPONSE);
        }
    }


	//下面621行至651行主要是部分回波解析，和status校验。
	//经过这段代码，可以检测出第一次回声的00P，并且把那个空白行去掉
	//评估指定扫描数和接到数据的status。
	//有两个左右：
	//一：校验，什么时候是00P和99b是有固定格式的，不符合则通信错误
	//二：分流，通过指定扫描数和status可以调整语句流向，从而顺利连续得到正确的数据

    if (urg->specified_scan_times != 1) {    //指定扫描帧数不为1
        if (!strncmp(buffer, "00", 2)) {     //捕捉到00P
			/*00状态只出现一次，无论是多次扫描指令还是
				单次扫描，即第一次命令回响时status是00P，
				后面的数据行都是99b*/
            n = connection_readline(&urg->connection,
                                    buffer, BUFFER_SIZE, urg->timeout);

            if (n != 0) {                    //00P后必跟着一个空白行
                ignore_receive_data_with_qt(urg, urg->timeout);
                return set_errno_and_return(urg, URG_INVALID_RESPONSE);
            } else {                         //命令回响分析完了，开始连续读数据
                return receive_data(urg, data,theta, intensity, time_stamp);
            }
        }
    }

    if (((urg->specified_scan_times == 1) && (strncmp(buffer, "00", 2))) ||         //status校验
        ((urg->specified_scan_times != 1) && (strncmp(buffer, "99", 2)))) {
        if (urg->error_handler) {
            type = urg->error_handler(buffer, urg);
        }

        //if (type == URG_UNKNOWN) {
            // Gx, Hx 偺偲偒偼 00P 偑曉偝傟偨偲偒偑僨乕僞
            // Mx, Nx 偺偲偒偼 99b 偑曉偝傟偨偲偒偑僨乕僞
            ignore_receive_data_with_qt(urg, urg->timeout);
            return set_errno_and_return(urg, URG_INVALID_RESPONSE);
        //}
    }

    // 僞僀儉僗僞儞僾偺庢摼
    n = connection_readline(&urg->connection,
                            buffer, BUFFER_SIZE, urg->timeout);               // timestamp获取
    if (n > 0) {
        if (time_stamp) {
            *time_stamp = urg_scip_decode(buffer, 4);
        }
    }
	
    // 僨乕僞偺庢摼   数据采集
    switch (type) {               //switch 的用法要注意。
    case URG_DISTANCE:
    case URG_MULTIECHO:
		ret = receive_length_data(urg, data, theta, NULL, type, buffer);   //递归出口，获取长数据
        break;

    case URG_DISTANCE_INTENSITY:
    case URG_MULTIECHO_INTENSITY:
        ret = receive_length_data(urg, data, theta,intensity, type, buffer);
        break;

    case URG_STOP:
    case URG_UNKNOWN:
        ret = 0;
        break;
    }

    // specified_scan_times == 1 偺偲偒偼 Gx 宯僐儅儞僪偑巊傢傟傞偨傔
    // 僨乕僞傪柧帵揑偵掆巭偟側偔偰傛偄
    if ((urg->specified_scan_times > 1) && (urg->scanning_remain_times > 0)) {
        if (--urg->scanning_remain_times <= 0) {
            // 僨乕僞偺掆巭偺傒傪峴偆    做数据的唯一停止
            urg_stop_measurement(urg);                                     //有限次数获取数据方式下，扫描帧数的计数，未扫描完成的帧数为0则代表扫描完成，退出测量
        }
    }
	
    return ret;
}


int urg_open(urg_t *urg, urg_connection_type_t connection_type,
             const char *device_or_address, long baudrate_or_port)
{
    int ret;
    long baudrate = baudrate_or_port;

    urg->is_active = URG_FALSE;
    urg->is_sending = URG_TRUE;
    urg->last_errno = URG_NOT_CONNECTED;
    urg->timeout = MAX_TIMEOUT;
    urg->scanning_skip_scan = 0;
    urg->error_handler = NULL;

    // 僨僶僀僗傊偺愙懕
    ret = connection_open(&urg->connection, connection_type,
                          device_or_address, baudrate_or_port);

    if (ret < 0) {
        switch (connection_type) {
        case URG_SERIAL:
            urg->last_errno = URG_SERIAL_OPEN_ERROR;
            break;

        case URG_ETHERNET:
            urg->last_errno = URG_ETHERNET_OPEN_ERROR;
            break;

        default:
            urg->last_errno = URG_INVALID_RESPONSE;
            break;
        }
        return urg->last_errno;
    }

    // 巜掕偟偨儃乕儗乕僩偱 URG 偲捠怣偱偒傞傛偆偵挷惍
    if (connection_type == URG_ETHERNET) {
        // Ethernet 偺偲偒偼壖偺捠怣懍搙傪巜掕偟偰偍偔
        baudrate = 115200;
    }

    if (connect_urg_device(urg, baudrate) != URG_NO_ERROR) {
        return set_errno_and_return(urg, ret);
    }
    urg->is_sending = URG_FALSE;

    // 曄悢偺弶婜壔
    urg->last_errno = URG_NO_ERROR;
    urg->range_data_byte = URG_COMMUNICATION_3_BYTE;
    urg->specified_scan_times = 0;
    urg->scanning_remain_times = 0;
    urg->is_laser_on = URG_FALSE;

    // 僷儔儊乕僞忣曬傪庢摼
    ret = receive_parameter(urg);
    if (ret == URG_NO_ERROR) {
        urg->is_active = URG_TRUE;
    }
    return ret;
}


void urg_close(urg_t *urg)
{
    if (urg->is_active) {
        ignore_receive_data_with_qt(urg, urg->timeout);
    }
    connection_close(&urg->connection);
    urg->is_active = URG_FALSE;
}


void urg_set_timeout_msec(urg_t *urg, int msec)
{
    urg->timeout = msec;
}


int urg_start_time_stamp_mode(urg_t *urg)
{
    const int expected[] = { 0, EXPECTED_END };
    int n;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    // TM0 傪敪峴偡傞
    n = scip_response(urg, "TM0\n", expected, urg->timeout, NULL, 0);
    if (n <= 0) {
        return set_errno_and_return(urg, URG_INVALID_RESPONSE);
    } else {
        return 0;
    }
}


long urg_time_stamp(urg_t *urg)
{
    const int expected[] = { 0, EXPECTED_END };
    char buffer[BUFFER_SIZE];
    char *p;
    int ret;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    ret = scip_response(urg, "TM1\n", expected,
                        urg->timeout, buffer, BUFFER_SIZE);
    if (ret < 0) {
        return ret;
    }

    // buffer 偐傜僞僀儉僗僞儞僾傪庢摼偟丄僨僐乕僪偟偰曉偡
    if (strcmp(buffer, "00P")) {
        // 嵟弶偺墳摎偑 "00P" 偱側偗傟偽栠傞
        return set_errno_and_return(urg, URG_RECEIVE_ERROR);
    }
    p = buffer + 4;
    if (strlen(p) != 5) {
        return set_errno_and_return(urg, URG_RECEIVE_ERROR);
    }
    if (p[5] == scip_checksum(p, 4)) {
        return set_errno_and_return(urg, URG_CHECKSUM_ERROR);
    }
    return urg_scip_decode(p, 4);
}


int urg_stop_time_stamp_mode(urg_t *urg)
{
    int expected[] = { 0, EXPECTED_END };
    int n;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    // TM2 傪敪峴偡傞
    n = scip_response(urg, "TM2\n", expected, urg->timeout, NULL, 0);
    if (n <= 0) {
        return set_errno_and_return(urg, URG_INVALID_RESPONSE);
    } else {
        return 0;
    }
}


static int send_distance_command(urg_t *urg, int scan_times, int skip_scan,
                                 char single_scan_ch, char continuous_scan_ch,
                                 char scan_type_ch)
{
    char buffer[BUFFER_SIZE];
    int write_size = 0;
    int front_index = urg->front_data_index;
    int n;

    urg->specified_scan_times = (scan_times < 0) ? 0 : scan_times;
    urg->scanning_remain_times = urg->specified_scan_times;
    urg->scanning_skip_scan = (skip_scan < 0) ? 0 : skip_scan;
    if (scan_times >= 100) {
        // 寁應夞悢偑 99 傪墇偊傞応崌偼丄柍尷夞偺僗僉儍儞傪峴偆        如果超过99就执行无限次
        urg->specified_scan_times = 0;
    }

    if (urg->scanning_remain_times == 1) {
        // 儗乕僓敪岝傪巜帵     激光发射指示
        urg_laser_on(urg);

        write_size = snprintf(buffer, BUFFER_SIZE, "%c%c%04d%04d%02d\n",
                              single_scan_ch, scan_type_ch,
                              urg->scanning_first_step + front_index,
                              urg->scanning_last_step + front_index,
                              urg->scanning_skip_step);
    } else {
        write_size = snprintf(buffer, BUFFER_SIZE, "%c%c%04d%04d%02d%01d%02d\n",
                              continuous_scan_ch, scan_type_ch,
                              urg->scanning_first_step + front_index,
                              urg->scanning_last_step + front_index,
                              urg->scanning_skip_step,
                              skip_scan, urg->specified_scan_times);
        urg->is_sending = URG_TRUE;
    }
	urg->scanning_first_step + front_index,
	//	printf("foont_index=%d,firststep=%d,last=%d\n", front_index, urg->scanning_first_step, urg->scanning_last_step);
    n = connection_write(&urg->connection, buffer, write_size);
    if (n != write_size) {
        return set_errno_and_return(urg, URG_SEND_ERROR);
    }

    return 0;
}


int urg_start_measurement(urg_t *urg, urg_measurement_type_t type,
                          int scan_times, int skip_scan)
{
    char range_byte_ch;
    int ret = 0;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    if ((skip_scan < 0) || (skip_scan > 9)) {
        ignore_receive_data_with_qt(urg, urg->timeout);
        return set_errno_and_return(urg, URG_INVALID_PARAMETER);
    }

    // !!! Mx 宯, Nx 宯偺寁應拞偺偲偒偼丄QT 傪敪峴偟偰偐傜
    // !!! 寁應奐巒僐儅儞僪傪憲怣偡傞傛偆偵偡傞
    // !!! 偨偩偟丄MD 寁應拞偵 MD 傪敪峴偡傞傛偆偵丄摨偠僐儅儞僪偺応崌偼
    // !!! Mx 宯, Nx 宯偺寁應偼忋彂偒偡傞偙偲偑偱偒傞傛偆偵偡傞

    // 巜掕偝傟偨僞僀僾偺僷働僢僩傪惗惉偟丄憲怣偡傞
    switch (type) {
    case URG_DISTANCE:
        range_byte_ch =
            (urg->range_data_byte == URG_COMMUNICATION_2_BYTE) ? 'S' : 'D';
        ret = send_distance_command(urg, scan_times, skip_scan,
                                    'G', 'M', range_byte_ch);
        break;

    case URG_DISTANCE_INTENSITY:
        ret = send_distance_command(urg, scan_times, skip_scan,
                                    'G', 'M', 'E');
        break;

    case URG_MULTIECHO:
        ret = send_distance_command(urg, scan_times, skip_scan,
                                    'H', 'N', 'D');
        break;

    case URG_MULTIECHO_INTENSITY:
        ret = send_distance_command(urg, scan_times, skip_scan,
                                    'H', 'N', 'E');
        break;

    case URG_STOP:
    case URG_UNKNOWN:
    default:
        ignore_receive_data_with_qt(urg, urg->timeout);
        urg->last_errno = URG_INVALID_PARAMETER;
        ret = urg->last_errno;
        break;
    }

    return ret;
}


int urg_get_distance(urg_t *urg, long data[], double theta[],long *time_stamp)
{
    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }
    return receive_data(urg, data, theta,NULL, time_stamp);
}


int urg_get_distance_intensity(urg_t *urg,
                               long data[],double theta[], unsigned short intensity[],
                               long *time_stamp)
{
    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    return receive_data(urg, data, theta, intensity, time_stamp);
}


int urg_get_multiecho(urg_t *urg, long data_multi[],double theta_multi[] ,long *time_stamp)
{
    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    return receive_data(urg, data_multi, theta_multi,NULL, time_stamp);
}


int urg_get_multiecho_intensity(urg_t *urg,
                                long data_multi[], double theta_multi[],
                                unsigned short intensity_multi[],
                                long *time_stamp)
{
    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    return receive_data(urg, data_multi, theta_multi,intensity_multi, time_stamp);
}


int urg_stop_measurement(urg_t *urg)
{
    enum { MAX_READ_TIMES = 3 };
    int ret = URG_INVALID_RESPONSE;
    int n;
    int i;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    // QT 傪敪峴偡傞
    n = connection_write(&urg->connection, "QT\n", 3);
    if (n != 3) {
        return set_errno_and_return(urg, URG_SEND_ERROR);
    }

    for (i = 0; i < MAX_READ_TIMES; ++i) {
        // QT 偺墳摎偑曉偝傟傞傑偱丄嫍棧僨乕僞傪撉傒幪偰傞
        ret = receive_data(urg, NULL,NULL, NULL, NULL);
        if (ret == URG_NO_ERROR) {
            // 惓忢墳摎
            urg->is_laser_on = URG_FALSE;
            urg->is_sending = URG_FALSE;
            return set_errno_and_return(urg, URG_NO_ERROR);
        }
    }
    return ret;
}


int urg_set_scanning_parameter(urg_t *urg, int first_step, int last_step,
                               int skip_step)
{
    // 愝掕偺斖埻奜傪巜掕偟偨偲偒偼丄僄儔乕傪曉偡    若指定的设置超出范围，则返回错误
    if (((skip_step < 0) || (skip_step >= 100)) ||
        (first_step > last_step) ||
        (first_step < -urg->front_data_index) ||
        (last_step > (urg->last_data_index - urg->front_data_index))) {
        return set_errno_and_return(urg, URG_SCANNING_PARAMETER_ERROR);
    }

    urg->scanning_first_step = first_step;
    urg->scanning_last_step = last_step;
    urg->scanning_skip_step = skip_step;

    return set_errno_and_return(urg, URG_NO_ERROR);
}


int urg_set_connection_data_size(urg_t *urg,
                                 urg_range_data_byte_t data_byte)
{
    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    if ((data_byte != URG_COMMUNICATION_3_BYTE) ||
        (data_byte != URG_COMMUNICATION_2_BYTE)) {
        return set_errno_and_return(urg, URG_DATA_SIZE_PARAMETER_ERROR);
    }

    urg->range_data_byte = data_byte;

    return set_errno_and_return(urg, URG_NO_ERROR);
}


int urg_laser_on(urg_t *urg)
{
    int expected[] = { 0, 2, EXPECTED_END };
    int ret;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    if (urg->is_laser_on != URG_FALSE) {
        // 婛偵儗乕僓偑敪岝偟偰偄傞偲偒偼丄僐儅儞僪傪憲怣偟側偄傛偆偵偡傞    激光器已经发光的时候，不发送指令？
        urg->last_errno = 0;
        return urg->last_errno;
    }

    ret = scip_response(urg, "BM\n", expected, urg->timeout, NULL, 0);
    if (ret >= 0) {
        urg->is_laser_on = URG_TRUE;
        ret = 0;
    }
    return ret;
}


int urg_laser_off(urg_t *urg)
{
    return urg_stop_measurement(urg);
}


int urg_reboot(urg_t *urg)
{
    int expected[] = { 0, 1, EXPECTED_END };
    int ret;
    int i;

    if (!urg->is_active) {
        return set_errno_and_return(urg, URG_NOT_CONNECTED);
    }

    // 俀夞栚偺 RB 憲怣屻丄愙懕傪愗抐偡傞
    for (i = 0; i < 2; ++i) {
        ret = scip_response(urg, "RB\n", expected, urg->timeout, NULL, 0);
        if (ret < 0) {
            return set_errno_and_return(urg, URG_INVALID_RESPONSE);
        }
    }
    urg->is_active = URG_FALSE;
    urg_close(urg);

    urg->last_errno = 0;
    return urg->last_errno;
}


void urg_sleep(urg_t *urg)
{
    enum { RECEIVE_BUFFER_SIZE = 4 };
    int sl_expected[] = { 0, EXPECTED_END };
    char receive_buffer[RECEIVE_BUFFER_SIZE];

    if (urg_stop_measurement(urg) != URG_NO_ERROR) {
        return;
    }

    scip_response(urg, "%SL\n", sl_expected, MAX_TIMEOUT,
                  receive_buffer, RECEIVE_BUFFER_SIZE);
}


void urg_wakeup(urg_t *urg)
{
    urg_stop_measurement(urg);
}


int urg_is_stable(urg_t *urg)
{
    const char *stat = urg_sensor_status(urg);
    return strncmp("Stable", stat, 6) ? 0 : 1;
}


static char *copy_token(char *dest, char *receive_buffer,
                        const char *start_str, const char *end_ch, int lines)
{
    size_t start_str_len = strlen(start_str);
    size_t end_ch_len = strlen(end_ch);
    int i;
    size_t j;

    for (j = 0; j < end_ch_len; ++j) {
        const char *p = receive_buffer;

        for (i = 0; i < lines; ++i) {
            if (!strncmp(p, start_str, start_str_len)) {

                char *last_p = strchr(p + start_str_len, end_ch[j]);
                if (last_p) {
                    *last_p = '\0';
                    memcpy(dest, p + start_str_len,
                           last_p - (p + start_str_len) + 1);
                    return dest;
                }
            }
            p += strlen(p) + 1;
        }
    }
    return NULL;
}


static const char *receive_command_response(urg_t *urg,
                                            char *buffer, int buffer_size,
                                            const char* command,
                                            int response_lines)
{
    const int vv_expected[] = { 0, EXPECTED_END };
    int ret;

    if (!urg->is_active) {
        return NOT_CONNECTED_MESSAGE;
    }

    ret = scip_response(urg, command, vv_expected, urg->timeout,
                        buffer, buffer_size);
    if (ret < response_lines) {
        return RECEIVE_ERROR_MESSAGE;
    }

    return NULL;
}


const char *urg_sensor_product_type(urg_t *urg)
{
    enum {
        RECEIVE_BUFFER_SIZE = BUFFER_SIZE * VV_RESPONSE_LINES,
    };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const char *ret;
    char *p;

    ret = receive_command_response(urg, receive_buffer, RECEIVE_BUFFER_SIZE,
                                   "VV\n", VV_RESPONSE_LINES);
    if (ret) {
        return ret;
    }

    p = copy_token(urg->return_buffer,
                   receive_buffer, "PROD:", ";", VV_RESPONSE_LINES);
    return (p) ? p : RECEIVE_ERROR_MESSAGE;
}


const char *urg_sensor_serial_id(urg_t *urg)
{
    enum {
        RECEIVE_BUFFER_SIZE = BUFFER_SIZE * VV_RESPONSE_LINES,
    };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const char *ret;
    char *p;

    ret = receive_command_response(urg, receive_buffer, RECEIVE_BUFFER_SIZE,
                                   "VV\n", VV_RESPONSE_LINES);
    if (ret) {
        return ret;
    }

    p = copy_token(urg->return_buffer,
                   receive_buffer, "SERI:", ";", VV_RESPONSE_LINES);
    return (p) ? p : RECEIVE_ERROR_MESSAGE;
}


const char *urg_sensor_firmware_version(urg_t *urg)
{
    enum {
        RECEIVE_BUFFER_SIZE = BUFFER_SIZE * VV_RESPONSE_LINES,
    };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const char *ret;
    char *p;

    if (!urg->is_active) {
        return NOT_CONNECTED_MESSAGE;
    }

    ret = receive_command_response(urg, receive_buffer, RECEIVE_BUFFER_SIZE,
                                   "VV\n", VV_RESPONSE_LINES);
    if (ret) {
        return ret;
    }

    p = copy_token(urg->return_buffer,
                   receive_buffer, "FIRM:", " (", VV_RESPONSE_LINES);
    return (p) ? p : RECEIVE_ERROR_MESSAGE;
}


const char *urg_sensor_status(urg_t *urg)
{
    enum {
        RECEIVE_BUFFER_SIZE = BUFFER_SIZE * II_RESPONSE_LINES,
    };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const char *ret;
    char *p;

    if (!urg->is_active) {
        return NOT_CONNECTED_MESSAGE;
    }

    ret = receive_command_response(urg, receive_buffer, RECEIVE_BUFFER_SIZE,
                                   "II\n", II_RESPONSE_LINES);
    if (ret) {
        return ret;
    }

    p = copy_token(urg->return_buffer,
                   receive_buffer, "STAT:", ";", II_RESPONSE_LINES);
    return (p) ? p : RECEIVE_ERROR_MESSAGE;
}


const char *urg_sensor_state(urg_t *urg)
{
    enum {
        RECEIVE_BUFFER_SIZE = BUFFER_SIZE * II_RESPONSE_LINES,
    };
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const char *ret;
    char *p;

    if (!urg->is_active) {
        return NOT_CONNECTED_MESSAGE;
    }

    ret = receive_command_response(urg, receive_buffer, RECEIVE_BUFFER_SIZE,
                                   "II\n", II_RESPONSE_LINES);
    if (ret) {
        return ret;
    }

    p = copy_token(urg->return_buffer,
                   receive_buffer, "MESM:", " (", II_RESPONSE_LINES);
    return (p) ? p : RECEIVE_ERROR_MESSAGE;
}


void urg_set_error_handler(urg_t *urg, urg_error_handler handler)
{
    urg->error_handler = handler;
}
