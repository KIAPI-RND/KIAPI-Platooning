

#include <cpp-framework/sock/sock_handler.hpp>
#include <cpp-framework/gps/gps_handler.h>
#include <pq-utils/pq_handler.h>
#include <nr-v2x/nr_v2x_conf.h>
#include "server.h"
#include <cpp-framework/data/j2735.h>

server_handler::server_handler(uint32_t port) : thread(), sock(this)
{
    db_upate_tick.set(100);
    sock.tcp_bind(port);
    thread.start(this, &server_handler::progress_thread, nullptr);
}

server_handler::~server_handler()
{
    sock.tcp_close();
}

void server_handler::set_db(pq_db_config_t config)
{
    pq.connect(config);
}
