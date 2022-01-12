#include "Endian.h"

// 这些平台都没用过 用到了在说
// #elif defined(U_OS_FREEBSD)
// /* https://www.unix.com/man-page/freebsd/9/bswap32/ */
// #include <sys/endian.h>
// #define bswap_16(x) bswap16(x)
// #define bswap_32(x) bswap32(x)
// #define bswap_64(x) bswap64(x)

// #elif defined(__OpenBSD__)

// #include <sys/types.h>
// #define bswap_16(x) swap16(x)
// #define bswap_32(x) swap32(x)
// #define bswap_64(x) swap64(x)

// #elif defined(__NetBSD__)
// /* https://nixdoc.net/man-pages/NetBSD/bswap16.3.html */
// #include <sys/types.h>
// #include <machine/bswap.h>
// #if defined(__BSWAP_RENAME) && !defined(__bswap_32)
// #define bswap_16(x) bswap16(x)
// #define bswap_32(x) bswap32(x)
// #define bswap_64(x) bswap64(x)
// #endif

// qint16 Endian::bSwap(qint16 value) { return bSwap<qint16>(value); }
// quint16 Endian::bSwap(quint16 value) { return bSwap<quint16>(value); }
// qint32 Endian::bSwap(qint32 value) { return bSwap<qint32>(value); }
// quint32 Endian::bSwap(quint32 value) { return bSwap<quint32>(value); }
// qint64 Endian::bSwap(qint64 value) { return bSwap<qint64>(value); }
// quint64 Endian::bSwap(quint64 value) { return bSwap<quint64>(value); }
// float Endian::bSwap(float value) { return bSwap<float>(value); }
// double Endian::bSwap(double value) { return bSwap<double>(value); }

#if IS_LITTLEENDIAN
quint16 Endian::hton(quint16 hvalue) { return bSwap<quint16>(hvalue); }
qint16 Endian::hton(qint16 hvalue) { return bSwap<qint16>(hvalue); }
quint32 Endian::hton(quint32 hvalue) { return bSwap<quint32>(hvalue); }
qint32 Endian::hton(qint32 hvalue) { return bSwap<qint32>(hvalue); }
quint64 Endian::hton(quint64 hvalue) { return bSwap<quint64>(hvalue); }
qint64 Endian::hton(qint64 hvalue) { return bSwap<qint64>(hvalue); }

quint16 Endian::ntoh(quint16 nvalue) { return bSwap<quint16>(nvalue); }
qint16 Endian::ntoh(qint16 nvalue) { return bSwap<qint16>(nvalue); }
quint32 Endian::ntoh(quint32 nvalue) { return bSwap<quint32>(nvalue); }
qint32 Endian::ntoh(qint32 nvalue) { return bSwap<qint32>(nvalue); }
quint64 Endian::ntoh(quint64 nvalue) { return bSwap<quint64>(nvalue); }
qint64 Endian::ntoh(qint64 nvalue) { return bSwap<qint64>(nvalue); }

#else
quint16 Endian::hton(quint16 hvalue) { return hvalue; }
qint16 Endian::hton(qint16 hvalue) { return hvalue; }
quint32 Endian::hton(quint32 hvalue) { return hvalue; }
qint32 Endian::hton(qint32 hvalue) { return hvalue; }
quint64 Endian::hton(quint64 hvalue) { return hvalue; }
qint64 Endian::hton(qint64 hvalue) { return hvalue; }

quint16 Endian::ntoh(quint16 nvalue) { return nvalue; }
qint16 Endian::ntoh(qint16 nvalue) { return nvalue; }
quint32 Endian::ntoh(quint32 nvalue) { return nvalue; }
qint32 Endian::ntoh(qint32 nvalue) { return nvalue; }
quint64 Endian::ntoh(quint64 nvalue) { return nvalue; }
qint64 Endian::ntoh(qint64 nvalue) { return nvalue; }

#endif
