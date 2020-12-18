struct KeepConfig {
    /** The time (in seconds) the connection needs to remain 
     * idle before TCP starts sending keepalive probes (TCP_KEEPIDLE socket option)
     */
    int keepidle;
    /** The maximum number of keepalive probes TCP should 
     * send before dropping the connection. (TCP_KEEPCNT socket option)
     */
    int keepcnt;

    /** The time (in seconds) between individual keepalive probes.
     *  (TCP_KEEPINTVL socket option)
     */
    int keepintvl;
};

/**
* enable TCP keepalive on the socket
* @param fd file descriptor
* @return 0 on success -1 on failure
*/

int set_tcp_keepalive(int sockfd)
{
    int optval = 1;

    return setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

/** Set the keepalive options on the socket
* This also enables TCP keepalive on the socket
*
* @param fd file descriptor
* @param fd file descriptor
* @return 0 on success -1 on failure
*/

int set_tcp_keepalive_cfg(int sockfd, const struct KeepConfig *cfg)
{
    int rc;

    //first turn on keepalive
    rc = set_tcp_keepalive(sockfd);
    if (rc != 0) {
        return rc;
    }

    //set the keepalive options
    rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &cfg->keepcnt, sizeof cfg->keepcnt);
    if (rc != 0) {
        return rc;
    }

    rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &cfg->keepidle, sizeof cfg->keepidle);
    if (rc != 0) {
        return rc;
    }

    rc = setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &cfg->keepintvl, sizeof cfg->keepintvl);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

status = send( sock, buffer, buflen, MSG_DONTWAIT );
 
if (status == -1) 
{
   /* send failed */
  printf( "send failed: %s\n", strerror(errno) );
  if (errno == EPIPE)
 
} else 
{
 
  /* send succeeded -- or did it? */
 
}


int sock, status;
sock = socket( AF_INET, SOCK_STREAM, 0 );
status = read( sock, buffer, buflen );
 
if (status > 0) {
 
  /* Data read from the socket */
 
} else if (status == -1) {
 
  /* Error, check errno, take action... */
 
} else if (status == 0) {
 
  /* Peer closed the socket, finish the close */
  close( sock );
 
  /* Further processing... */
 
