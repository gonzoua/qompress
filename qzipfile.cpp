/**
 * Copyright (c) 2011 Oleksandr Tymoshenko <gonzo@bluezbox.com>
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "qzipfile.h"
namespace qompress 
{

QZipFile::QZipFile(const QString &name, QObject *parent) :
    QIODevice(parent), m_fileName(name), m_unzFile(0)
{
}

QZipFile::~QZipFile()
{
}

bool QZipFile::open()
{
    if (m_unzFile) {
        setErrorString("File is already open"); 
        return false;
    }

    m_unzFile = unzOpen64(m_fileName.toAscii());
    if (m_unzFile)
        return true;
    else {
        // no valid information at this point
        setErrorString("Error opening Zip file");
        return false;
    }
}

void QZipFile::close()
{
    unzClose(m_unzFile);
    m_unzFile = 0;
}

qint64 QZipFile::readData(char*, qint64)
{
    return -1;
}

qint64 QZipFile::writeData(const char*, qint64)
{
    return -1;
}


QZipFileEntry QZipFile::currentEntry()
{
    int err;
    char filename_inzip[256];
    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(m_unzFile, &file_info, filename_inzip, 
            sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (err) {
        setErrorString("Failed to current get entry info");
        return QZipFileEntry();
    }
    else 
        return QZipFileEntry(filename_inzip, file_info);

}

bool QZipFile::gotoFirstEntry()
{
    return (unzGoToFirstFile(m_unzFile) == UNZ_OK);
}

bool QZipFile::nextEntry()
{
    int err = unzGoToNextFile(m_unzFile);
    return (err == UNZ_OK);
}

bool QZipFile::extractCurrentEntry(QIODevice &out, const QString &password)
{
    int err;
    char buf[8192];
    bool result = true;

    if (!password.isEmpty())
        err = unzOpenCurrentFilePassword(m_unzFile, password.toAscii());
    else
        err = unzOpenCurrentFile(m_unzFile);
    if (err!=UNZ_OK) {
        setErrorString("Error opening archive entry");
        return false;
    }

    do
    {
        err = unzReadCurrentFile(m_unzFile, buf, sizeof(buf));
        if (err<0)
        {
            setErrorString("Error reading current archive entry");
            break;
        }
        if (err>0) {
            qint64 written;

            do {
                written = out.write(buf, err);
                if (written < 0)
                    break;
                err -= written; 
            } while (err > 0);

            if (written < 0)
                err = -1;
        }
    }
    while (err>0);

    if (err < 0)
        result = false;

    err = unzCloseCurrentFile(m_unzFile);

    if (err != UNZ_OK)
        result = false;

    return result;
}

bool QZipFile::extractEntry(QIODevice &out, const QString file, const QString &password)
{
    if (unzLocateFile(m_unzFile, file.toAscii(), 1) != UNZ_OK)
    {
        setErrorString("File '" + file + "' not found in archive");
        return false;
    }

    return extractCurrentEntry(out, password);
}

QStringList QZipFile::filenames()
{
    QStringList result;
    gotoFirstEntry();
    do {
        QZipFileEntry info = currentEntry();
        if (info.isValid())
            result << info.name();
    } while (nextEntry());

    return result;
}

} // namespace
