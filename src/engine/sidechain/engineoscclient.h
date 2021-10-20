/***************************************************************************
                          EngineOscClient.h  -  description
                             -------------------
    copyright            : (C) 2007 by John Sully
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

#pragma once

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/sidechain/sidechainworker.h"
#include "lo/lo.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include <QList>
#include <QElapsedTimer>

class ConfigKey;
class Encoder;

class EngineOscClient : public QObject, public SideChainWorker {
  Q_OBJECT
public:
  EngineOscClient(UserSettingsPointer &pConfig);
  virtual ~EngineOscClient();

private slots:
  void connectServer();
  // interface SideChainWorker
  void process(const CSAMPLE *pBuffer, const int iBufferSize);
  void shutdown() {}

private:
  lo_address m_serverAddress;
  int m_deck;

  UserSettingsPointer m_pConfig;
  QList<ControlProxy *> m_connectedControls;
};