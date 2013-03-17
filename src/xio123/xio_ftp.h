/*
 * Copyright 2008-2013 M.Mueller
 * Copyright 2006 Dmitry A.Steklenev
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XIO_FTP_H
#define XIO_FTP_H

#include "xio.h"
#include "xio_protocol.h"
#include "xio_socket.h"
#include <cpp/xstring.h>

#ifndef FTPBASEERR
#define FTPBASEERR 22000
#endif

#define FTP_ANONYMOUS_USER  "anonymous"
#define FTP_ANONYMOUS_PASS  "xio123@dom.pm123"

#define FTP_CONNECTION_ALREADY_OPEN 125
#define FTP_OPEN_DATA_CONNECTION    150
#define FTP_OK                      200
#define FTP_FILE_STATUS             213
#define FTP_SERVICE_READY           220
#define FTP_TRANSFER_COMPLETE       226
#define FTP_PASSIVE_MODE            227
#define FTP_LOGGED_IN               230
#define FTP_FILE_ACTION_OK          250
#define FTP_DIRECTORY_CREATED       257
#define FTP_FILE_CREATED            257
#define FTP_WORKING_DIRECTORY       257
#define FTP_NEED_PASSWORD           331
#define FTP_NEED_ACCOUNT            332
#define FTP_FILE_OK                 350
#define FTP_FILE_UNAVAILABLE        450
#define FTP_SYNTAX_ERROR            500
#define FTP_BAD_ARGS                501
#define FTP_NOT_IMPLEMENTED         502
#define FTP_BAD_SEQUENCE            503
#define FTP_NOT_IMPL_FOR_ARGS       504
#define FTP_NOT_LOGGED_IN           530
#define FTP_FILE_NO_ACCESS          550
#define FTP_PROTOCOL_ERROR          999

class XIOftp : public XIOreadonly
{private:
  XSFLAGS       support;
  XIOsocket     s_socket;   // Connection.
  int64_t       s_pos;      // Current position of the stream pointer.
  int64_t       s_size;     // Size of the associated file.
  xstring       s_location; // Saved resource location.

  XIOsocket     s_datasocket;
  char          s_reply[512];

 private:
  /// Get and parse a FTP server response.
  int read_reply();
  /// Sends a command to a FTP server and checks response.
  int send_command(const char* command, const char* params);
  /// Initiates the transfer of the file specified by filename.
  int transfer_file(int64_t range);

 public:
  /// Initializes the ftp protocol.
  XIOftp();
  //virtual ~XIOftp();
  virtual int open(const char* filename, XOFLAGS oflags);
  virtual int read(void* result, unsigned int count);
  virtual int close();
  virtual int64_t tell();
  virtual int64_t seek(int64_t offset, XIO_SEEK origin);
  virtual int64_t getsize();
  virtual int getstat(XSTATL* st);
  virtual XSFLAGS supports() const;
  virtual XIO_PROTOCOL protocol() const;

  /// Maps the error number in errnum to an error message string.
  static const char* strerror(int errnum);
};

#endif /* XIO_FTP_H */

