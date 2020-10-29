/***************************************************************************
                          EngineOscClient.cpp  -  class to record the mix
                             -------------------
    copyright            : (C) 2007 by John Sully
    copyright            : (C) 2010 by Tobias Rafreider
    copyright            : (C) 2020 by Jack Foltz
    email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "engine/sidechain/engineoscclient.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "oscclient/defs_oscclient.h"
#include "preferences/usersettings.h"

#include "errordialoghandler.h"
#include "mixer/playerinfo.h"
#include "util/event.h"

#include "mixer/playermanager.h"
#include <QDebug>
#include <string>

EngineOscClient::EngineOscClient(UserSettingsPointer &pConfig)
    : m_pConfig(pConfig), m_prefUpdate(ConfigKey("[Preferences]", "updated")) {

  connectServer();

  auto control = [=](const char *name, const char *k) {
    return new ControlProxy(ConfigKey(name, k), this);
  };
  auto deck_control = [=](int n, const char *k) {
    return new ControlProxy(PlayerManager::groupForDeck(n), k, this);
  };

  auto send = [=](const char *path, const char *types, auto... params) {
    std::string addr = std::string("/mixxx/") + path;
    lo_send(m_serverAddress, addr.c_str(), types, params...);
  };

  // master controls
  auto *xfader = control("[Master]", "crossfader");
  xfader->connectValueChanged(this, [=](float v) { send("xfader", "f", v); });

  // per-deck controls
  for (int deckNr = 0; deckNr < (int)PlayerManager::numDecks(); deckNr++) {
    auto *play = deck_control(deckNr, "play");
    play->connectValueChanged(this, [=](int i) {
      send("deck/playing", "ii", deckNr, i);
    });

    auto *volume = deck_control(deckNr, "volume");
    volume->connectValueChanged(this, [=](float f) {
      send("deck/volume", "if", deckNr, f);
    });

    auto *beat = deck_control(deckNr, "beat_active");
    beat->connectValueChanged(this, [=](int i) {
      send("deck/beat", "ii", deckNr, i);
    });

    auto *bpm = deck_control(deckNr, "bpm");
    bpm->connectValueChanged(this, [=](float f) {
      send("deck/bpm", "if", deckNr, f);
    });

    auto *pos = deck_control(deckNr, "playposition");
    pos->connectValueChanged(this, [=](float f) {
      send("deck/pos", "if", deckNr, f);
    });

    auto *dur = deck_control(deckNr, "duration");
    dur->connectValueChanged(this, [=](float f) {
      send("deck/duration", "if", deckNr, f);
    });

    auto *rate = deck_control(deckNr, "rate");
    rate->connectValueChanged(this, [=](float f) {
      send("deck/rate", "if", deckNr, 1.0 + f);
    });

    auto *rateRange = deck_control(deckNr, "rateRange");
    rateRange->connectValueChanged(this, [=](float f) {
      send("deck/rateRange", "if", deckNr, f);
    });

    auto *rev = deck_control(deckNr, "reverse");
    rev->connectValueChanged(this, [=](int i) {
      send("deck/reverse", "ii", deckNr, i);
    });
  }

  // reconnect on settings changes
  //connect(&m_prefUpdate, SIGNAL(valueChanged(double)), this,
  //        SLOT(connectServer()));
}

EngineOscClient::~EngineOscClient() {}

void EngineOscClient::process(const CSAMPLE*, const int) {
  PlayerInfo &playerInfo = PlayerInfo::instance();

  for (int deckNr = 0; deckNr < (int)PlayerManager::numDecks(); deckNr++) {
    TrackPointer track =
        playerInfo.getTrackInfo(PlayerManager::groupForDeck(deckNr));
    if (track) {
      lo_send(m_serverAddress, "/mixxx/deck/title", "is",
              deckNr, track->getTitle().toUtf8().data());
      lo_send(m_serverAddress, "/mixxx/deck/artist", "is",
              deckNr, track->getArtist().toUtf8().data());
      lo_send(m_serverAddress, "/mixxx/deck/song", "iss",
              deckNr, track->getArtist().toUtf8().data(),
	      track->getTitle().toUtf8().data());
      lo_send(m_serverAddress, "/mixxx/deck/file", "is",
              deckNr, track->getLocation().toUtf8().data());
    }
  }
}

void EngineOscClient::connectServer() {
  QString server =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"));
  QString port =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"));
  m_serverAddress =
      lo_address_new(server.toLatin1().data(), port.toLatin1().data());
}
