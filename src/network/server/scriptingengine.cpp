/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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

#include "scriptingengine.h"
#include <QtQml/QJSEngine>
#include <QDebug>
#include "protocol/scriptingprotocol.h"

using namespace std;

class ScriptingPlanetaryImager : public QObject {
  Q_OBJECT
public:
  ScriptingPlanetaryImager(const Configuration::ptr &configuration, const SaveImages::ptr &saveImages);
  void setImager(Imager *imager) { this->imager = imager; }
  
public slots:
  void startRecording();
  void stopRecording();
signals:
  void message(const QString &text);
  
private:
  Configuration::ptr configuration;
  SaveImages::ptr saveImages;
  Imager *imager= nullptr;
};

DPTR_IMPL(ScriptingEngine) {
  ScriptingEngine *q;
  unique_ptr<ScriptingPlanetaryImager> scriptedImager;
  QJSEngine engine;
};


ScriptingPlanetaryImager::ScriptingPlanetaryImager(const Configuration::ptr& configuration, const SaveImages::ptr& saveImages)
  : configuration{configuration}, saveImages{saveImages}
{
}


ScriptingEngine::ScriptingEngine(const Configuration::ptr &configuration, const SaveImages::ptr &saveImages, const NetworkDispatcher::ptr &dispatcher, QObject *parent) 
: QObject(parent),  NetworkReceiver{dispatcher}, dptr(this)
{
  d->scriptedImager = make_unique<ScriptingPlanetaryImager>(configuration, saveImages);
  connect(d->scriptedImager.get(), &ScriptingPlanetaryImager::message, this, [=](const QString &s) { emit reply(s + "\n"); });
  QJSValue objectValue = d->engine.newQObject(d->scriptedImager.get());
  d->engine.globalObject().setProperty("i", objectValue);
  register_handler(ScriptingProtocol::Script, [this](const NetworkPacket::ptr &packet) {
    run(packet->payloadVariant().toString());
  });
  connect(this, &ScriptingEngine::reply, this, [dispatcher](const QString &message) {
    dispatcher->send(ScriptingProtocol::packetScriptReply() << message);
  });
}

ScriptingEngine::~ScriptingEngine()
{
}

void ScriptingEngine::run(const QString& script)
{
  QJSValue v = d->engine.evaluate(script);
  emit reply(v.toString());
}

void ScriptingEngine::setImager(Imager* imager)
{
  d->scriptedImager->setImager(imager);
}

void ScriptingPlanetaryImager::startRecording()
{
  if(! imager) {
    emit message("Error: you must select a cameraa first");
    return;
  }
  saveImages->startRecording(imager);
}

void ScriptingPlanetaryImager::stopRecording()
{
  saveImages->endRecording();
}

#include "scriptingengine.moc"
