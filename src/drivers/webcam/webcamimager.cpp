/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "webcamimager_p.h"
using namespace std;


WebcamImager::Private::Private(const QString &name, int index, const ImageHandlerPtr &handler, WebcamImager *q)
    : name {name}, index {index}, handler {handler}, q {q} {
}



WebcamImager::WebcamImager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(name, index, handler, this)
{
    d->capture = make_shared<cv::VideoCapture> (d->index);
    if (!d->capture->isOpened()) {
        qDebug() << "error opening device";
    }
}






WebcamImager::~WebcamImager()
{
    stopLive();
}

Imager::Chip WebcamImager::chip() const
{
    Chip chip{};
    return chip;
}

QString WebcamImager::name() const
{
    return d->name;
}


Imager::Settings WebcamImager::settings() const
{
    Imager::Settings _settings;

    return _settings;
}

void WebcamImager::setSetting(const Imager::Setting &setting)
{
}

void WebcamImager::startLive()
{
    d->live = true;
    d->future = QtConcurrent::run([ = ] {
        fps_counter fps([ = ](double rate) {
            emit this->fps(rate);
        }, fps_counter::Mode::Elapsed);

        while (d->live) {
            cv::Mat frame;
            *d->capture >> frame;
            d->handler->handle(frame);
            ++fps;
        }
    });
}

void WebcamImager::stopLive()
{
    d->live = false;
    if(d->future.isRunning())
        d->future.waitForFinished();
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
