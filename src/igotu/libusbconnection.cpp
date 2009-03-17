/******************************************************************************
 * Copyright (C) 2009  Michael Hofmann <mh21@piware.de>                       *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
 ******************************************************************************/

#include "exception.h"
#include "libusbconnection.h"

#include <boost/shared_ptr.hpp>

#include <usb.h>

#include <QtCore>

namespace igotu
{

class LibusbConnectionPrivate
{
public:
    static QList<struct usb_device*> find_devices(unsigned vendor,
            unsigned product);

    QByteArray receiveBuffer;
    boost::shared_ptr<struct usb_dev_handle> handle;
};

// LibusbConnectionPrivate =====================================================

QList<struct usb_device*> LibusbConnectionPrivate::find_devices(unsigned vendor,
        unsigned product)
{
    QList<struct usb_device*> result;

    for (struct usb_bus *bus = usb_get_busses(); bus; bus = bus->next) {
        for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vendor
                && (product == 0 || dev->descriptor.idProduct == product))
                result.append(dev);
        }
    }

    return result;
}

// LibusbConnection ============================================================

LibusbConnection::LibusbConnection(unsigned vendorId, unsigned productId) :
    dataPtr(new LibusbConnectionPrivate)
{
    D(LibusbConnection);

    usb_init();
    usb_set_debug(255);
    usb_find_busses();
    usb_find_devices();

    if (vendorId == 0)
        vendorId = 0x0df7;
    if (productId == 0)
        productId = 0x0900;

    QList<struct usb_device*> devs = d->find_devices(vendorId, productId);
    // Just in case, try without a product id
    if (devs.isEmpty())
        devs = d->find_devices(vendorId, 0);
    if (devs.isEmpty())
        throw IgotuError(tr("Unable to find device %1")
                .arg(QString().sprintf("%04x:%04x", vendorId, productId)));

    Q_FOREACH (struct usb_device *dev, devs) {
        d->handle.reset(usb_open(dev), usb_close);
        if (d->handle)
            break;
    }

    if (!d->handle)
        throw IgotuError(tr("Unable to open device %1")
                .arg(QString().sprintf("%04x:%04x", vendorId, productId)));

    if (usb_claim_interface(d->handle.get(), 0) != 0)
        throw IgotuError(tr("Unable to claim interface 0 on device %1")
                .arg(QString().sprintf("%04x:%04x", vendorId, productId)));
}

LibusbConnection::~LibusbConnection()
{
    D(LibusbConnection);

    usb_release_interface(d->handle.get(), 0);
}

void LibusbConnection::send(const QByteArray &query)
{
    D(LibusbConnection);

    d->receiveBuffer.clear();

    if (usb_control_msg(d->handle.get(), 0x21, 0x09, 0x0200, 0x0000,
                const_cast<char*>(query.data()), query.size(), 1000) < 0)
        throw IgotuError(tr("Unable to send data to the device"));
}

QByteArray LibusbConnection::receive(unsigned expected)
{
    D(LibusbConnection);

    unsigned toRead = expected;
    QByteArray result;
    Q_FOREVER {
        // This is not ideal, the receive needs to be initiated before the
        // actual send(), and the receives need to stick closely together,
        // otherwise data gets lost
        unsigned toRemove = qMin(unsigned(d->receiveBuffer.size()), toRead);
        result += d->receiveBuffer.left(toRemove);
        d->receiveBuffer.remove(0, toRemove);
        toRead -= toRemove;
        if (toRead == 0)
            return result;
        QByteArray data(0x10, 0);
        int received = usb_interrupt_read(d->handle.get(), 0x00000081,
                data.data(), 0x10, 20);
        if (received < 0)
            throw IgotuError(tr("Unable to read data from the device"));
        d->receiveBuffer += data.left(received);
    }
}

} // namespace
