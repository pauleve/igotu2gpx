/******************************************************************************
 * Copyright (C) 2009  Michael Hofmann <mh21@mh21.de>                         *
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

#ifndef _IGOTU2GPX_SRC_IGOTU_MESSAGES_H_
#define _IGOTU2GPX_SRC_IGOTU_MESSAGES_H_

#include "global.h"

namespace igotu
{

class IGOTU_EXPORT Messages
{
public:
    static int verbose();
    static void setVerbose(int value);

    // stdout + no LF
    static void directOutput(const QByteArray &data);

    // stdout + LF
    static void textOutput(const QString &message);

    // stderr + LF
    static void errorMessage(const QString &message);
    static void normalMessage(const QString &message);
    static void verboseMessage(const QString &message);

    // stderr + no LF
    static void normalMessagePart(const QString &message);
};

} // namespace igotu

#endif

