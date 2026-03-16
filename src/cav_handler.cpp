#include "cav_handler.h" 
#include "cav_msg.h"
#include <cpp-framework/common/time.h>
#include <string>
#include <cstring>
#include <cpp-framework/data/j2735.h>
#include <cpp-framework/text/text_tool.h>
#include <nr-v2x/nr_v2x_utils.h>
 
#define DEBUG_OPTION 0

cav_handler::cav_handler(int port) : sock(this)
{ 
    sock.tcp_bind(port);
}

cav_handler::~cav_handler()
{
    sock.tcp_close();
}

bool cav_handler::forward_lv_msg(uint64_t timestamp, uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_LV_T &msg, const v2x_parameter_field_t &param)
{
    return false;
}
bool cav_handler::forward_fv_msg(uint64_t timestamp, uint32_t dev_id, uint32_t src, uint32_t msg_seq, const DB_V2X_PLATOONING_FV_T &msg, const v2x_parameter_field_t &param)
{

    return false;
}

int cav_handler::on_sock_receive(int index, io_struct &sock)
{
    return 0;
}

int cav_handler::on_sock_connection(int index, io_struct &sock, bool connected)
{
    return 0;
}