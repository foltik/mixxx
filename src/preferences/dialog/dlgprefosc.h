#pragma once

#include "util/memory.h"

#include <QRadioButton>
#include <QWidget>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefoscdlg.h"
#include "preferences/usersettings.h"

class ControlObject;
class ControlObjectThread;

class DlgPrefOsc : public DlgPreferencePage, public Ui::DlgPrefOscDlg {
    Q_OBJECT
  public:
    DlgPrefOsc(QWidget *parent, UserSettingsPointer &pConfig);
    virtual ~DlgPrefOsc();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  private slots:
    void slotError(double error);

  signals:
    void apply(const QString &);

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;

    std::unique_ptr<ControlObject> m_pUpdateCO;
    std::unique_ptr<ControlObject> m_pErrorCO;
};