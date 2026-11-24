#ifndef SOCKS5_H
#define SOCKS5_H

/**
 * socks5.h - constantes de los protocolos SOCKSv5 (RFC 1928) y
 * autenticación usuario/contraseña (RFC 1929).
 *
 * Compartidas por hello, auth, request y socks5nio.
 */

/* RFC 1928 */

#define SOCKS5_VERSION                0x05

/* métodos de autenticación (hello) */
#define SOCKS5_METHOD_NO_AUTH         0x00
#define SOCKS5_METHOD_GSSAPI          0x01
#define SOCKS5_METHOD_USERPASS        0x02
#define SOCKS5_METHOD_NO_ACCEPTABLE   0xFF

/* comandos (request) */
#define SOCKS5_CMD_CONNECT            0x01
#define SOCKS5_CMD_BIND               0x02
#define SOCKS5_CMD_UDP_ASSOCIATE      0x03

/* tipos de dirección (request / reply) */
#define SOCKS5_ATYP_IPV4              0x01
#define SOCKS5_ATYP_DOMAINNAME        0x03
#define SOCKS5_ATYP_IPV6              0x04

/** códigos de respuesta (campo REP del reply) */
enum socks5_reply {
    SOCKS5_REPLY_SUCCEEDED                  = 0x00,
    SOCKS5_REPLY_GENERAL_FAILURE            = 0x01,
    SOCKS5_REPLY_CONNECTION_NOT_ALLOWED     = 0x02,
    SOCKS5_REPLY_NETWORK_UNREACHABLE        = 0x03,
    SOCKS5_REPLY_HOST_UNREACHABLE           = 0x04,
    SOCKS5_REPLY_CONNECTION_REFUSED         = 0x05,
    SOCKS5_REPLY_TTL_EXPIRED                = 0x06,
    SOCKS5_REPLY_COMMAND_NOT_SUPPORTED      = 0x07,
    SOCKS5_REPLY_ADDRESS_TYPE_NOT_SUPPORTED = 0x08,
};

/*  RFC 1929 */

#define SOCKS5_AUTH_VERSION           0x01
#define SOCKS5_AUTH_SUCCESS           0x00
#define SOCKS5_AUTH_FAILURE           0x01

/** largo máximo de UNAME y PASSWD (campo de 1 byte) */
#define SOCKS5_AUTH_MAX_LEN           255

#endif
