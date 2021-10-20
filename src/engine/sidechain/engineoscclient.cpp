/***************************************************************************
                          EngineOscClient.cpp  -  class to record the mix
                             -------------------
    copyright            : (C) 2007 by John Sully
    copyright            : (C) 2010 by Tobias Rafreider
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
#include <QtDebug>

EngineOscClient::EngineOscClient(UserSettingsPointer &pConfig)
    : m_pConfig(pConfig) {

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

  connectServer();
  control("[Preferences]", "updated")->connectValueChanged(this, [=]() { connectServer(); });
  
  deck_control(0, "bpm")->connectValueChanged(this, [=](float bpm) {
    if (m_deck == 0) {
      send("bpm", "f", bpm);
    }
  });

  deck_control(1, "bpm")->connectValueChanged(this, [=](float bpm) {
    if (m_deck == 1) {
      send("bpm", "f", bpm);
    }
  });

  connect(&PlayerInfo::instance(),
          &PlayerInfo::currentPlayingDeckChanged,
          this,
          [=](int deck) { 
            m_deck = deck;
            send("bpm", "f", deck_control(deck, "bpm")->get());
          });
}

EngineOscClient::~EngineOscClient() {}

// void EngineOscClient::sendState() {
//   PlayerInfo &playerInfo = PlayerInfo::instance();

//   int numDecks = (int)PlayerManager::numDecks();
//   lo_send(m_serverAddress, "/mixxx/numDecks", "i", numDecks);

//   for (int deckNr = 0; deckNr < numDecks; deckNr++) {
//     // lo_send(m_serverAddress, "/mixxx/deck/playing", "ii", deckNr,
//     //         (int)playerInfo.isDeckPlaying(deckNr));

//     // speed
//     ControlProxy rate(ConfigKey(PlayerManager::groupForDeck(deckNr), "rate"));
//     ControlProxy rateRange(
//         ConfigKey(PlayerManager::groupForDeck(deckNr), "rateRange"));
//     ControlProxy rev(ConfigKey(PlayerManager::groupForDeck(deckNr), "reverse"));
//     float speed = 1 + float(rate.get()) * float(rateRange.get());
//     if (rev.get())
//       speed *= -1;
//     lo_send(m_serverAddress, "/mixxx/deck/speed", "if", deckNr, speed);

//     ControlProxy posRel(
//         ConfigKey(PlayerManager::groupForDeck(deckNr), "playposition"));
//     lo_send(m_serverAddress, "/mixxx/deck/pos", "if", deckNr,
//             float(posRel.get()));

//     ControlProxy dur(
//         ConfigKey(PlayerManager::groupForDeck(deckNr), "duration"));
//     lo_send(m_serverAddress, "/mixxx/deck/duration", "if", deckNr,
//             float(dur.get()));

//     QString title = "";
//     TrackPointer pTrack =
//         playerInfo.getTrackInfo(PlayerManager::groupForDeck(deckNr));
//     if (pTrack) {
//       title = pTrack->getTitle();
//     }
//     lo_send(m_serverAddress, "/mixxx/deck/title", "is", deckNr,
//             title.toUtf8().data());
//   }
//   m_time.restart();
// }

// void EngineOscClient::maybeSendState() {
//   if (m_time.elapsed() < 10)
//     return;
//   sendState();
// }

void EngineOscClient::process(const CSAMPLE*, const int) {
}

void EngineOscClient::connectServer() {
  QString server =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"));
  QString port =
      m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"));
  m_serverAddress =
      lo_address_new(server.toLatin1().data(), port.toLatin1().data());
}