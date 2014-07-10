/****************************************************************************
 *
 *   Copyright (c) 2012-2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file uploader.h Firmware uploader for PX4 boards
 */

#ifndef _PX4_Uploader_H
#define _PX4_Uploader_H value

#include <stdint.h>
#include <QSerialPort>

#include <QObject>
#include <QFile>
#include <QByteArray>

class PX4_Uploader : public QObject
{
    Q_OBJECT
public:
    PX4_Uploader(QSerialPort* port, QObject *parent=NULL);
    virtual ~PX4_Uploader();

    int upload(const QString& filename, int filterId=-1, bool insync=false);
    int detect(int &r_board_id);
    int get_bl_info(quint32 &board_id, quint32 &board_rev, quint32 &flash_size, QString &humanReadable, bool &insync);

    QString getBoardName();
    QString getBootloaderName();

signals:
     void upgradeProgressChanged(int percent);
     void upgradeStatusChanged(const QString& str);

private:
	enum {

		PROTO_NOP		= 0x00,
		PROTO_OK		= 0x10,
		PROTO_FAILED		= 0x11,
		PROTO_INSYNC		= 0x12,
		PROTO_EOC		= 0x20,
		PROTO_GET_SYNC		= 0x21,
		PROTO_GET_DEVICE	= 0x22,
		PROTO_CHIP_ERASE	= 0x23,
		PROTO_CHIP_VERIFY	= 0x24,
		PROTO_PROG_MULTI	= 0x27,
		PROTO_READ_MULTI	= 0x28,
		PROTO_GET_CRC		= 0x29,
		PROTO_REBOOT		= 0x30,

		INFO_BL_REV		= 1,		/**< bootloader protocol revision */
        BL_REV			= 4,		/**< supported bootloader protocol  */
		INFO_BOARD_ID		= 2,		/**< board type */
		INFO_BOARD_REV		= 3,		/**< board revision */
		INFO_FLASH_SIZE		= 4,		/**< max firmware size in bytes */

        PROG_MULTI_MAX		= 128,		/**< protocol max is 255, must be multiple of 4 */
        READ_MULTI_MAX		= 60		/**< protocol max is 255, something overflows with >= 64 */

	};

    QSerialPort*		_io_fd;
    QFile		_fw_fd;
    QList<QString> boardNames;

    quint32	bl_rev;         ///< bootloader revision
    quint32    board_id;       ///< board ID
    quint32    board_rev;      ///< board revision
    quint32    flash_size;     ///< flash size

	void			log(const char *fmt, ...);

    int			recv_single(uint8_t &c, unsigned timeout);
	int			recv(uint8_t *p, unsigned count);
	void			drain();
	int			send(uint8_t c);
	int			send(uint8_t *p, unsigned count);
	int			get_sync(unsigned timeout = 1000);
	int			sync();
	int			get_info(int param, uint32_t &val);
	int			erase();
	int			program(size_t fw_size);
	int			verify_rev2(size_t fw_size);
	int			verify_rev3(size_t fw_size);
	int			reboot();
    void        send_app_reboot();
};

#endif
