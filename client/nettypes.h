#ifndef NETTYPES_H
#define NETTYPES_H

#pragma once
#include <QByteArray>
#include <QHostAddress>
#include <QMetaType>

struct TcpPacket { QByteArray payload; };
struct UdpPacket { QHostAddress peer; quint16 port; QByteArray payload; };

Q_DECLARE_METATYPE(TcpPacket)
Q_DECLARE_METATYPE(UdpPacket)

#endif // NETTYPES_H
